# MiniMock : a minimalist, header-only, C/C++ mock library

## Introduction

MiniMock is a minimalist mock and test library with following characteristics :
- Easy to integrate : header-only, `#include` it and you're done.
- Lightweight : approximatively 100 lines of code.
- No dependency other than C++11.
- Use native langage features instead of reinventing them differently.
- Easy to learn : it provides a very limited set of functions while covering a wide range of use cases.

In short, MiniMock offers basic mock features by providing only 8 macros :
- 2 macros to create tests : `TEST`, `CREATE_MAIN_ENTRY_POINT`.
- 2 macros to mock functions : `MINI_MOCK_FUNCTION`, `MINI_MOCK_ON_CALL`.
- 4 macros to compare things : `EXPECT`, `EXPECT_MSG`, `ASSERT_MSG`, `NEAR`.

And that's it !

#### Limitations

MiniMock has the following limitations :
- Currently it's a proof-of-concept not tested in a wide range of platforms and use-cases.
- Some complex features typically provided by usual mock frameworks are not and won't be provided (ex : `Times(AtLeast(17))`).
- It has been developped and tested with gcc, it's not guaranteed to work with other tools.

#### Why using MiniMock ?

- You want to explore alternatives to existing mock tools.
- You search a quick, easy and lightweight way to add some tests to your project.

#### Why not using MiniMock ?

- You already master an existing mock framework and are happy with it.
- You need a strong, mature framework, created and maintained by a strong organization.

#### TODO

- Encapsulate macros in safe blocs (see [swallowing the semicolon](https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Swallowing-the-Semicolon.html#Swallowing-the-Semicolon)).
- Add `mini_mock` and `mini_mock::internal` namespaces instead of "mini_mock" prefix everywhere.
- Add mini_mock tests to test mini_mock own behavior.
- Allow to mock several functions with same name but different arguments.
- Allow to mock class methods.

## MiniMock basics

#### Integrating

A single `include` is required to use MiniMock in your tests :
```C++
#include "mini_mock.hpp"
```

#### Creating tests

Two macros allow to create tests :
```C++
TEST(my_first_test,[]() {
    ...
});

TEST(my_second_test,[]() {
    ...
});

CREATE_MAIN_ENTRY_POINT();
```
These macros create a main entry point (`int main(int argc, char **argv)`) that requires the test name
as the first argument and executes it.

It seems to be a de-facto standard in the world of test frameworks, making :
- MiniMock easy to integrate with existing test launchers (I use CTest which is offered with CMake).
- MiniMock easy to launch manually in command line : `my_test   my_first_test`.
- Each test executed in a known clean state because a new application is launched for each test execution.

#### Mocking functions

A single macro allows to define a mocked function with its signature :
```C++
MINI_MOCK_FUNCTION(function_name,return_type,arguments_def,arguments);
```
For example :
```C++
MINI_MOCK_FUNCTION(external_library_do_something,int,(bool b,int i),(b,i));
```
This macro magically creates a few types, variables and functions that will be used internally by MiniMock.

A single macro allows to register a callback to be called when a mocked function is called :
```C++
// 'function_name' is the name of the function that has been declared with MINI_MOCK_FUNCTION
// 'callback' will be called in place of the mocked function
// 'expected_calls_count' defines the exact times that the callback must be called
MINI_MOCK_ON_CALL(function_name,callback,expected_calls_count=1);
```
For example :
```C++
MINI_MOCK_ON_CALL(external_library_do_something,[](bool b,int i){
    // Simply check the input parameters if it makes sense
    EXPECT(!b);
    EXPECT(i==10);
    // Simply return the value
    return 15;
},3); // this function must be called exactly 3 times by code under test
```

Several callbacks can be recorded successively for the same function, in the exact order in which they must be called.

#### Testing

Three macros allow to compare things in a test body :
```C++
// If condition is false :
// - an automatic message will be printed (containing file name and line number)
// - the test continues
// - the test will fail at the end
EXPECT(condition);

// If condition is false :
// - the given message will be printed
// - the test continues
// - the test will fail at the end
EXPECT_MSG(condition,message);

// If condition is false :
// - the given message will be printed
// - the test fails and stops immediately
ASSERT_MSG(condition,message);
```

And a shortcut makes it easy to compare floating point values :
```C++
// NEAR is true if val1 is near val2 :
NEAR(val1,val2)

// NEAR use ACCEPTABLE_DIFFERENCE constant
// which can be redefined by user if a different value is needed :
ACCEPTABLE_DIFFERENCE = 0.000001
```

## Example

To illustrate MiniMock usage, a minimal example is presented in [example](example) subfolder.
