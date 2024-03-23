#! /bin/sh

while read -r msg; do
    #  --- shellcheck disable=SC2210
    if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
    then
        echo "{\"action\":\"authenticate\",\"password\": \"test\"}"
    fi
done