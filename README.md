# Calendar Converter

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-064F8C.svg)](https://cmake.org/)
[![Boost](https://img.shields.io/badge/Boost-1.82%2B-00599C.svg)](https://www.boost.org/)
[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

A C++20 library for working with dates and converting between the **Gregorian**, **Julian**, and **Revised Julian** calendars.

The library uses the **Chronological Julian Day Number (CJDN)** as a common internal representation. This allows the same `Date` object to be represented in any supported calendar and makes date comparison independent of the calendar in which a date was originally created.

The project is particularly suitable as a low-level component for applications dealing with historical chronology, date conversion, astronomical calculations, genealogy, archival data, or any software where several calendar systems need to coexist.

## ✨ Features

* Gregorian / Julian/ Revised Julian calendars
* Conversion through Chronological Julian Day Number (CJDN)
* Arbitrarily large year values using `boost::multiprecision::cpp_int`
* Leap-year, Weekday calculation for all supported calendars
* Calendar-independent date comparison
* Validation of dates and calendar values
* GoogleTest-based test suite
* CMake integration

---

## 📚 Supported Calendars

The library exposes three calendar constants:

| Calendar       | Constant        | Leap-year rule                                        |
| -------------- | --------------- | ----------------------------------------------------- |
| Gregorian      | `Gregorian`     | Divisible by 4, except centuries not divisible by 400 |
| Julian         | `Julian`        | Divisible by 4                                        |
| Revised Julian | `RevisedJulian` | Revised Julian leap-year rule                         |

The Revised Julian leap-year calculation implemented by the library follows the rule based on the remainder of the year modulo 900. There are 218 leap years in a 900-year cycle, giving an average year length of approximately 365.242222 days. An important consequence is that the Revised Julian and Gregorian calendars coincide for long periods but do not have identical long-term leap-year rules.

---

# 🧠 How It Works

A date is converted into a **Chronological Julian Day Number (CJDN)**.

Conceptually:

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
date.ymd(RevisedJulian);
```

Internally, comparisons are performed using CJDN, so two dates representing the same physical calendar day compare equal even if they were constructed using different calendars. The implementation uses the PImpl idiom for Date, keeping implementation details out of the public header. The combination of PImpl and the CJDN-based model keeps the public API small while allowing the arithmetic implementation to evolve independently.

> **Note:** The CJDN used by this project should not be confused with the **Julian calendar**. A Julian Day Number is an absolute day count used as a numerical representation of a date, while the Julian calendar is a civil calendar that can itself be represented using that count.

---

# 🚀 Quick Start

## Requirements

* C++20-compatible compiler
* CMake 3.31 or newer
* Boost 1.82 or newer

The project currently requires Boost through CMake's package configuration and compiles the library with C++20.

## Clone

```bash
git clone https://github.com/abramov7613/calendar_converter.git
```

## CMake Integration

The library can be added to another CMake project with `add_subdirectory()`.

For example:

```cmake
add_subdirectory(path/to/calendar_converter)
# The project build a static library named `calendar_converter`
target_link_libraries(my_application
    PRIVATE
        calendar_converter
)
```

Then:

```cpp
#include "calendar_converter.h"
//...
```

## Build

```bash
cd path/to/my_application/source_root
cmake -S . -B build
cmake --build build
```

---

# 💻 Usage

Include the main header:

```cpp
#include "calendar_converter.h"

namespace cc = calendar_converter;
```

## Create a date

Gregorian is the default calendar:

```cpp
cc::Date date(2025, 1, 1);
```

Or explicitly specify a calendar:

```cpp
cc::Date date(2025, 1, 1, Gregorian);
```

Julian:

```cpp
cc::Date date(2025, 1, 1, Julian);
```

Revised Julian:

```cpp
cc::Date date(2025, 1, 1, RevisedJulian);
```

---

# 🔄 Convert Between Calendars

A `Date` can be queried in any supported calendar.

```cpp
#include "calendar_converter.h"
#include <iostream>

int main() {
    using namespace calendar_converter;

    Date date(2025, 1, 1, Gregorian);

    auto [gy, gm, gd] = date.ymd(Gregorian);
    auto [jy, jm, jd] = date.ymd(Julian);
    auto [ry, rm, rd] = date.ymd(RevisedJulian);

    std::cout << "Gregorian: "
              << gy << "-" << gm << "-" << gd << "\n";

    std::cout << "Julian: "
              << jy << "-" << jm << "-" << jd << "\n";

    std::cout << "Revised Julian: "
              << ry << "-" << rm << "-" << rd << "\n";
}
```

There is no separate `convert()` operation. The `Date` object represents the same CJDN and provides its date components in the requested calendar.

---

# 🔢 Large Years

One of the main features of this library is support for extremely large year values. The implementation uses a compact integer representation for small years and CJDN values and switches to `cpp_int` when necessary.

For example:

```cpp
Date date(
    "1000000000000000000000000000000000000",
    1,
    1,
    Gregorian
);
```

The implementation uses:

```cpp
boost::multiprecision::cpp_int
```

for large CJDN and year calculations.

The source currently permits year strings up to **2048 decimal digits**.

---

# 🧮 Leap Years

Check whether a year is a leap year:

```cpp
bool leap = is_leap_year(2024, Gregorian);
```

For large years:

```cpp
bool leap = is_leap_year(
    "1000000000000000000000000000000000000",
    Gregorian
);
```

---

# 📏 Days in a Month

Use:

```cpp
int days = days_in_month(2, true);
```

Examples:

```cpp
days_in_month(1, false); // 31
days_in_month(2, false); // 28
days_in_month(2, true);  // 29
days_in_month(4, false); // 30
```

The function takes the month number and whether the corresponding year is a leap year.

---

# 🗓️ Weekday

Get the weekday of a date:

```cpp
WeekDay weekday = date.weekday();
```

Available values:

```cpp
WeekDay::Sun
WeekDay::Mon
WeekDay::Tue
WeekDay::Wed
WeekDay::Thu
WeekDay::Fri
WeekDay::Sat
```

The enumeration uses Sunday as `0` and Saturday as `6`.

Example:

```cpp
switch (date.weekday()) {
case WeekDay::Sun:
    std::cout << "Sunday\n";
    break;
case WeekDay::Mon:
    std::cout << "Monday\n";
    break;
default:
    std::cout << "Another day\n";
    break;
}
```

---

# ⏱️ Current UTC Date

Get today's UTC date:

```cpp
Date today = Date::today_utc();
```

A calendar can be specified:

```cpp
Date today_julian = Date::today_utc(Julian);
```

The implementation obtains the current Gregorian UTC date and represents that same date in the requested calendar.

---

# 🔍 Date Components

Get individual components:

```cpp
auto year = date.year();
auto month = date.month();
auto day = date.day();
```

A target calendar can be specified:

```cpp
auto julian_year = date.year(Julian);
```

Or get all three components at once:

```cpp
auto [year, month, day] = date.ymd(Gregorian);
```

Month and day can also be retrieved together:

```cpp
auto [month, day] = date.md(Gregorian);
```

---

# 🔢 CJDN

Get the Chronological Julian Day Number:

```cpp
std::string cjdn = date.cjdn();
```

Example:

```cpp
Date date(2025, 1, 1, Gregorian);

std::cout << "CJDN: "
          << date.cjdn()
          << '\n';
```

The value is returned as a `std::string`, allowing CJDN values that do not fit into standard integer types.

---

# ⚖️ Date Comparison

`Date` supports all standard relational operators. Example:

```cpp
Date first(2025, 1, 1);
Date second(2025, 1, 2);

if (first < second) {
    std::cout << "first comes before second\n";
}
```

Comparison is based on CJDN rather than the original calendar.

---

# 🛡️ Validation

The library validates:

* calendar values
* month values
* day values
* year values
* leap-year-dependent month lengths
* minimum supported date
* maximum year string length

Invalid input results in an exception.

For example:

```cpp
Date invalid(2025, 2, 30, Gregorian);
```

will be rejected because February 30 is not a valid Gregorian date.

Years must be at least `1`, and year strings may contain no more than 2048 decimal digits.

---

# 📐 Algorithms and Dr. Louis Strous

The calendar conversion algorithms used by this project are based on the work of **Dr. Louis Strous**. The source code explicitly credits Strous in the implementation comments.

A particularly relevant reference is Dr. Strous' **Astronomy Answers — Modern Calendars**, which describes algorithms and astronomical/calendar relationships used for computational calendar conversion. The work is associated with the **University of Utrecht** and provides material relevant to Gregorian, Julian, and Revised Julian calendar calculations.

[Louis Strous — Astronomy Answers: Modern Calendars](https://aa.quae.nl/en/antwoorden/moderne_kalenders.html)

[Astronomy Answers — Louis Strous' collection of astronomical and calendar material](https://aa.quae.nl/)

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

### Julian Day Numbers and Calendar Conversion

A Julian Day Number is a **continuous numerical count of days**, not a calendar.

The **Julian calendar** is a civil calendar with its own leap-year rules.

The distinction is important:

```text
Julian calendar
       │
       │ conversion
       ▼
Julian Day Number / CJDN
       │
       │ conversion
       ▼
Gregorian / Julian / Revised Julian
```

Using a continuous day count as an intermediate representation is a well-established approach to calendar arithmetic. It makes it possible to perform date arithmetic and calendar conversion without directly implementing every possible calendar-to-calendar transformation.

For additional background on Julian Day Numbers and computational calendar conversion, see:

[Fourmilab — Calendar Converter and Calendar Calculations](https://www.fourmilab.ch/documents/calendar)

The Fourmilab documentation provides an independent implementation and useful reference for Julian Day Numbers and civil-calendar conversion.

---

# 🌍 Example: One Date, Three Calendars

```cpp
#include "calendar_converter.h"
#include <iostream>

int main() {
    using namespace calendar_converter;

    const Date date(2025, 1, 1, Gregorian);

    const auto [gy, gm, gd] = date.ymd(Gregorian);
    const auto [jy, jm, jd] = date.ymd(Julian);
    const auto [ry, rm, rd] = date.ymd(RevisedJulian);

    std::cout
        << "Gregorian:      "
        << gy << "-" << gm << "-" << gd << '\n';

    std::cout
        << "Julian:         "
        << jy << "-" << jm << "-" << jd << '\n';

    std::cout
        << "Revised Julian: "
        << ry << "-" << rm << "-" << rd << '\n';

    std::cout
        << "CJDN:           "
        << date.cjdn() << '\n';

    return 0;
}
```

This illustrates the central idea of the library: the calendar representation can change without changing the underlying date.

---

# 🧪 Tests

Tests are enabled by default when the project is the top-level CMake project.

Build:

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
ctest --test-dir build --output-on-failure
```

The project currently contains three GoogleTest-based test executables:

```text
calendar_converter_test1
calendar_converter_test2
calendar_converter_test3
```

The tests include thousands of randomized conversions, providing an additional check against arithmetic and boundary errors. GoogleTest `v1.17.0` is fetched automatically by CMake when `BUILD_TESTS` is enabled.

To disable tests:

```bash
cmake -S . -B build -DBUILD_TESTS=OFF
cmake --build build
```

---

# 📁 Project Structure

```text
calendar_converter/
├── .github/
│   └── workflows/
├── tests/
│   ├── calendar_converter_test1.cpp
│   ├── calendar_converter_test2.cpp
│   └── calendar_converter_test3.cpp
├── calendar_converter.h
├── calendar_converter.cpp
├── CMakeLists.txt
├── LICENSE
└── README.md
```

---

# 📦 Library API at a Glance

| API                  | Description                   |
| -------------------- | ----------------------------- |
| `Date(...)`          | Create a date                 |
| `Date::today_utc()`  | Get current UTC date          |
| `Date::year()`       | Get year (as std::string)     |
| `Date::month()`      | Get month                     |
| `Date::day()`        | Get day                       |
| `Date::ymd()`        | Get year, month and day       |
| `Date::md()`         | Get month and day             |
| `Date::cjdn()`       | Get CJDN                      |
| `Date::weekday()`    | Get weekday                   |
| `is_leap_year()`     | Check leap year               |
| `days_in_month()`    | Get number of days in a month |
| `==`, `!=`           | Equality comparison           |
| `<`, `>`, `<=`, `>=` | Ordering comparison           |

---

# ⚠️ Supported Date Range

The implementation currently requires the year to be at least `1`.

The minimum accepted dates correspond to:

```text
Gregorian: 01-01-01
Julian:    03-01-01
```

The year may contain up to 2048 decimal digits.

Dates earlier than the supported range are rejected.

---

# 📜 References

### Primary algorithm reference

**Louis Strous — Astronomy Answers: Modern Calendars**

[https://aa.quae.nl/en/antwoorden/moderne_kalenders.html](https://aa.quae.nl/en/antwoorden/moderne_kalenders.html)

### Astronomy Answers

Collection of astronomical and calendar-related material by Louis Strous.

[https://aa.quae.nl/en/](https://aa.quae.nl/)

### Fourmilab Calendar Converter

An independent implementation and reference for Julian Day Numbers and civil-calendar conversion.

[Fourmilab Calendar Converter](https://www.fourmilab.ch/documents/calendar)

### Related calendar arithmetic

The general approach of representing calendar dates through a continuous numerical day count is also used by other calendar-arithmetic implementations.

[Soroush's libcalendars — calendar arithmetic algorithms](https://github.com/soroush/libcalendars)

---

# 📜 License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for the complete license text.
