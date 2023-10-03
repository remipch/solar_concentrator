cmake_minimum_required(VERSION 3.0)

project(quirc_02 LANGUAGES C)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

include_directories(lib)

add_library(capstone_detector
    lib/decode.c
    lib/identify.c
    lib/version_db.c
    lib/quirc.c)

target_link_libraries(capstone_detector)

add_executable(qrtest
    tests/dbgutil.c
    tests/qrtest.c)

target_link_libraries(qrtest capstone_detector ${JPEG_LIBRARIES} ${PNG_LIBRARIES}  m)
