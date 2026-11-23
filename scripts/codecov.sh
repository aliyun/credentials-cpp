#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
build_dir="${1:-build/coverage}"
mkdir -p "$build_dir"
build_dir=$(cd "$build_dir" && pwd)
profiles=$(mktemp -d "$build_dir/profiles.XXXXXX")
trap 'rm -rf "$profiles"' EXIT

cmake -S . -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER="${CC:-clang}" \
  -DCMAKE_CXX_COMPILER="${CXX:-clang++}" \
  -DBUILD_SHARED_LIBS=ON \
  -DENABLE_UNIT_TESTS=ON \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build "$build_dir" --target alibabacloud_credentialsTest --parallel "${BUILD_JOBS:-2}"
LLVM_PROFILE_FILE="$profiles/%m-%p.profraw" ctest --test-dir "$build_dir" \
  --output-on-failure --no-tests=error

case "$(uname -s)" in
  Darwin) library="$build_dir/src/libalibabacloud_credentials.dylib" ;;
  Linux) library="$build_dir/src/libalibabacloud_credentials.so" ;;
  *) echo "Coverage is supported on Linux and macOS." >&2; exit 1 ;;
esac
llvm-profdata merge -sparse "$profiles"/*.profraw -o "$build_dir/coverage.profdata"
llvm-cov export "$library" -object "$build_dir/tests/alibabacloud_credentialsTest" \
  -instr-profile="$build_dir/coverage.profdata" -format=lcov "$PWD/src" "$PWD/include" \
  > "$build_dir/coverage.lcov"
