#!/usr/bin/env bash

# AI CREATED SCRIPT

set -euo pipefail

rm -rf ./build

BUILD_TYPE="Debug"
BUILD_TESTS=false
RUN_APP=false

for arg in "$@"; do
    case "$arg" in
        --release)
            BUILD_TYPE="Release"
            ;;

        --test)
            BUILD_TESTS=true
            ;;

        --run)
            RUN_APP=true
            ;;

        *)
            echo "Usage: $0 [--release] [--test] [--run]"
            exit 1
            ;;
    esac
done

echo "==> Building ryuzaki ($BUILD_TYPE)"

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTING="$BUILD_TESTS"

cmake --build build --parallel

echo
echo "==> Build complete!"
echo "==> Binary: ./build/ryuzaki"

if [[ "$BUILD_TESTS" == true ]]; then
    echo "==> Tests: ./build/ryuzaki_tests"
fi

if [[ "$RUN_APP" == true ]]; then
    echo

    if [[ "$BUILD_TESTS" == true ]]; then
        echo "==> Running tests..."
        ctest --test-dir build --output-on-failure
    else
        echo "==> Running ryuzaki..."
        ./build/ryuzaki
    fi
fi