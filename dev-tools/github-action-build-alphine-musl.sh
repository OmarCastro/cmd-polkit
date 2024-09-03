#!/usr/bin/env sh
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$DIR/.."

meson setup build-alphine-musl -Db_coverage=true --warnlevel 2 || exit 1
meson compile -C build-alphine-musl || exit 1
meson test --wrap='valgrind --leak-check=full' -C build-alphine-musl
ninja coverage -C build-alphine-musl