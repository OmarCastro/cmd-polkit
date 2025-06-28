#!/usr/bin/env python

"""
setup instructions on ../rofi-example-with-fprintd-so.sh
"""

import json
import sys
import subprocess
import asyncio

rofi_process_num = 0
rofi_message = ""
rofi_message_logs = ""
prompt = "password: "
rofi_process = None
to_exit = False

def shutdown():
    """
    Cancel all running async tasks (other than this one) when called.
    By catching asyncio.CancelledError, any running task can perform
    any necessary cleanup when it's cancelled.
    """
    global to_exit
    loop = asyncio.get_event_loop()
    tasks = []
    for task in asyncio.all_tasks(loop):
        if task is not asyncio.current_task(loop):
            task.cancel()
            tasks.append(task)
    to_exit = True
            

def kill_rofi():
    global rofi_process
    process_to_terminate = rofi_process
    rofi_process = None
    if process_to_terminate == None: return
    if process_to_terminate.returncode != None: return
    process_to_terminate.terminate()

async def handle_process_exit(handling_process, proc_num):
    global rofi_process_num
    out, err = await handling_process.communicate()
    if rofi_process_num == proc_num:
        if out == b'':
            print( """{"action": "cancel"}""" , flush=True)
            shutdown()
        else:
            print( """{"action": "authenticate", "password": "%s"}""" % str(out, "utf-8").rstrip("\n"), flush=True)

async def open_rofi_password():
    global rofi_process
    global rofi_process_num
    kill_rofi()
    rofi_process_num += 1
    message = rofi_message + "\n" if rofi_message_logs == "" else rofi_message + "\n\n<b>Logs:</b>\n" + rofi_message_logs
    new_process = await asyncio.create_subprocess_exec(
       'rofi', '-password', '-dmenu', '-markup', '-no-fixed-num-lines', '-p', prompt, '-mesg', message,
       stdin=asyncio.subprocess.PIPE,
       stdout=asyncio.subprocess.PIPE,
       stderr=asyncio.subprocess.PIPE)
    rofi_process = new_process
    asyncio.create_task(handle_process_exit(new_process, rofi_process_num))

async def log_notify_message_in_rofi(text):
    global rofi_message_logs
    kill_rofi()
    rofi_message_logs += text + "\n"
    await open_rofi_password()

async def connect_stdin_stdout():
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await loop.connect_read_pipe(lambda: protocol, sys.stdin)
    w_transport, w_protocol = await loop.connect_write_pipe(asyncio.streams.FlowControlMixin, sys.stdout)
    writer = asyncio.StreamWriter(w_transport, w_protocol, reader, loop)
    return reader, writer

async def main():
    global rofi_message
    global prompt
    stdin, stdoun = await connect_stdin_stdout()
    line = ""
    while not to_exit:
        try:
            line = await stdin.readuntil()
        except:
            continue
        jsonObj = json.loads(line)
        if "action" not in jsonObj :
            continue
        elif jsonObj["action"] == "request password":
            #print( """{"password": "%s"}""" % "tesmar" , flush=True)
            if "prompt" in jsonObj:
                prompt = jsonObj["prompt"]
            if "message" in jsonObj:
                message = jsonObj["message"]
                rofi_message = message
            if "polkit action" in jsonObj and jsonObj["polkit action"] != None:
                polkit_action = jsonObj["polkit action"]
                rofi_message = rofi_message + "\n\n"
                if "description" in polkit_action: rofi_message = rofi_message + "\n<b>    Action:</b> " + polkit_action["description"]
                if "id"          in polkit_action: rofi_message = rofi_message + "\n<b> Action ID:</b> " + polkit_action["id"] 
                if "vendor name" in polkit_action: rofi_message = rofi_message + "\n<b>    Vendor:</b> " + polkit_action["vendor name"] 
                if "vendor url"  in polkit_action: rofi_message = rofi_message + "\n<b>Vendor URL:</b> " + polkit_action["vendor url"]
            await open_rofi_password()
        elif jsonObj["action"] == "show info":
            if "text" in jsonObj:
                await log_notify_message_in_rofi("info: " + jsonObj["text"])
        elif jsonObj["action"] == "show error":
            if "text" in jsonObj:
                await log_notify_message_in_rofi("error: " + jsonObj["text"])
        elif jsonObj["action"] == "authorization response":
            if "authorized" in jsonObj:
                if jsonObj["authorized"]:
                    kill_rofi()
                    break
                else:
                    await log_notify_message_in_rofi("error: authentication failed")
       
            

asyncio.run(main())



