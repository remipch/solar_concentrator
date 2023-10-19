
# Enable CTest features (provides 'add_test' command below)
# enable_testing()

project(target_detector_test)

# WTF: even if CMAKE_BUILD_TYPE=DEBUG
# NDEBUG must be undefined
# otherwise source line and file are not printed is 'assert' fails :
add_definitions(-U NDEBUG)

# libjpeg is used to load test images (internally in CImg)
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
add_compile_definitions(cimg_use_jpeg=1)
# find_package(PNG REQUIRED)
# include_directories(${PNG_INCLUDE_DIRS})
# add_compile_definitions(cimg_use_png=1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/missing_capstones.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/missing_capstones.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/misplaced_capstones.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/misplaced_capstones.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/misaligned_capstones.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/misaligned_capstones.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/correct_capstones.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/correct_capstones.jpg COPYONLY)

add_executable( target_detector_test
    target_detector_test.cpp
    ../target_detector.cpp
    ../capstone_detector/quirc.c
    ../capstone_detector/identify.c
)

include_directories(
    ../include
    ../capstone_detector/
    ../../image/include
)

target_link_libraries(target_detector_test ${JPEG_LIBRARIES})

# Auto populate the tests from test source file
# Note : CMake must be reconfigured if tests are added/renamed/removed from test source file
# TODO : move this in a common cmake function for reuse
set(TEST_REGEX "^TEST\\(([A-Za-z0-9_]+).*")
file(STRINGS target_detector_test.cpp detected_tests REGEX ${TEST_REGEX})
foreach(test ${detected_tests})
    STRING(REGEX REPLACE ${TEST_REGEX} "\\1" test ${test})
    message(STATUS "detected test ${test}")
    add_test(NAME target_detector_test_${test} COMMAND target_detector_test ${test})
endforeach()
