#! /bin/sh

if test "$1" != '__internal_call'; then

    prepParams() { for i in "$@"; do printf "'%s' " "$i"; done }
    parameters="$(prepParams "$@")"

    exec cmd-polkit-agent -sv -c "$0 __internal_call $parameters*"
else
    shift 1

    tmpdir=
    function cleanup () {
    trap - EXIT
    if [ -n "$tmpdir" ] ; then rm -rf "$tmpdir"; fi
    if [ -n "$1" ]; then trap - $1; kill -$1 $$; fi
    }
    tmpdir=$(mktemp -d)
    trap 'cleanup' EXIT
    trap 'cleanup HUP' HUP
    trap 'cleanup TERM' TERM
    trap 'cleanup INT' INT
    mkfifo "$tmpdir/pipe"

    ZEN_PID=""
    LOGS=""

    function reinit_zenity {
        if [ -n "$ZEN_PID" ]; then
            kill $ZEN_PID
        fi
        zenity --text-info <<< "$LOGS" &
        ZEN_PID=$!
    }


    # Read incoming messages one by one (line by line)
    # from stdin to variable $msg
    while read -r msg; do
        if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
        then
            prompt="$( printf '%s' "$msg" | jq -rc '.prompt // "Password:"')"
            message="$(printf '%s' "$msg" | jq -rc '.message')"
            message="$message$(printf '%s' "$msg" | jq -rc 'if .["polkit action"].description  then "\n\n Action: \t\t" + .["polkit action"].description else empty end')"
            message="$message$(printf '%s' "$msg" | jq -rc 'if .["polkit action"].id             then "\n Action ID: \t\t" + .["polkit action"].id else empty end')"
            message="$message$(printf '%s' "$msg" | jq -rc 'if .["polkit action"]["vendor name"] then "\n Vendor: \t\t" + .["polkit action"]["vendor name"] else empty end')"
            message="$message$(printf '%s' "$msg" | jq -rc 'if .["polkit action"]["vendor url"]  then "\n Vendor URL: \t" + .["polkit action"]["vendor url"] else empty end')"


            response="$(zenity --entry --hide-text --text="$message\n\n$prompt" --title="Authentication required")"
            exit_code=$?

            # Cancel authentication if no password was given,
            # otherwise respond with given password
            if [ $exit_code -ne 0 ]
            then echo '{"action":"cancel"}'
            elif test -z "$response"; then echo '{"action":"authenticate","password": ""}'
            else echo "{\"action\":\"authenticate\",\"password\": \"$response\"}"
            fi
        elif echo "$msg" | jq -e '.action == "show info"' 1>/dev/null 2>/dev/null; then
            LOGS="$LOGS
info: $(echo "$msg" | jq '.text')"
            reinit_zenity
        elif echo "$msg" | jq -e '.action == "show error"' 1>/dev/null 2>/dev/null; then
            LOGS="$LOGS
error: $(echo "$msg" | jq '.text')"
            reinit_zenity
        elif echo "$msg" | jq -e '.action == "authorization response"' 1>/dev/null 2>/dev/null; then
            if [ -n "$ZEN_PID" ]; then
                kill $ZEN_PID
            fi
        fi
    done
fi
