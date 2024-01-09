project(sun_tracker_state_machine_test)

# WTF: even if CMAKE_BUILD_TYPE=DEBUG
# NDEBUG must be undefined
# otherwise source line and file are not printed is 'assert' fails :
add_definitions(-U NDEBUG)

add_executable(sun_tracker_state_machine_test sun_tracker_state_machine_test.cpp
                                              ../sun_tracker_state_machine.cpp)

include_directories(
    .. ../include ../../image/include ../../target_detector/include
    ../../camera/include ../../motors/include)

target_link_libraries(sun_tracker_state_machine_test)

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
