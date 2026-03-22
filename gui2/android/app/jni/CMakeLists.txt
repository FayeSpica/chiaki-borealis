cmake_minimum_required(VERSION 3.16)

project(CHIAKI_BOREALIS_ANDROID)

set(CMAKE_CXX_STANDARD 17)

# Paths
set(CHIAKI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
set(GUI2_ROOT "${CHIAKI_ROOT}/gui2")
set(BOREALIS_ROOT "${GUI2_ROOT}/borealis")
set(SDL2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL")

# ====== SDL2 ======
add_subdirectory(${SDL2_SOURCE_DIR} SDL2)

# ====== Prebuilt OpenSSL (from wiliwili) ======
set(OPENSSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openssl-android")
set(OPENSSL_ROOT_DIR "${OPENSSL_DIR}")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_DIR}/include")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_DIR}/lib/${ANDROID_ABI}/libssl.a")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_DIR}/lib/${ANDROID_ABI}/libcrypto.a")
set(OPENSSL_FOUND TRUE)

# Create imported targets for OpenSSL
add_library(OpenSSL::SSL STATIC IMPORTED)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
)
add_library(OpenSSL::Crypto STATIC IMPORTED)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
)

# ====== Borealis library ======
set(BOREALIS_LIB_SOURCES
    ${BOREALIS_ROOT}/library/lib/application.cpp
    ${BOREALIS_ROOT}/library/lib/absolute_layout.cpp
    ${BOREALIS_ROOT}/library/lib/animations.cpp
    ${BOREALIS_ROOT}/library/lib/applet_frame.cpp
    ${BOREALIS_ROOT}/library/lib/box_layout.cpp
    ${BOREALIS_ROOT}/library/lib/button.cpp
    ${BOREALIS_ROOT}/library/lib/crash_frame.cpp
    ${BOREALIS_ROOT}/library/lib/dialog.cpp
    ${BOREALIS_ROOT}/library/lib/dropdown.cpp
    ${BOREALIS_ROOT}/library/lib/header.cpp
    ${BOREALIS_ROOT}/library/lib/hint.cpp
    ${BOREALIS_ROOT}/library/lib/i18n.cpp
    ${BOREALIS_ROOT}/library/lib/image.cpp
    ${BOREALIS_ROOT}/library/lib/label.cpp
    ${BOREALIS_ROOT}/library/lib/layer_view.cpp
    ${BOREALIS_ROOT}/library/lib/list.cpp
    ${BOREALIS_ROOT}/library/lib/logger.cpp
    ${BOREALIS_ROOT}/library/lib/material_icon.cpp
    ${BOREALIS_ROOT}/library/lib/notification_manager.cpp
    ${BOREALIS_ROOT}/library/lib/popup_frame.cpp
    ${BOREALIS_ROOT}/library/lib/progress_display.cpp
    ${BOREALIS_ROOT}/library/lib/progress_spinner.cpp
    ${BOREALIS_ROOT}/library/lib/rectangle.cpp
    ${BOREALIS_ROOT}/library/lib/repeating_task.cpp
    ${BOREALIS_ROOT}/library/lib/scroll_view.cpp
    ${BOREALIS_ROOT}/library/lib/sidebar.cpp
    ${BOREALIS_ROOT}/library/lib/staged_applet_frame.cpp
    ${BOREALIS_ROOT}/library/lib/style.cpp
    ${BOREALIS_ROOT}/library/lib/swkbd.cpp
    ${BOREALIS_ROOT}/library/lib/tab_frame.cpp
    ${BOREALIS_ROOT}/library/lib/table.cpp
    ${BOREALIS_ROOT}/library/lib/task_manager.cpp
    ${BOREALIS_ROOT}/library/lib/theme.cpp
    ${BOREALIS_ROOT}/library/lib/thumbnail_frame.cpp
    ${BOREALIS_ROOT}/library/lib/view.cpp
    ${BOREALIS_ROOT}/library/lib/extern/nanovg/nanovg.c
    ${BOREALIS_ROOT}/library/lib/extern/libretro-common/features/features_cpu.c
    ${BOREALIS_ROOT}/library/lib/extern/libretro-common/encodings/encoding_utf.c
    ${BOREALIS_ROOT}/library/lib/extern/libretro-common/compat/compat_strl.c
    ${BOREALIS_ROOT}/library/lib/extern/libretro-common/streams/file_stream.c
    ${BOREALIS_ROOT}/library/lib/extern/fmt/src/format.cc
    ${BOREALIS_ROOT}/library/lib/extern/fmt/src/os.cc
)

