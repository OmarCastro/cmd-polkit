#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}")"
../build/cmd-polkit-agent -sv -c "python scripts/rofi-example.py"
