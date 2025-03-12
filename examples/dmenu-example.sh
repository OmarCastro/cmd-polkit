#!/bin/sh

#
# Requirements: jq, notify-send
#
# If you use dmenu, consider applying this patch and setting MENU='dmenu -P'
#     https://tools.suckless.org/dmenu/patches/password/
#

# Set dmenu program and its options.
# You can use wmenu -P for Wayland.
MENU='dmenu'

notify() {
	[ -z "$MESSAGE" ] && return 1
	[ -n "$NOTIFY_ID" ] && notify-send -h 'int:transient:1' -r "$NOTIFY_ID" "$@" || NOTIFY_ID="$(notify-send -h 'int:transient:1' -p "$@")"
}

notify_hide() {
	[ -n "$NOTIFY_ID" ] && notify -t 0 ''
}

body() {
	jq -rc --arg error "$ERROR" '
		if $error != ""                      then $error + "\n" else empty end,
		if .["polkit action"].description    then .["polkit action"].description else empty end,
		if .["polkit action"].id             then "\n  <i>Action ID</i>: " + .["polkit action"].id else empty end,
		if .["polkit action"]["vendor name"] then "  <i>Vendor</i>: " + .["polkit action"]["vendor name"] else empty end,
		if .["polkit action"]["vendor url"]  then "  <i>Vendor URL</i>: " + .["polkit action"]["vendor url"] else empty end
	'
}

password() {
	prompt="$(printf '%s' "$1" | jq -rc '.prompt // "Password"')"
	MESSAGE="$(printf '%s' "$1" | jq -rc '.message // empty')"
	body="$(printf '%s' "$1" | body)"

	notify -t 300000 "$MESSAGE" "$body"

	# Request a password prompt from dmenu
	response="$($MENU -p "${prompt%': '}" </dev/null)"

	# Cancel authentication if no password was given
	if [ -z "$response" ]; then
		echo '{"action":"cancel"}'
		notify_hide
		return 0
	fi

	# Respond with the provided password
	jq -cn --arg pwd "$response" '{action: "authenticate", password: $pwd}'
}

response() {
	if printf '%s' "$1" | jq -e '.authorized' >/dev/null; then
		notify_hide
	else
		ERROR='<b><span fgcolor="#ab4642">Authentication failed!</span></b>'
		notify -u low "$MESSAGE" 'Authentication failed!'
	fi
}

if [ -z "$POLKIT_DMENU_INTERNAL" ]; then
	export POLKIT_DMENU_INTERNAL=1
	exec cmd-polkit-agent -s -c "$0"
	exit 0
fi

while IFS= read -r msg; do
	action="$(printf '%s' "$msg" | jq -rc '.action')"

	case "$action" in
		'request password')
			password "$msg"
			;;
		'authorization response')
			response "$msg"
			;;
	esac
done
