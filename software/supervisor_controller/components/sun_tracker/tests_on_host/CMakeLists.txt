project(sun_tracker_state_machine_test)

# WTF: even if CMAKE_BUILD_TYPE=DEBUG
# NDEBUG must be undefined
# otherwise source line and file are not printed is 'assert' fails :
add_definitions(-U NDEBUG)

# libjpeg is used to load test images (internally in CImg)
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
add_compile_definitions(cimg_use_jpeg=1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/spot_on_center.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/spot_on_center.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/spot_on_top_right_1.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/spot_on_top_right_1.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/spot_on_top_right_2.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/spot_on_top_right_2.jpg COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/spot_on_top_right_3.jpg
               ${CMAKE_CURRENT_BINARY_DIR}/spot_on_top_right_3.jpg COPYONLY)

add_executable(sun_tracker_logic_test sun_tracker_logic_test.cpp
                                      ../sun_tracker_logic)

add_executable(sun_tracker_state_machine_test sun_tracker_state_machine_test.cpp
                                              ../sun_tracker_state_machine.cpp)

include_directories(
    .. ../include ../../image/include ../../target_detector/include
    ../../camera/include ../../motors/include)

target_link_libraries(sun_tracker_logic_test ${JPEG_LIBRARIES})

target_link_libraries(sun_tracker_state_machine_test ${JPEG_LIBRARIES})

# Auto populate the tests from test source file
# Note : CMake must be reconfigured if tests are added/renamed/removed from test source file
# TODO : move this in a common cmake function for reuse
set(TEST_REGEX "^TEST\\(([A-Za-z0-9_]+).*")
file(STRINGS sun_tracker_state_machine_test.cpp detected_tests
     REGEX ${TEST_REGEX})
foreach(test ${detected_tests})
    string(REGEX REPLACE ${TEST_REGEX} "\\1" test ${test})
    message(STATUS "detected test ${test}")
    add_test(NAME sun_tracker_state_machine_test_${test}
             COMMAND sun_tracker_state_machine_test ${test})
endforeach()

file(STRINGS sun_tracker_logic_test.cpp detected_tests REGEX ${TEST_REGEX})
foreach(test ${detected_tests})
    string(REGEX REPLACE ${TEST_REGEX} "\\1" test ${test})
    message(STATUS "detected test ${test}")
    add_test(NAME sun_tracker_logic_test_${test} COMMAND sun_tracker_logic_test
                                                         ${test})
endforeach()
