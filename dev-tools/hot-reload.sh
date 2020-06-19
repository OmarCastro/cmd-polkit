#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

PROJECT_DIR=$(pwd)

rm -rf build
mkdir build
cd build
meson --prefix=/usr ..
cd ..

current_pid=""

function kill_running_app {
	if [[ "$current_pid" != "" ]]; then
		echo "killing running app with pid $current_pid"
		kill $current_pid;
		wait $current_pid;
	fi
}

function reload {
	kill_running_app
	$PROJECT_DIR/build/cmd-polkit-agent -sv -c "python scripts/rofi-example.py" &
	current_pid=$! 
	echo "app running with pid $current_pid"

}

function build_and_reload {
	cd $PROJECT_DIR/build && ninja
	reload
}

function handle_exit {
	kill_running_app
}

trap handle_exit EXIT

build_and_reload
while inotifywait -e close_write $PROJECT_DIR/src; do
	build_and_reload
done
