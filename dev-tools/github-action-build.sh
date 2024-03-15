#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
cd ..

meson setup build-test -Db_coverage=true --reconfigure
meson build -C build-test
meson test -C build-test
ninja coverage-html -C build-test
dev-tools/run docs