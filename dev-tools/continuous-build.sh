#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

PROJECT_DIR=$(pwd)

rm -rf build
mkdir build
cd build
meson --prefix=/usr ..
cd ..
while inotifywait -e close_write $PROJECT_DIR/src; do (cd $PROJECT_DIR/build; ninja); done
