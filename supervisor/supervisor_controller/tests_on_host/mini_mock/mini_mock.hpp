#pragma once


// MiniMock : a minimalist, header-only, C/C++ mock library
// (see https://github.com/remipch/mini_mock for introduction, documentation and example)


// MIT License
//
// Copyright (c) 2023 Rémi PEUCHOT
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


// Usefull links to understand internal magic happening here :
//   https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Stringification.html
//   https://gcc.gnu.org/onlinedocs/cpp/Concatenation.html
//   https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Swallowing-the-Semicolon.html#Swallowing-the-Semicolon
// Preprocessor output can be investigated with a command line like :
//   gcc -Ipath/to/mini_mock/folder -E -CC path/to/my_test.cpp


#if(__cplusplus<201103)
#warning "C++ >= 11 is required to use std::function and std::deque"
#endif

#include <string>
#include <cmath>
#include <functional>
#include <iostream>
#include <deque>
#include <map>

// Useful console colors
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define END_COLOR "\033[0m"

static int mini_mock_failed_conditions_count = 0;

// If condition is false :
// - an automatic message will be printed (with file name and line number)
// - the test continues
// - the test will fail at the end
#define EXPECT(condition) { \
    if(condition) { \
        std::cout << GREEN << "    passed condition (";\
    } \
    else { \
        mini_mock_failed_conditions_count++; \
        std::cout << RED << "    failed condition (";\
    } \
    std::cout << #condition << ") at " <<__FILE__ << ":"<< __LINE__ << END_COLOR << '\n'; \
}

// If condition is false :
// - the given message will be printed
// - the test continues
// - the test will fail at the end
#define EXPECT_MSG(condition,message) { \
    if(!condition) { \
        mini_mock_failed_conditions_count++; \
        std::cout << RED << "    failed condition (" #condition << ") : "; \
        std::cout << message << " at " <<__FILE__ << ":"<< __LINE__ << END_COLOR << '\n'; \
    } \
}

// If condition is false :
// - the given message will be printed
// - the test fails and stops immediately
#define ASSERT_MSG(condition,message) { \
    if(!condition) { \
        std::cout << RED << "    failed condition (" #condition << ") : "; \
        std::cout << message << " at " <<__FILE__ << ":"<< __LINE__ << END_COLOR << '\n'; \
        exit(-1); \
    } \
}

// ACCEPTABLE_DIFFERENCE can be redefined by user before including this file
// if a different value is needed
#ifndef ACCEPTABLE_DIFFERENCE
#define ACCEPTABLE_DIFFERENCE 0.000001
#endif

// NEAR is true if val1 is near val2
#define NEAR(val1,val2) (std::abs(val1-val2)<=ACCEPTABLE_DIFFERENCE)

// For each callback name, associate the number of recorded callbacks
static std::map<std::string,int> mini_mock_recorded_callbacks;

// Thank you Nero for the hack to remove parenthesis : https://stackoverflow.com/a/62984543
#define DEPAREN(X) ESC(ISH X)
#define ISH(...) ISH __VA_ARGS__
#define ESC(...) ESC_(__VA_ARGS__)
#define ESC_(...) VAN ## __VA_ARGS__
#define VANISH

#define MINI_MOCK_FUNCTION(function_name,return_type,arguments_def,arguments) \
MINI_MOCK_FUNCTION_INTERNAL(function_name,return_type,DEPAREN(arguments_def),DEPAREN(arguments))

#define MINI_MOCK_FUNCTION_INTERNAL(function_name,return_type,arguments_def,arguments) \
typedef std::function<return_type(arguments_def)> mini_mock_callback__ ## function_name; \
\
struct mini_mock_callback_record__ ## function_name { \
    mini_mock_callback__ ## function_name callback; \
    int remaining_calls_count; \
}; \
\
static std::deque<mini_mock_callback_record__ ## function_name> mini_mock_callbacks__ ## function_name; \
\
void mini_mock__ ## function_name(mini_mock_callback__ ## function_name callback,int expected_calls_count=1) { \
    struct mini_mock_callback_record__ ## function_name record{ \
        .callback = callback, \
        .remaining_calls_count = expected_calls_count, \
    }; \
    mini_mock_callbacks__ ## function_name.push_back(record); \
    mini_mock_recorded_callbacks[#function_name]+=expected_calls_count; \
} \
\
return_type  function_name(arguments_def) { \
    ASSERT_MSG(!mini_mock_callbacks__ ## function_name.empty(),"No callback defined for " #function_name); \
    mini_mock_callback_record__ ## function_name& callback_record = mini_mock_callbacks__ ## function_name.front(); \
    if(callback_record.remaining_calls_count>1) { \
        callback_record.remaining_calls_count--; \
    } \
    else { \
        mini_mock_callbacks__ ## function_name.pop_front(); \
    } \
    mini_mock_recorded_callbacks[#function_name]--; \
    return callback_record.callback(arguments); \
}

// MINI_MOCK_ON_CALL(function_name,callback,expected_calls_count=1)
// 'function_name' is the name of the function that has been declared with MINI_MOCK_FUNCTION
// 'callback' will be called in place of the mocked function
// 'expected_calls_count' defines the exact times that the callback must be called
#define MINI_MOCK_ON_CALL(function_name,...) mini_mock__ ## function_name(__VA_ARGS__)

typedef std::function<void()> mini_mock_test_function;

static std::map<std::string,mini_mock_test_function> mini_mock_tests;

// Declare a struct for test definition, it allows to update
// the global test list for each test created in file scope
struct mini_mock_test {
    mini_mock_test(std::string test_name,mini_mock_test_function test_function){
        mini_mock_tests[test_name] = test_function;
    }
};

// This macro is to be interpreted as :
// #define TEST(test,test_function) mini_mock_test test(#test,test_function)
// However, 'test_function' cannot be passed as macro parameter because C++ precompilator
// will split it by all internal commas it found inside its body
// (see https://stackoverflow.com/a/49826249)
// Passing it as variadic arguments allows to pass it correctly to mini_mock_test constructor
#define TEST(test,...) mini_mock_test test(#test,__VA_ARGS__)

// This macro must be placed after all 'TEST' macros
#define CREATE_MAIN_ENTRY_POINT() \
    int main(int argc, char **argv) {  \
        if(argc!=2) { \
            std::cout << RED << "test name must be passed as command line argument" << END_COLOR << '\n'; \
            exit(-1); \
        } \
        std::string test_name(argv[1]); \
        std::cout << YELLOW << test_name << END_COLOR << '\n'; \
        if (mini_mock_tests.find(test_name) == mini_mock_tests.end()) { \
            std::cout << RED << "unknown test  :" << test_name << END_COLOR << '\n'; \
            exit(-1); \
        } \
        mini_mock_test_function test_function = mini_mock_tests.at(test_name); \
        test_function(); \
        for (const auto &recorded_callback : mini_mock_recorded_callbacks) { \
            int uncalled_callbacks_count = recorded_callback.second; \
            ASSERT_MSG(uncalled_callbacks_count==0, std::to_string(uncalled_callbacks_count) \
            + " mock callback(s) never called for " + recorded_callback.first); \
        } \
        if(mini_mock_failed_conditions_count==0) { \
            std::cout << GREEN << "PASSED" << END_COLOR << '\n'; \
            exit(0); \
        } \
        else { \
            std::cout << RED << "FAILED " << END_COLOR << '\n'; \
            exit(-1); \
        } \
    }
