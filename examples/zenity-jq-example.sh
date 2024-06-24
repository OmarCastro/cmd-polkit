#! /bin/sh

if test "$1" != '__internal_call'; then

    prepParams() { for i in "$@"; do printf "'%s' " "$i"; done }
    parameters="$(prepParams "$@")"

    exec cmd-polkit-agent -p -c "$0 __internal_call $parameters*"
else
    shift 1

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

            # Cancel authentication if no password was given,
            # otherwise respond with given password
            if test -z "$response"
            then echo '{"action":"cancel"}'
            else echo "{\"action\":\"authenticate\",\"password\": \"$response\"}"
            fi
        fi
    done
fi