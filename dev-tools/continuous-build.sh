#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

PROJECT_DIR=$(pwd)

build_project() {
    cd $PROJECT_DIR/build-test
    meson build
    meson test
    ninja coverage-html
}

rm -rf build-test
meson setup build-test -Db_coverage=true --reconfigure
build_project

while inotifywait -e close_write $PROJECT_DIR/src $PROJECT_DIR/test; do 
    build_project
done
