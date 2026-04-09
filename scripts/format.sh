#!/bin/bash

DIR_SCRIPT="$(dirname $0)"
DIR_TOP="$DIR_SCRIPT/.."

pushd "$DIR_TOP"

echo "Formatting C++ files using clang-format..."

find include tests src \
    -name "*.cpp" -o \
    -name "*.cc" -o \
    -name "*.c" -o \
    -name "*.hpp" -o \
    -name "*.h" \
    2>/dev/null | xargs clang-format -i -style=file

echo "Formatting complete."

popd