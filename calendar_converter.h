#pragma once
#include <memory>
#include <string>
#include <tuple>

namespace calendar_converter {

enum class Calendar { J, R, G };
constexpr auto Julian = Calendar::J;
constexpr auto RevisedJulian = Calendar::R;
constexpr auto Gregorian = Calendar::G;

enum class WeekDay {
  Sun = 0,
  Mon = 1,
  Tue = 2,
  Wed = 3,
  Thu = 4,
  Fri = 5,
  Sat = 6
};

using Year = std::string;
using Month = int;
using Day = int;

bool is_leap_year(const Year &year, Calendar calendar);
bool is_leap_year(long long year, Calendar calendar);
int days_in_month(Month month, bool leap);

class Date {
  class Implementation;
  friend Implementation;
  template <typename T> friend class DateCommonImplementation;
  std::unique_ptr<Implementation> pImpl;

public:
  static Date today_utc(Calendar = Gregorian);
  ~Date() noexcept;
  Date(const Year &y, Month m, Day d, Calendar = Gregorian);
  Date(long long, Month, Day, Calendar = Gregorian);
  Date(const Date &);
  Date(Date &&) noexcept;
  Date &operator=(Date);
  void swap(Date &other) noexcept;
  Year year(Calendar = Gregorian) const;
  Month month(Calendar = Gregorian) const;
  Day day(Calendar = Gregorian) const;
  std::tuple<Year, Month, Day> ymd(Calendar = Gregorian) const;
  std::tuple<Month, Day> md(Calendar = Gregorian) const;
  std::string cjdn() const;
  WeekDay weekday() const;
  bool operator==(const Date &) const;
  bool operator!=(const Date &) const;
  bool operator<(const Date &) const;
  bool operator>(const Date &) const;
  bool operator<=(const Date &) const;
  bool operator>=(const Date &) const;
  friend void swap(Date &lhs, Date &rhs) noexcept { lhs.swap(rhs); }
}; // class Date

} // namespace calendar_converter
