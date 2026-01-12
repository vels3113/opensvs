#!/bin/bash

./configure.sh --preset debug
cmake --build build/debug -j$(nproc)

FILES=$(find . -type f ! -path "./build/*" -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' ); 

if [[ -z "$FILES" ]]; then
  echo "No C++ files to analyze.";
  exit 0;
fi;

clang-tidy -p build/debug --quiet $FILES --fix --format-style=file > clang-tidy-report.txt
echo "Clang-Tidy analysis complete. Report saved to clang-tidy-report.txt."