
# Enable CTest features (provides 'add_test' command below)
# enable_testing()

project(motors_state_machine_test)

add_executable( motors_state_machine_test
    motors_state_machine_test.cpp
    ../motors_state_machine.cpp
    )

include_directories(.. ../include)

# Auto populate the tests from test source file
# Note : CMake must be reconfigured if tests are added/renamed/removed from test source file
# TODO : move this in a common cmake function for reuse
set(TEST_REGEX "^TEST\\(([A-Za-z0-9_]+).*")
file(STRINGS motors_state_machine_test.cpp detected_tests REGEX ${TEST_REGEX})
foreach(test ${detected_tests})
    STRING(REGEX REPLACE ${TEST_REGEX} "\\1" test ${test})
    message(STATUS "detected test ${test}")
    add_test(NAME motors_state_machine_test_${test} COMMAND motors_state_machine_test ${test})
endforeach()
