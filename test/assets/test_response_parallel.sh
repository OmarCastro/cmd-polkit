#! /bin/sh
FILE="test-parallel.timings.txt"

echo "start" >> "$FILE"

while read -r msg; do
    #  --- shellcheck disable=SC2210
    if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
    then
        sleep 0.25
        echo "end" >> "$FILE"
        echo "{\"action\":\"authenticate\",\"password\": \"success\"}"
    fi
done