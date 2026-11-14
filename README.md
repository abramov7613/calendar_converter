# Calendar Converter

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-064F8C.svg)](https://cmake.org/)
[![ez3i](https://img.shields.io/badge/dependency-ez3i-7C3AED.svg)](https://github.com/abramov7613/extracted_z3_integer)
[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

A C++20 library for calendrical calculations and date conversion between the Gregorian, Julian, and Revised Julian calendars.

The project stores each date as a single absolute Chronological Julian Day Number (CJDN) and exposes the same logical day in any supported calendar. It also supports very large year values through an arbitrary-precision integer implementation backed by `ez3i`.

## Features

- Gregorian, Julian, and Revised Julian calendar support
- Conversion through a common internal CJDN representation
- Large-year arithmetic via `ez3i`/arbitrary-precision integer support
- Leap-year checks for all supported calendars
- Calendar-independent comparisons and weekday calculation
- Validation of invalid month/day/year combinations
- CMake integration with optional tests

## Supported calendars

The library exposes:

- `calendar_converter::Gregorian`
- `calendar_converter::Julian`
- `calendar_converter::RevisedJulian` (alias `calendar_converter::RJulian`)

Leap-year behavior:

- Gregorian: divisible by 4, except centuries not divisible by 400
- Julian: divisible by 4
- Revised Julian: follows the historical revised-Julian cycle, including the 900-year rule

## Requirements

- C++20 compiler
- CMake 3.31+
- Git access to fetch the bundled dependency `ez3i` during configure

## Quick start

### Add to a CMake project

```cmake
cmake_minimum_required(VERSION 3.31)
project(my_app LANGUAGES CXX)

add_subdirectory(path/to/calendar_converter)

target_link_libraries(my_app PRIVATE calendar_converter)
```

### Include the header

```cpp
#include "calendar_converter.h"

namespace cc = calendar_converter;
```

## Basic usage

### Construct a date

```cpp
using namespace calendar_converter;

Date gregorian(2025, 1, 1, Gregorian);
Date julian(2025, 1, 1, Julian);
Date revised(2025, 1, 1, RevisedJulian);
```

You can also pass year values as strings for very large years:

```cpp
Date big_year("1000000000000000000000000000000000000", 12, 31, Gregorian);
```

### Query components in any calendar

```cpp
const Date date(2025, 1, 1, Gregorian);

const auto [year, month, day] = date.ymd(Gregorian);
std::cout << year.str() << "-" << month << "-" << day << '\n';

const auto [m, d] = date.md(Julian);
std::cout << m << "/" << d << '\n';
```

### Convert between calendars

```cpp
const Date date(2025, 1, 1, Gregorian);

const auto [g_year, g_month, g_day] = date.ymd(Gregorian);
const auto [j_year, j_month, j_day] = date.ymd(Julian);
const auto [r_year, r_month, r_day] = date.ymd(RevisedJulian);

std::cout << "Gregorian: " << g_year.str() << '-' << g_month << '-' << g_day << '\n';
std::cout << "Julian:    " << j_year.str() << '-' << j_month << '-' << j_day << '\n';
std::cout << "Revised:   " << r_year.str() << '-' << r_month << '-' << r_day << '\n';
```

### Compare and order dates

```cpp
auto d1 = Date("2024", 1, 1, Gregorian);
auto d2 = Date("2024", 1, 2, Gregorian);

if (d1 < d2) {
    std::cout << "d1 is earlier than d2\n";
}

if (d1 == Date("2024", 1, 1, Julian)) {
    std::cout << "Same absolute day across calendars\n";
}
```

### Weekday and leap-year checks

```cpp
const Date date(2024, 2, 29, Gregorian);

if (is_leap_year(2024, Gregorian)) {
    std::cout << "2024 is a leap year\n";
}

switch (date.weekday()) {
    case WeekDay::Sun: std::cout << "Sunday\n"; break;
    case WeekDay::Mon: std::cout << "Monday\n"; break;
    case WeekDay::Tue: std::cout << "Tuesday\n"; break;
    case WeekDay::Wed: std::cout << "Wednesday\n"; break;
    case WeekDay::Thu: std::cout << "Thursday\n"; break;
    case WeekDay::Fri: std::cout << "Friday\n"; break;
    case WeekDay::Sat: std::cout << "Saturday\n"; break;
}
```

### Current UTC date

```cpp
const Date today = Date::now_utc(Gregorian);
const auto [year, month, day] = today.ymd(Gregorian);
```

## `flexint` and big dates

`Date` uses `flexint` internally for year and CJDN values. `flexint` is a lightweight wrapper around the project’s arbitrary-precision integer support, which allows safely representing values beyond ordinary 32/64-bit numeric limits.

```cpp
const Date far_future("999999999999999999999999999999", 1, 1, Gregorian);

std::cout << far_future.cjdn().str() << '\n';
```

## Validation

The library validates:

- calendar enum values
- month values
- day values
- leap-year-dependent month lengths
- minimum and maximum supported ranges

Invalid input throws `std::invalid_argument`.

```cpp
Date invalid("2023", 2, 29, Gregorian); // throws
```

## Building the project

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run the tests:

```bash
ctest --test-dir build --output-on-failure
```

To disable tests:

```bash
cmake -S . -B build -DBUILD_TESTS=OFF
```

## Project structure

```text
calendar_converter/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── calendar_converter.h
├── calendar_converter.cpp
├── flexint.h
├── flexint.cpp
├── tests/
│   ├── calendar_converter_test1.cpp
│   ├── calendar_converter_test2.cpp
│   └── calendar_converter_test3.cpp
└── .github/
```

## API overview

- `Date(year, month, day, calendar = Gregorian)`
- `Date::now_utc(calendar = Gregorian)`
- `Date::min()` / `Date::max()`
- `Date::ymd(calendar)`
- `Date::md(calendar)`
- `Date::year(calendar)`
- `Date::month(calendar)`
- `Date::day(calendar)`
- `Date::cjdn()`
- `Date::weekday()`
- `Date::increment(days)`
- `Date::decrement(days)`
- `is_leap_year(year, calendar)`
- `days_in_month(month, leap)`

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for the full text.
