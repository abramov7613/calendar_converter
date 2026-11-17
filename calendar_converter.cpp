#include "calendar_converter.h"
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace cc = calendar_converter;


namespace {

using cc::flexint;
using cc::Calendar;
using cc::Day;
using cc::Gregorian;
using cc::Julian;
using cc::Month;
using cc::RevisedJulian;
using cc::WeekDay;
using cc::Year;
using cc::MIN_YEAR;
using cc::MAX_YEAR_DIGITS;


constexpr std::int32_t MIN_CJDN = 1'721'426;


template <typename T>
concept Ostreamable = requires(std::ostream &out, T &&value) {
  out << std::forward<T>(value);
};


template <typename Exception = std::runtime_error, typename Arg, typename... Args>
requires (
  std::derived_from<Exception, std::exception> &&
  Ostreamable<Arg> &&
  (Ostreamable<Args> && ...)
)
void THROW(Arg &&arg, Args &&...args) {
  std::ostringstream out;
  out << std::forward<Arg>(arg);
  ((out << ' ' << std::forward<Args>(args)), ...);
  throw Exception(out.str());
}


template<typename T1, typename T2> requires (
  (std::same_as<T1, int> || std::same_as<T1, flexint>) &&
  (std::same_as<T2, int> || std::same_as<T2, flexint>)
)
auto floor_div(const T1& a, const T2& b) {
    using ResultType = std::conditional_t<
      std::is_same_v<T1, flexint> || std::is_same_v<T2, flexint>,
      flexint,
      int
    >;
    ResultType r = (a - (a < 0 ? b - 1 : 0)) / b ;
    return r;
}


template<typename T1, typename T2> requires (
  (std::same_as<T1, int> || std::same_as<T1, flexint>) &&
  (std::same_as<T2, int> || std::same_as<T2, flexint>)
)
auto floor_mod(const T1& a, const T2& b) {
    using ResultType = std::conditional_t<
      std::is_same_v<T1, flexint> || std::is_same_v<T2, flexint>,
      flexint,
      int
    >;
    ResultType r = a - floor_div(a, b) * b ;
    return r;
}


template<typename T1, typename T2> requires (
  (std::same_as<T1, int> || std::same_as<T1, flexint>) &&
  (std::same_as<T2, int> || std::same_as<T2, flexint>)
)
auto positive_rem_div(const T1& a, const T2& b) {
    // positive remainder division: a = q * b + r, where 0 <= r < abs(b)
    using ResultType = std::conditional_t<
      std::is_same_v<T1, flexint> || std::is_same_v<T2, flexint>,
      flexint,
      int
    >;
    ResultType quotient = a / b;
    ResultType remainder = a % b;
    if (remainder < 0) {
      if (b > 0) {
        quotient -= 1;
        remainder += b;
      } else {
        quotient += 1;
        remainder -= b;
      }
    }
    return std::make_pair(quotient, remainder);
}


constexpr bool calendar_is_valid(Calendar calendar) {
  switch (calendar) {
    case Gregorian:
    case Julian:
    case RevisedJulian:
      return true;
  }
  return false;
}


bool is_leap(const flexint &year, const Calendar calendar) {
  if (!calendar_is_valid(calendar))
    THROW<std::invalid_argument>("invalid Calendar value: ",
                                 static_cast<int>(calendar));
  switch (calendar) {
    case Julian:
      return year%4 == 0 ;

    case Gregorian:
      return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);

    case RevisedJulian:
      return year % 4 == 0 && (year % 100 != 0 || year % 900 % 400 == 200);
  }
  return false;
}


int m_length(const Month month, const bool leap) {
  switch (month) {
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    return 31;
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  case 2:
    return leap ? 29 : 28;
  default:
    THROW<std::invalid_argument>("invalid month value: ", month);
  }
  return -1;
}


