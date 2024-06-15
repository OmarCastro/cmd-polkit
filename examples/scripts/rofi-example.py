#!/usr/bin/env python

import json
import sys
import subprocess

message = ""
rofi_message = message
prompt = "password: "

def open_rofi():
    cmd = ['rofi', '-password', '-dmenu', '-markup', '-no-fixed-num-lines', '-p', prompt, '-mesg', rofi_message ]
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
        if "polkit action" in jsonObj and jsonObj["polkit action"] != None:
            polkit_action = jsonObj["polkit action"]
            rofi_message = rofi_message + "\n\n"
            if "description" in polkit_action: rofi_message = rofi_message + "\n<b>    Action:</b> " + polkit_action["description"]
            if "id"          in polkit_action: rofi_message = rofi_message + "\n<b> Action ID:</b> " + polkit_action["id"] 
            if "vendor name" in polkit_action: rofi_message = rofi_message + "\n<b>    Vendor:</b> " + polkit_action["vendor name"] 
            if "vendor url"  in polkit_action: rofi_message = rofi_message + "\n<b>Vendor URL:</b> " + polkit_action["vendor url"]
        open_rofi()
    #elif jsonObj["action"] == "authorization response":
    #    if "authorized" in jsonObj:
    #        if jsonObj["authorized"]:
    #            break
    #        else:
    #            rofi_message = message + "\n password failed"
    #            open_rofi()




