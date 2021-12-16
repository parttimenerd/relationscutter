#! /bin/sh

cd "$(dirname "$0")"
./build_protobuf.sh
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 4
cd -