#! /bin/sh
FILE="test-serial.timings.txt"

while read -r msg; do
    #  --- shellcheck disable=SC2210
    if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
    then
        MESSAGE='`'"$(echo "$msg" | grep -o '/usr/bin/echo [0-9]')"'` auth'
        echo "start $MESSAGE" >> "$FILE"
        sleep 0.25
        echo "  end $MESSAGE" >> "$FILE"
        echo "{\"action\":\"authenticate\",\"password\": \"success\"}"
    fi
done