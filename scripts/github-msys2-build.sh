#!/bin/bash

set -xe

git submodule update --init --recursive

export CC=gcc
export CXX=g++

export BUILD_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
export CMAKE_BUILD_ROOT="$(echo $BUILD_ROOT | sed 's|^/\([a-z]\)|\1:|g')"

# build ffmpeg
scripts/build-ffmpeg.sh .

# build opus
rm -rf opus
git clone https://github.com/xiph/opus.git && cd opus && git checkout ad8fe90db79b7d2a135e3dfd2ed6631b0c5662ab
mkdir build && cd build
cmake \
	-G Ninja \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="$CMAKE_BUILD_ROOT/opus-prefix" \
	..
ninja
ninja install

cd $BUILD_ROOT

wget https://download.firedaemon.com/FireDaemon-OpenSSL/openssl-1.1.1s.zip && 7z x openssl-1.1.*.zip

wget https://www.libsdl.org/release/SDL2-devel-2.26.2-VC.zip && 7z x SDL2-devel-2.26.2-VC.zip

export CMAKE_SDL_ROOT="$CMAKE_BUILD_ROOT/SDL2-2.26.2"

echo "set(SDL2_INCLUDE_DIRS \"$CMAKE_SDL_ROOT/include\")
set(SDL2_LIBRARIES \"$CMAKE_SDL_ROOT/lib/x64/SDL2.lib\")
set(SDL2_LIBDIR \"$CMAKE_SDL_ROOT/lib/x64\")
include($CMAKE_SDL_ROOT/cmake/sdl2-config-version.cmake)" > "$CMAKE_SDL_ROOT/SDL2Config.cmake"


mkdir protoc && cd protoc
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protoc-3.9.1-win64.zip && 7z x protoc-3.9.1-win64.zip

export PATH="$PWD/bin:$PATH"

PYTHON=c:/hostedtoolcache/windows/Python/3.9.13/x64/python.exe
${PYTHON} -m pip install protobuf==3.19.5

COPY_DLLS="$BUILD_ROOT/openssl-1.1/x64/bin/libcrypto-1_1-x64.dll $BUILD_ROOT/openssl-1.1/x64/bin/libssl-1_1-x64.dll $CMAKE_SDL_ROOT/lib/x64/SDL2.dll"

echo "-- Configure"

cd $BUILD_ROOT
rm -rf build
mkdir build && cd build

cmake \
    -G Ninja \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_PREFIX_PATH="$CMAKE_BUILD_ROOT/ffmpeg-prefix;$CMAKE_BUILD_ROOT/opus-prefix;$CMAKE_BUILD_ROOT/openssl-1.1/x64" \
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

# copy dlls
cp -v $COPY_DLLS ChiakiBorealis
cp -v /mingw64/bin/libiconv-2.dll ChiakiBorealis
cp -v /mingw64/bin/libwinpthread-1.dll ChiakiBorealis
cp -v /mingw64/bin/libgcc_s_seh-1.dll ChiakiBorealis
cp -v /mingw64/bin/libstdc++-6.dll ChiakiBorealis
cp -v /mingw64/bin/glfw3.dll ChiakiBorealis

# copy resources
mkdir ChiakiBorealis/gui2
cp -vr gui2/res ChiakiBorealis/gui2
cp -vr gui2/chiaki.conf ChiakiBorealis/

ls -alh ChiakiBorealis
