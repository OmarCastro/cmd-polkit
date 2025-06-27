#!/usr/bin/env python

"""
setup instructions on ../rofi-example-with-fprintd-so.sh
"""

import json
import sys
import subprocess

message = ""
rofi_message = message
rofi_notification_message = ""
prompt = "password: "
message_process = None

def kill_rofi_message():
    global message_process
    if message_process != None:
        message_process.terminate()
        message_process = None

def open_rofi_password():
    kill_rofi_message()
    cmd = ['rofi', '-password', '-dmenu', '-markup', '-no-fixed-num-lines', '-p', prompt, '-mesg', rofi_message ]
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    out, err = p.communicate();
    if out == b'':
        print( """{"action": "cancel"}""" )
        exit(0)
    else:
        print( """{"action": "authenticate", "password": "%s"}""" % str(out, "utf-8").rstrip("\n"), flush=True)

def log_notify_message_in_rofi(text):
    global message_process
    global rofi_notification_message
    kill_rofi_message()
    if rofi_notification_message != "":
        rofi_notification_message += "\n"
    rofi_notification_message += text
    cmd = ['rofi', '-e', rofi_notification_message ]
    message_process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)


for line in sys.stdin:
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
        open_rofi_password()
    elif jsonObj["action"] == "show info":
        if "text" in jsonObj:
            log_notify_message_in_rofi("info: " + jsonObj["text"])
    elif jsonObj["action"] == "show error":
        if "text" in jsonObj:
            log_notify_message_in_rofi("error: " + jsonObj["text"])
    elif jsonObj["action"] == "authorization response":
        if "authorized" in jsonObj:
            if jsonObj["authorized"]:
                kill_rofi_message()
                break
            else:
                log_notify_message_in_rofi("error: authentication failed")




