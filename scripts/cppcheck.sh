#!/bin/bash

./configure.sh --preset debug
cmake --build build/debug -j$(nproc)
cppcheck --project=./build/debug/compile_commands.json --enable=warning,style,performance,portability --std=c++20 --inline-suppr --template=gcc --quiet --suppressions-list=cppcheck.suppress