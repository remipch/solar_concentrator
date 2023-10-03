cmake_minimum_required(VERSION 3.0)

project(quirc_02 LANGUAGES C)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

include_directories(lib)

add_executable(quirc_02
    lib/decode.c
    lib/identify.c
    lib/quirc.c
    lib/version_db.c
#     main.c
    tests/dbgutil.c
#     tests/inspect.c
    tests/qrtest.c)

target_link_libraries(quirc_02 ${JPEG_LIBRARIES} ${PNG_LIBRARIES}  m)

# install(TARGETS quirc_02 RUNTIME DESTINATION bin)
