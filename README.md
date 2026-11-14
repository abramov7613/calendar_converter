# Calendar Converter

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-blue.svg)](https://cmake.org/)
[![ez3i](https://img.shields.io/badge/dependency-ez3i-7C3AED.svg)](https://github.com/abramov7613/extracted_z3_integer)
[![build & test](https://github.com/abramov7613/calendar_converter/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/abramov7613/calendar_converter/actions/workflows/cmake-multi-platform.yml)

A C++20 library for working with dates and converting between the **Gregorian**, **Julian**, and **Revised Julian** calendars.

The library uses the **Chronological Julian Day Number (CJDN)** as a common internal representation. This allows the same `Date` object to be represented in any supported calendar and makes date comparison independent of the calendar in which a date was originally created.

The project is particularly suitable as a low-level component for applications dealing with historical chronology, date conversion, astronomical calculations, genealogy, archival data, or any software where several calendar systems need to coexist.

## Features

- Gregorian, Julian, and Revised Julian calendar support
- Conversion through a common internal CJDN representation
- Large-year arithmetic via `ez3i`/arbitrary-precision integer support
- Leap-year checks for all supported calendars
- Calendar-independent comparisons and weekday calculation
- Validation of invalid month/day/year combinations
- CMake integration with optional tests

## Supported calendars

The library exposes three calendar constants:

| Calendar       | Constant        | Leap-year rule                                        |
| -------------- | --------------- | ----------------------------------------------------- |
| Gregorian      | `Gregorian`     | Divisible by 4, except centuries not divisible by 400 |
| Julian         | `Julian`        | Divisible by 4                                        |
| Revised Julian | `RJulian`       | Revised Julian leap-year rule                         |

The Revised Julian leap-year calculation implemented by the library follows the rule based on the remainder of the year modulo 900. There are 218 leap years in a 900-year cycle, giving an average year length of approximately 365.242222 days. An important consequence is that the Revised Julian and Gregorian calendars coincide for long periods but do not have identical long-term leap-year rules.

## How It Works

A date is converted into a **Chronological Julian Day Number (CJDN)**. Conceptually:

```text
               ┌───────────────────────────────────────────┐
               │   Gregorian / Julian / Revised Julian     │
               └───────────────────────────────────────────┘
                                   │
                                   ▼
                          ┌─────────────────┐
                          │      CJDN       │
                          │                 │
                          │ common internal │
                          │ representation  │
                          └────────┬────────┘
                                   │
          ┌────────────────────────┐───────────────────────┐
          ▼                        ▼                       ▼
  ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
  │     Julian      │     │    Gregorian    │     │ Revised Julian  │
  └─────────────────┘     └─────────────────┘     └─────────────────┘
```

This means that a date does not permanently belong to the calendar used during construction.

For example:

```cpp
Date date(2025, 1, 1, Gregorian);
```

can later be queried as:

```cpp
date.ymd(Gregorian);
date.ymd(Julian);
date.ymd(RJulian);
```

Internally, comparisons are performed using CJDN, so two dates representing the same physical calendar day compare equal even if they were constructed using different calendars. The implementation uses the PImpl idiom for Date, keeping implementation details out of the public header. The combination of PImpl and the CJDN-based model keeps the public API small while allowing the arithmetic implementation to evolve independently.

> **Note:** The CJDN used by this project should not be confused with the **Julian calendar**. A Julian Day Number is an absolute day count used as a numerical representation of a date, while the Julian calendar is a civil calendar that can itself be represented using that count.

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
Date revised(2025, 1, 1, RJulian);
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
const auto [r_year, r_month, r_day] = date.ymd(RJulian);

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
└── tests/
    ├── calendar_converter_test1.cpp
    ├── calendar_converter_test2.cpp
    ├── calendar_converter_test3.cpp
    └── CMakeLists.txt
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

## Algorithms and Dr. Louis Strous

The calendar conversion algorithms used by this project are based on the work of **Dr. Louis Strous**. The source code explicitly credits Strous in the implementation comments.

A particularly relevant reference is Dr. Strous' **Astronomy Answers — Modern Calendars**, which describes algorithms and astronomical/calendar relationships used for computational calendar conversion. The work is associated with the **University of Utrecht** and provides material relevant to Gregorian, Julian, and Revised Julian calendar calculations.

The implementation in this project follows the same general computational approach: calendar dates are converted to a common day-number representation and can subsequently be converted back into the desired calendar.

Conceptually:

```text
Gregorian ────────┐  
                  │           ┌────► Gregorian
Julian ───────────┼──►  CJDN  ├────► Julian
                  │           └────► Revised Julian
Revised Julian ───┘  
```

This intermediate representation avoids the need to implement independent conversion formulas for every pair of calendars.

## References

### Primary algorithm reference

**Louis Strous — Astronomy Answers: Modern Calendars**

[Louis Strous — Astronomy Answers: Modern Calendars](https://aa.quae.nl/en/antwoorden/moderne_kalenders.html)

### Astronomy Answers

Collection of astronomical and calendar-related material by Louis Strous.

[Astronomy Answers — Louis Strous' collection of astronomical and calendar material](https://aa.quae.nl/)

### Fourmilab Calendar Converter

An independent implementation and reference for Julian Day Numbers and civil-calendar conversion.

[Fourmilab Calendar Converter](https://www.fourmilab.ch/documents/calendar)

### Related calendar arithmetic

The general approach of representing calendar dates through a continuous numerical day count is also used by other calendar-arithmetic implementations.

[Soroush's libcalendars — calendar arithmetic algorithms](https://github.com/soroush/libcalendars)
