cmake_minimum_required(VERSION 3.0)

project(quirc)

# apt-get install libopencv-dev
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# apt-get install libsdl2-dev
#find_package(SDL2 REQUIRED)
#include_directories(${SDL2_INCLUDE_DIRS})

# apt-get install libsdl2-gfx-dev
#find_package(SDL2_gfx REQUIRED)
#include_directories(${SDL2_GFX_INCLUDE_DIRS})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

include_directories(lib)

add_library(capstone_detector
    lib/identify.c
    lib/decode.c
    lib/version_db.c
    lib/quirc.c)

target_link_libraries(capstone_detector)

add_executable(inspect
    tests/dbgutil.c
    tests/inspect_opencv.cxx)

target_link_libraries(inspect capstone_detector ${OpenCV_LIBS} ${JPEG_LIBRARIES} ${PNG_LIBRARIES}  m)

add_executable(qrtest
    tests/dbgutil.c
    tests/qrtest.c)

target_link_libraries(qrtest capstone_detector ${JPEG_LIBRARIES} ${PNG_LIBRARIES}  m)
