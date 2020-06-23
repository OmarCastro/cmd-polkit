#!/usr/bin/env python

import json
import sys
import subprocess

message = ""
rofi_message = message
prompt = "password: "

def open_rofi():
    cmd = ['rofi', '-password', '-dmenu', '-p', prompt, '-mesg', rofi_message ]
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    out, err = p.communicate();
    if out == b'':
        print( """{"action": "cancel"}""" )
        exit(0)
    else:
        print( """{"action": "authenticate", "password": "%s"}""" % str(out, "utf-8").rstrip("\n"), flush=True)


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
        open_rofi()
    #elif jsonObj["action"] == "authorization response":
    #    if "authorized" in jsonObj:
    #        if jsonObj["authorized"]:
    #            break
    #        else:
    #            rofi_message = message + "\n password failed"
    #            open_rofi()




