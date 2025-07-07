#! /bin/sh

<< ////

This example expects you to have fprint setup. Make sure you follow the instructions for your distro.

Setup
======

This example expects you to have fprint setup. Make sure you follow the instructions for your distro.
Generally you should have it setup with "fprintd-enroll" command, pointed to a finger choosen
as an argument of the executable

I leave are some links for some distros I found while learning how to configure it on my machine

Linux Mint, last seen on 2025-06-26: https://forums.linuxmint.com/viewtopic.php?t=408129
Arch Linux, last seen on 2025-06-26: https://wiki.archlinux.org/title/Fprint

After setup with fprint you will need to setup PAM on polkit. 
The polkit PAM configuration is in "/etc/pam.d/polkit-1" file

Depending on each distro you can configure in 1 or 2 ways:

1. With "pam_fprintd.so": 
  Prompts for a password first; if you press Enter on empty password, it will proceed to fingerprint authentication. 

2. With "pam_fprintd_grosshack.so": 
  Prompts for a password and fingerprint at the same time, This may be needed for some graphical programs which
  do not allow blank password input, such as Gnome's built-in polkit agent.


This example was made for "pam_fprintd_grosshack.so" config, so you can apply the configuration by adding the following 2 lines
at the top of /etc/pam.d/polkit-1:

"""
auth            sufficient      pam_fprintd_grosshack.so
auth            sufficient      pam_unix.so try_first_pass nullok
...
"""

If /etc/pam.d/polkit-1 does not exist, you copy from /usr/lib/pam.d/polkit-1 first before adding the previous 2 lines.
Note: pam_fprintd_grosshack.so may be located in a different package, so you may need to install it first.

Once again, if it fails or if there is any issue regarding polkit configuration, check the instructions on how to configure
for your distro.


How to use this example
=======================

1. run this executable
2. run any command that requires polkit autentication (e.g. "pkexec echo 123")
3. When zenity dialog opens do one of the following:
  - write the password and press Enter
  - put the finger on the fingerprint scanner
3a. If it fails, an the error message appear in rofi, do the step again
5. Fingerprint scanned successfully and the authentication is successful!



Have fun
Omar Castro


////


if test "$1" != '__internal_call'; then

    prepParams() { for i in "$@"; do printf "'%s' " "$i"; done }
    parameters="$(prepParams "$@")"

    exec cmd-polkit-agent -sv -c "$0 __internal_call $parameters*"
else
    shift 1

    ZEN_PID=""
    LOGS=""
    PROMPT=""
    MESSAGE=""

    function list_child_pids_of() { 
        for f in /proc/$1/task/*/children;
            do l=""; 
            [ -r $f ] &&read l<$f;
            for p in $l;
                do echo $p; 
                list_child_pids_of $p; 
            done; 
        done; 
    }


    function close_dialog {
        if [ -n "$ZEN_PID" ]; then
            child_proc_list="$(list_child_pids_of $ZEN_PID)"
            kill "$ZEN_PID"
            wait $ZEN_PID
            echo $child_proc_list | xargs -r kill

        fi
    }


    function open_dialog {
        if [ -n "$LOGS" ]; then
            message_text="$MESSAGE\n\nLOGS:\n$LOGS\n$PROMPT"
        else
            message_text="$MESSAGE\n\n$PROMPT"
        fi
        response="$(zenity --entry --hide-text --text="$message_text" --title="Authentication required")"
        exit_code=$?
        if [ $exit_code -ne 0 ]
        then echo '{"action":"cancel"}'
        elif test -z "$response"; then echo '{"action":"authenticate","password": ""}'
        else echo "{\"action\":\"authenticate\",\"password\": \"$response\"}"
        fi

    }

    function reopen_dialog {
        close_dialog
        (open_dialog) &
        ZEN_PID=$!
    }


    # Read incoming messages one by one (line by line)
    # from stdin to variable $msg
    while read -r msg; do
        if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
        then
            PROMPT="$( printf '%s' "$msg" | jq -rc '.prompt // "Password:"')"
            MESSAGE="$(printf '%s' "$msg" | jq -rc '.message')"
            MESSAGE="$MESSAGE$(printf '%s' "$msg" | jq -rc 'if .["polkit action"].description  then "\n\n Action: \t\t" + .["polkit action"].description else empty end')"
            MESSAGE="$MESSAGE$(printf '%s' "$msg" | jq -rc 'if .["polkit action"].id             then "\n Action ID: \t\t" + .["polkit action"].id else empty end')"
            MESSAGE="$MESSAGE$(printf '%s' "$msg" | jq -rc 'if .["polkit action"]["vendor name"] then "\n Vendor: \t\t" + .["polkit action"]["vendor name"] else empty end')"
            MESSAGE="$MESSAGE$(printf '%s' "$msg" | jq -rc 'if .["polkit action"]["vendor url"]  then "\n Vendor URL: \t" + .["polkit action"]["vendor url"] else empty end')"

            reopen_dialog
        elif echo "$msg" | jq -e '.action == "show info"' 1>/dev/null 2>/dev/null; then
            LOGS="${LOGS}info: $(echo "$msg" | jq '.text')
"
            reopen_dialog
        elif echo "$msg" | jq -e '.action == "show error"' 1>/dev/null 2>/dev/null; then
            LOGS="${LOGS}error: $(echo "$msg" | jq '.text')
"
            reopen_dialog
        elif echo "$msg" | jq -e '.action == "authorization response"' 1>/dev/null 2>/dev/null; then
            close_dialog
            exit 0
        fi
    done
fi
