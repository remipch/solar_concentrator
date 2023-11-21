# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # To build tests to be run on host :
# cd project_dir
# mkdir build_tests_on_host
# cd build_tests_on_host
# cmake -DTESTS_ON_HOST=1 -DCMAKE_BUILD_TYPE:STRING=DEBUG ..
# make
# cp ./compile_commands.json .. # Because clangd find it in root project folder
# Then run them :
# ctest
# ctest -V
# ctest -V -R "token"
# ctest -V -R "^prefix"
# ctest -V -R "suffix$"

# # To build production code to be run on target :
# . $HOME/esp/esp-idf/export.sh
# cd project_dir
# mkdir build_esp32
# cd build_esp32
# cmake -DIDF_TARGET=esp32 -DCMAKE_BUILD_TYPE:STRING=DEBUG ..
# make menuconfig
# make
# cp ./compile_commands.json .. # Because clangd find it in root project folder
# ESPPORT=/dev/ttyUSB0 make flash
# minicom -D /dev/ttyUSB0 -b 115200 -c on

message(STATUS "TESTS_ON_HOST = ${TESTS_ON_HOST}")
if("${TESTS_ON_HOST}")
    message(WARNING "Build tests to be run on host")
    # In this case we create a classical cmake lists
    # which adds all 'tests_on_host' sub-directories
    # Such tests must be completely independent from ESP_IDF framework

    # Must be called here in root cmakelists for the sub-directory tests to be found by ctest :
    enable_testing()

    # Common tools that can be used for tests :
    include_directories(tests_on_host/stub)
    include_directories(tests_on_host/mini_mock)

    # Add all component's tests_on_host directories
    add_subdirectory(components/motors/tests_on_host)
    add_subdirectory(components/target_detector/tests_on_host)
else()
    message(WARNING "Build production code to be run on ${IDF_TARGET}")
    # In this case we create cmake lists following the standard ESP-IDF guideline
    # all tests_on_host sub-directories are ignored
    message(STATUS "IDF_PATH = $ENV{IDF_PATH}")

    add_compile_options(-fdiagnostics-color=always)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    # !!!WTF!!! this line must be removed (after esp idf v5.0) otherwise CONFIG_SPIRAM will not appear in menuconfig :
    # set(COMPONENTS main)
    project(panel_controller)
endif()