add_library(borealis STATIC ${BOREALIS_LIB_SOURCES})
target_include_directories(borealis PUBLIC
    ${BOREALIS_ROOT}/library/include
    ${BOREALIS_ROOT}/library/include/borealis/extern
    ${BOREALIS_ROOT}/library/include/borealis/extern/glad
    ${BOREALIS_ROOT}/library/include/borealis/extern/nanovg
    ${BOREALIS_ROOT}/library/include/borealis/extern/libretro-common
    ${BOREALIS_ROOT}/library/lib/extern/fmt/include
    ${BOREALIS_ROOT}/library/lib/extern/libretro-common/include
    ${BOREALIS_ROOT}/library/lib/extern/glm
)
target_link_libraries(borealis PUBLIC SDL2)
target_compile_definitions(borealis PUBLIC
    __SDL2__
    ANDROID
    BOREALIS_RESOURCES="/sdcard/chiaki/res/"
)

# ====== chiaki-lib ======
# Disable OpenSSL ExternalProject, use prebuilt instead
set(CHIAKI_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(CHIAKI_ENABLE_CLI OFF CACHE BOOL "" FORCE)
set(CHIAKI_ENABLE_GUI2 OFF CACHE BOOL "" FORCE)
set(CHIAKI_ENABLE_ANDROID OFF CACHE BOOL "" FORCE)
set(CHIAKI_ENABLE_FFMPEG_DECODER OFF CACHE BOOL "" FORCE)
set(CHIAKI_ENABLE_PI_DECODER OFF CACHE BOOL "" FORCE)
set(CHIAKI_LIB_ENABLE_OPUS OFF CACHE BOOL "" FORCE)
set(CHIAKI_LIB_OPENSSL_EXTERNAL_PROJECT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CHIAKI_ROOT}/third-party ${CMAKE_CURRENT_BINARY_DIR}/third-party)
add_subdirectory(${CHIAKI_ROOT}/lib ${CMAKE_CURRENT_BINARY_DIR}/chiaki-lib)

# ====== Main shared library ======
add_library(chiaki-borealis SHARED
    ${GUI2_ROOT}/src/main.cpp
    ${GUI2_ROOT}/src/gui.cpp
    ${GUI2_ROOT}/src/io.cpp
    ${GUI2_ROOT}/src/settings.cpp
    ${GUI2_ROOT}/src/host.cpp
    ${GUI2_ROOT}/src/discoverymanager.cpp
)

target_include_directories(chiaki-borealis PRIVATE
    ${GUI2_ROOT}/include
    ${BOREALIS_ROOT}/library/include
    ${BOREALIS_ROOT}/library/include/borealis/extern
    ${BOREALIS_ROOT}/library/lib/extern
    ${BOREALIS_ROOT}/library/lib/extern/fmt/include
    ${OPENSSL_INCLUDE_DIR}
)

target_compile_definitions(chiaki-borealis PRIVATE
    __SDL2__
    ANDROID
    BOREALIS_RESOURCES="/sdcard/chiaki/res/"
    CHIAKI_ENABLE_FFMPEG_DECODER=0
)

find_library(ANDROID_LIB_LOG log)
find_library(ANDROID_LIB_ANDROID android)
find_library(ANDROID_LIB_GLESV3 GLESv3)
find_library(ANDROID_LIB_EGL EGL)

target_link_libraries(chiaki-borealis
    borealis
    chiaki-lib
    OpenSSL::SSL
    OpenSSL::Crypto
    SDL2
    SDL2main
    ${ANDROID_LIB_LOG}
    ${ANDROID_LIB_ANDROID}
    ${ANDROID_LIB_GLESV3}
    ${ANDROID_LIB_EGL}
)
