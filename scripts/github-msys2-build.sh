#!/bin/bash

set -xe

export CC=gcc
export CXX=g++

export BUILD_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"

# build ffmpeg
scripts/build-ffmpeg.sh .

# build opus
rm -rf opus
git clone https://github.com/xiph/opus.git && cd opus && git checkout ad8fe90db79b7d2a135e3dfd2ed6631b0c5662ab
mkdir build && cd build
cmake \
	-G Ninja \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="$BUILD_ROOT/opus-prefix" \
	..
ninja
ninja install

cd $BUILD_ROOT

wget https://download.firedaemon.com/FireDaemon-OpenSSL/openssl-1.1.1s.zip && 7z x openssl-1.1.*.zip

wget https://www.libsdl.org/release/SDL2-devel-2.26.2-VC.zip && 7z x SDL2-devel-2.26.2-VC.zip

export SDL_ROOT="$BUILD_ROOT/SDL2-2.26.2"

echo "set(SDL2_INCLUDE_DIRS \"$SDL_ROOT/include\")
set(SDL2_LIBRARIES \"$SDL_ROOT/lib/x64/SDL2.lib\")
set(SDL2_LIBDIR \"$SDL_ROOT/lib/x64\")
include($SDL_ROOT/cmake/sdl2-config-version.cmake)" > "$SDL_ROOT/SDL2Config.cmake"


mkdir protoc && cd protoc
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protoc-3.9.1-win64.zip &&
7z x protoc-3.9.1-win64.zip

cd $BUILD_ROOT

export PATH="$PWD/protoc/bin:$PATH"

PYTHON="python"
pip install protobuf==3.19.5

COPY_DLLS="$BUILD_ROOT/openssl-1.1/x64/bin/libcrypto-1_1-x64.dll $BUILD_ROOT/openssl-1.1/x64/bin/libssl-1_1-x64.dll $SDL_ROOT/lib/x64/SDL2.dll"

echo "-- Configure"

rm -rf build
mkdir build && cd build

cmake \
    -G Ninja \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_PREFIX_PATH="$BUILD_ROOT/ffmpeg-prefix;$BUILD_ROOT/opus-prefix;$BUILD_ROOT/openssl-1.1/x64" \
	-DPYTHON_EXECUTABLE="$PYTHON" \
	-DCHIAKI_ENABLE_TESTS=OFF \
	-DCHIAKI_ENABLE_PI_DECODER=OFF \
	-DCHIAKI_USE_SYSTEM_NANOPB=OFF \
	-DCHIAKI_ENABLE_GUI=OFF \
	-DCHIAKI_ENABLE_GUI2=ON \
	-DCHIAKI_ENABLE_SETSU=OFF \
	-DCHIAKI_ENABLE_CLI=OFF \
	-DCHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER=ON \
	..

echo "-- Build"

ninja

cd $BUILD_ROOT

echo "-- Deploy"

mkdir ChiakiBorealis && cp build/gui2/chiaki.exe ChiakiBorealis

cp -v $COPY_DLLS ChiakiBorealis

ls -alh ChiakiBorealis
