#!/bin/bash

set -xe

apk add git cmake ninja protoc py3-protobuf py3-setuptools opus-dev ffmpeg-dev sdl2-dev gcc g++ openssl-dev glfw-dev glm-dev

rm -rf build

cmake -Bbuild -GNinja -DCHIAKI_ENABLE_GUI=OFF -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_GUI2=ON -DCHIAKI_ENABLE_PI_DECODER=OFF -DCHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER=ON

ninja -C build

mkdir ChiakiBorealis && cp build/gui2/chiaki ChiakiBorealis

# copy resources
mkdir ChiakiBorealis/gui2
cp -vr gui2/res ChiakiBorealis/gui2
cp -vr scripts/chiaki.conf ChiakiBorealis/

ls -alh ChiakiBorealis