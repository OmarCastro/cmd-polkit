#! /bin/sh

retry=0

while read -r msg; do
    #  --- shellcheck disable=SC2210
    if echo "$msg" | jq -e '.action == "request password"' 1>/dev/null 2>/dev/null
    then 
        if [ "$retry" -gt "2" ]; then
                echo "{\"action\":\"authenticate\",\"password\": \"success\"}"
        else
            ((retry+=1))
            echo "{\"action\":\"authenticate\",\"password\": \"fail\"}"
        fi
    fi
done