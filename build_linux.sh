#!/usr/bin/env bash

error_handler() {
    exit_status=$?
    echo "'$BASH_COMMAND' failed on line ${BASH_LINENO[0]} with status $exit_status"
    exit $exit_status
}
trap error_handler ERR

if [ "$1" == "" ]; then
    echo "Build type was not specified, selecting 'Release' one by default..."
    BUILD_TYPE="Release"
else
    echo "Build type specified: $1"
    BUILD_TYPE="$1"
fi
BUILD_TYPE_LOWERCASE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

if [ ! -d "build/$BUILD_TYPE_LOWERCASE" ]; then
    echo "Output directory is absent, creating..."
    mkdir -p "build/$BUILD_TYPE_LOWERCASE"
fi
PWD_DIR=$(pwd)
cd "build/$BUILD_TYPE_LOWERCASE"

cmake ../.. -G "Ninja" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build .

cd "$PWD_DIR"
