#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

meson setup build-alphine-musl -Db_coverage=true --reconfigure --warnlevel 2 || exit 1
meson compile -C build-test || exit 1
meson test --wrap='valgrind --leak-check=full' -C build-alphine-musl
ninja coverage -C build-alphine-musl