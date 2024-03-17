#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

meson setup build-test -Db_coverage=true || exit 1
meson compile -C build-test || exit 1
meson test -C build-test
ninja coverage -C build-test
dev-tools/run docs