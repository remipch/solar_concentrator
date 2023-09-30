
# Enable CTest features (provides 'add_test' command below)
# enable_testing()

project(test_motors_logic)

add_executable(test_motors_logic test_motors_logic.cpp ../motors_logic.cpp)

include_directories(.. ../include)

# Auto populate the tests from test source file
# Note : CMake must be reconfigured if tests are added/renamed/removed from test source file
# TODO : move this in a common cmake function for reuse
set(TEST_REGEX "^TEST\\(([A-Za-z0-9_]+).*")
file(STRINGS test_motors_logic.cpp detected_tests REGEX ${TEST_REGEX})
foreach(test ${detected_tests})
    STRING(REGEX REPLACE ${TEST_REGEX} "\\1" test ${test})
    message(STATUS "detected test ${test}")
    add_test(NAME test_motors_logic_${test} COMMAND test_motors_logic ${test})
endforeach()
