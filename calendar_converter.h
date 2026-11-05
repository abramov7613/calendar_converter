#pragma once
#include <memory>
#include <string>
#include <tuple>
#include <compare>

namespace calendar_converter {

using Year = std::string;
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

bool is_leap_year(const Year &year, Calendar calendar);
bool is_leap_year(long long year, Calendar calendar);
int days_in_month(Month month, bool leap);

class Date {
  class Implementation;
  template <typename T> friend class DateCommonImplementation;
  std::unique_ptr<Implementation> pImpl;

public:
  static Date now_utc(Calendar = Gregorian);
  static const Date& min();
  static const Date& max();
  ~Date() noexcept;
  Date(const Year&, Month, Day, Calendar = Gregorian);
  Date(const std::tuple<Year, Month, Day>&, Calendar = Gregorian);
  Date(long long, Month, Day, Calendar = Gregorian);
  Date(const Date&);
  Date(Date&&) noexcept;
  Date& operator=(Date);
  void swap(Date&) noexcept;
  Year year(Calendar = Gregorian) const;
  Month month(Calendar = Gregorian) const;
  Day day(Calendar = Gregorian) const;
  std::tuple<Year, Month, Day> ymd(Calendar = Gregorian) const;
  std::tuple<Month, Day> md(Calendar = Gregorian) const;
  std::string cjdn() const;
  WeekDay weekday() const;
  std::strong_ordering operator<=>(const Date&) const;
  bool operator==(const Date&) const;
  Date& inc(unsigned long long days); // increment self
  Date& dec(unsigned long long days); // decrement self
  friend void swap(Date &lhs, Date &rhs) noexcept;
}; // class Date

} // namespace calendar_converter
