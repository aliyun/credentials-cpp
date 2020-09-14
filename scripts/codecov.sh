#!/bin/bash

currpath=$(dirname "$0")
basepath=$(cd "$currpath/../" || exit; pwd)

cd "$basepath/" || exit

mkdir -p cmake_build/
cd "cmake_build/" || exit
cmake -DENABLE_UNIT_TESTS=ON -DENABLE_COVERAGE=ON ..
cmake --build .
./tests

utdir="$basepath/cmake_build"

cd "$utdir/" || exit

echo '--------- generate initial info ---------------- '
mkdir -p coverage
lcov -z
lcov --no-external --initial --capture --output-file ./coverage/init.info || exit

echo '--------- run test ---------------- '
ctest --verbose --coverage

echo '--------- generate post info ---------------- '
lcov -c -o ./coverage/coverage.info || exit
lcov --list ./coverage/coverage.info

echo '--------- generate html report ---------------- '
genhtml -o coverage --prefix="$utdir" init.info coverage.info  || exit

echo 'check report: ' "$basepath/coverage/index.html"

echo ' ------remove tmp file ------'

rm cmake_build/coverage
