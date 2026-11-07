#pragma once
#include <string>
#include <tuple>
#include <compare>
#include <ostream>
#include <ez3i/mpz.h>

namespace calendar_converter {

namespace ez3i = extracted_z3_integer ;

class flexint {
  ez3i::mpz m_num;

public:
  static ez3i::mpz_manager& m() ;
  flexint();
  flexint(int n) ;
  flexint(char* s) ;
  flexint(const char* s) ;
  flexint(const std::string& s) ;
  flexint(std::string_view s) ;
  flexint(flexint const& n) ;
  flexint(flexint && n) noexcept ;
  ~flexint() ;
  explicit operator int() const ;
  operator ez3i::mpz const &() const ;
  operator ez3i::mpz&() ;
  ez3i::mpz const & get() const ;
  ez3i::mpz & get() ;
  flexint & operator=(flexint const & n) ;
  flexint & operator=(flexint&&) noexcept;
  flexint & operator=(ez3i::mpz const & n) ;
  flexint & operator=(int n) ;
  flexint & operator=(char* s) ;
  flexint & operator=(const char* s) ;
  flexint & operator=(const std::string& s) ;
  flexint & operator=(std::string_view s) ;
  void reset() ;
  void swap(flexint & n) noexcept ;
  void swap(ez3i::mpz & n) noexcept ;
  bool is_zero() const ;
  std::string str() const ;
  flexint & operator+=(ez3i::mpz const & a) ;
  flexint & operator-=(ez3i::mpz const & a) ;
  flexint & operator*=(ez3i::mpz const & a) ;
  flexint & operator/=(ez3i::mpz const & a) ;
  flexint & operator%=(ez3i::mpz const & a) ;
  friend flexint abs(flexint const& a) ;
  friend bool is_zero(flexint const & a) ;
  friend void swap(flexint & a, flexint & b) noexcept ;
  friend std::string to_string(flexint const& a) ;
  friend bool operator==(flexint const & a, ez3i::mpz const & b) ;
  friend bool operator!=(flexint const & a, ez3i::mpz const & b) ;
  friend bool operator<(flexint const & a, ez3i::mpz const & b) ;
  friend bool operator>(flexint const & a, ez3i::mpz const & b) ;
  friend bool operator<=(flexint const & a, ez3i::mpz const & b) ;
  friend bool operator>=(flexint const & a, ez3i::mpz const & b) ;
  friend std::strong_ordering operator<=>(flexint const & a, ez3i::mpz const & b) ;
  friend flexint operator+(flexint const & r1, ez3i::mpz const & r2) ;
  friend flexint operator-(flexint const & r1, ez3i::mpz const & r2) ;
  friend flexint operator*(flexint const & r1, ez3i::mpz const & r2) ;
  friend flexint operator/(flexint const & r1, ez3i::mpz const & r2) ;
  friend flexint operator%(flexint const & r1, ez3i::mpz const & r2) ;
  friend std::ostream & operator<<(std::ostream & out, flexint const & s) ;
}; // class flexint


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