flexint gregorian_to_cjdn(const flexint& year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#3_1
{
  const int c0 = floor_div(m - 3, 12);
  const int x1 = m - 12 * c0 - 3;
  const flexint x4 = year + c0;
  auto [x3, x2] = positive_rem_div(x4, 100);
  flexint result = d + 1721119;
  result += floor_div(146097 * x3, 4);
  result += floor_div(36525 * x2, 100);
  result += floor_div(153 * x1 + 2, 5);
  return result;
}


flexint julian_to_cjdn(const flexint &year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#5_1
{
  const int c0 = floor_div(m - 3, 12);
  const flexint j1 = floor_div(1461 * (year + c0), 4);
  // This expression is always small enough for int
  const int j2 = floor_div(153 * m - 1836 * c0 - 457, 5);
  flexint result = j1 + j2 + d + 1721117;
  return result;
}


flexint revisedjulian_to_cjdn(const flexint &year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#4_1
{
  const int c0 = floor_div(m - 3, 12);
  const int x1 = m - c0 * 12 - 3;
  const flexint x4 = year + c0;
  const flexint x3 = floor_div(x4, 100);
  const flexint x2 = floor_mod(x4, 100);
  flexint result = d + 1721119;
  result += floor_div(328718 * x3 + 6, 9);
  result += floor_div(36525 * x2, 100);
  result += floor_div(153 * x1 + 2, 5);
  return result;
}


std::tuple<Year, Month, Day> cjdn_to_gregorian(const flexint& cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#3_2
{
  auto [x3, r3] = positive_rem_div(4 * cjdn - 6884477, 146097);
  auto [x2, r2] = positive_rem_div(100 * floor_div(r3, 4) + 99, 36525);
  auto [x1, r1] = positive_rem_div(5 * floor_div(r2, 100) + 2, 153);
  const auto c0 = static_cast<int>(floor_div(x1 + 2, 12));
  const auto d = static_cast<int>(floor_div(r1, 5) + 1);
  const auto m = static_cast<int>(x1 - 12 * c0 + 3);
  const flexint y = x3 * 100 + x2 + c0;
  return {y, m, d};
}


std::tuple<Year, Month, Day> cjdn_to_julian(const flexint &cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#5_2
{
  const flexint y2 = cjdn - 1721118;
  const flexint k2 = y2 * 4 + 3;
  const int k1 =
      5 * floor_div(static_cast<int>(floor_mod(k2, 1461)), 4) + 2;
  const int x1 = floor_div(k1, 153);
  const int c0 = floor_div(x1 + 2, 12);
  const flexint y = floor_div(k2, 1461) + c0;
  const Month m = x1 - 12 * c0 + 3;
  const Day d = floor_div(floor_mod(k1, 153), 5) + 1;
  return {y, m, d};
}


std::tuple<Year, Month, Day> cjdn_to_revisedjulian(const flexint &cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#4_2
{
  const flexint k3 = 9 * (cjdn - 1721120) + 2;
  const flexint x3 = floor_div(k3, 328718);
  const int k2 =
      100 * floor_div(static_cast<int>(floor_mod(k3, 328718)), 9) + 99;
  const int x2 = floor_div(k2, 36525);
  const int k1 = floor_div(floor_mod(k2, 36525), 100) * 5 + 2;
  const int x1 = floor_div(k1, 153);
  const int c0 = floor_div(x1 + 2, 12);
  const flexint y = x3 * 100 + x2 + c0;
  const Month m = x1 - 12 * c0 + 3;
  const Day d = floor_div(floor_mod(k1, 153), 5) + 1;
  return {y, m, d};
}


const flexint& MAX_CJDN() {
  static const flexint result = revisedjulian_to_cjdn(
    std::string(MAX_YEAR_DIGITS, '9'),
    12,
    31
  );
  return result;
}


bool check_cjdn(const flexint& cjdn) {
  if (cjdn < MIN_CJDN || cjdn > MAX_CJDN()) return false;
  return true;
}

} // namespace


namespace calendar_converter {


bool is_leap_year(const Year &year, Calendar calendar) {
  return is_leap(year, calendar);
}


int days_in_month(Month month, bool is_leap_year) {
  return m_length(month, is_leap_year);
}


/*static*/ Date Date::now_utc(Calendar calendar) {
  const std::chrono::year_month_day std_ymd {
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())
  };
  const auto now = Date (
    static_cast<int>(std_ymd.year()),
    static_cast<unsigned>(std_ymd.month()),
    static_cast<unsigned>(std_ymd.day()),
    Gregorian
  );
  return Date(now.ymd(calendar), calendar);
}


/*static*/ const Date& Date::min() {
  static const Date d (MIN_CJDN);
  return d;
}


/*static*/ const Date& Date::max() {
  static const Date d (MAX_CJDN());
  return d;
}


Date::Date(const Year &year, Month m, Day d, Calendar calendar) {
  if (!calendar_is_valid(calendar)) THROW<std::invalid_argument>(
      "invalid Calendar value: ",
      static_cast<int>(calendar)
  );
  if (year < MIN_YEAR) THROW<std::invalid_argument>(
      "invalid year value: must be >= ",
      MIN_YEAR
  );
  if (m < 1 || m > 12)
    THROW<std::invalid_argument>("invalid month value: ", m);
  if (d < 1 || d > days_in_month(m, is_leap(year, calendar)))
    THROW<std::invalid_argument>("invalid day value: ", d);
  switch (calendar) {
    case Julian:
      cjdn_ = julian_to_cjdn(year, m, d);
      break;
    case Gregorian:
      cjdn_ = gregorian_to_cjdn(year, m, d);
      break;
    case RevisedJulian:
      cjdn_ = revisedjulian_to_cjdn(year, m, d);
      break;
  }
  if (!check_cjdn(cjdn_)) THROW<std::invalid_argument>(
    "cjdn value is out of range [",
    MIN_CJDN,
    ", ",
    MAX_CJDN(),
    "]"
  );
}


Date::Date(const std::tuple<Year, Month, Day>& ymd, Calendar c)
  : Date(std::get<0>(ymd), std::get<1>(ymd), std::get<2>(ymd), c)
{
}


Date::Date(flexint cjdn) : cjdn_(std::move(cjdn)) {
  if (!check_cjdn(cjdn_)) THROW<std::invalid_argument>(
    "cjdn value is out of range [",
    MIN_CJDN,
    ", ",
    MAX_CJDN(),
    "]"
  );
}


std::tuple<Year, Month, Day> Date::ymd(Calendar calendar) const {
  switch (calendar) {
    case Julian:    return cjdn_to_julian(cjdn_);
    case Gregorian: return cjdn_to_gregorian(cjdn_);
    case RJulian:   return cjdn_to_revisedjulian(cjdn_);
  }
  throw std::invalid_argument(
      "invalid Calendar value: " +
      std::to_string(static_cast<int>(calendar))
  );
}


std::tuple<Month, Day> Date::md(Calendar calendar) const {
  const auto result = ymd(calendar);
  return {std::get<1>(result), std::get<2>(result)};
}


Year Date::year(Calendar calendar) const {
  return std::get<0>(ymd(calendar));
}


Month Date::month(Calendar calendar) const {
  return std::get<1>(ymd(calendar));
}


Day Date::day(Calendar calendar) const {
  return std::get<2>(ymd(calendar));
}


flexint Date::cjdn() const {
  return cjdn_;
}


WeekDay Date::weekday() const {
  static_assert(static_cast<int>(cc::WeekDay::Sun) == 0);
  static_assert(static_cast<int>(cc::WeekDay::Mon) == 1);
  static_assert(static_cast<int>(cc::WeekDay::Tue) == 2);
  static_assert(static_cast<int>(cc::WeekDay::Wed) == 3);
  static_assert(static_cast<int>(cc::WeekDay::Thu) == 4);
  static_assert(static_cast<int>(cc::WeekDay::Fri) == 5);
  static_assert(static_cast<int>(cc::WeekDay::Sat) == 6);
  const flexint q = cjdn_ + 1;
  const int x = static_cast<int>(floor_mod(q, 7));
  return static_cast<WeekDay>(x);
}


void Date::swap(Date& d) noexcept {
  std::swap(*this, d);
}


void swap(Date& d1, Date& d2) noexcept {
  d1.swap(d2);
}


Date Date::increment(const flexint& days) {
  flexint new_cjdn = cjdn_ + days;
  if (!check_cjdn(new_cjdn))
    THROW<std::overflow_error>("cjdn out of range after increment");
  return Date(new_cjdn);
}


Date Date::decrement(const flexint& days) {
  flexint new_cjdn = cjdn_ - days;
  if (!check_cjdn(new_cjdn))
    THROW<std::overflow_error>("cjdn out of range after decrement");
  return Date(new_cjdn);
}


} // namespace calendar_converter
