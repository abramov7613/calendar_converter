#pragma once
#include <string>
#include <tuple>
#include <compare>
#include <ostream>
#include "flexint.h"

namespace calendar_converter {

using Year = flexint;
using Month = int;
using Day = int;


enum class Calendar { J, R, G };
constexpr auto Gregorian = Calendar::G;
constexpr auto Julian = Calendar::J;
constexpr auto RevisedJulian = Calendar::R;
constexpr auto RJulian = RevisedJulian;


enum class WeekDay {
  Sun = 0,
  Mon = 1,
  Tue = 2,
  Wed = 3,
  Thu = 4,
  Fri = 5,
  Sat = 6
};


bool is_leap_year(const Year &, Calendar);
int days_in_month(Month month, bool leap);


class Date {
  flexint cjdn_; // Chronological Julian Day Number

public:
  static Date now_utc(Calendar = Gregorian);
  static const Date& min();
  static const Date& max();
  Date(const Year&, Month, Day, Calendar = Gregorian);
  Date(const std::tuple<Year, Month, Day>&, Calendar = Gregorian);
  Date(const Date&) = default;
  Date(Date&&) noexcept = default;
  Date& operator=(const Date&) = default;
  Date& operator=(Date&&) = default;
  bool operator==(const Date&) const = default;
  std::strong_ordering operator<=>(const Date&) const = default;
  std::tuple<Year, Month, Day> ymd(Calendar = Gregorian) const;
  std::tuple<Month, Day> md(Calendar = Gregorian) const;
  Year year(Calendar = Gregorian) const;
  Month month(Calendar = Gregorian) const;
  Day day(Calendar = Gregorian) const;
  flexint cjdn() const;
  WeekDay weekday() const;
  void swap(Date&) noexcept;
  friend void swap(Date&, Date&) noexcept ;
  //Date& inc(const flexint& days);
  //Date& dec(const flexint& days);
}; // class Date

} // namespace calendar_converter
