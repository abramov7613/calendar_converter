#include "calendar_converter.h"
#include <boost/multiprecision/cpp_int.hpp>
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

static_assert(static_cast<int>(cc::WeekDay::Sun) == 0);

static_assert(static_cast<int>(cc::WeekDay::Sat) == 6);

namespace {

using INT = boost::multiprecision::cpp_int;
using cc::Calendar;
using cc::Day;
using cc::Gregorian;
using cc::Julian;
using cc::Month;
using cc::RevisedJulian;
using cc::WeekDay;
using cc::Year;

constexpr std::int32_t MIN_CJDN = 1'721'426;
constexpr std::int32_t MIN_YEAR = 1;
constexpr std::size_t MAX_YEAR_DIGITS = 2048;

template <typename T>
concept CalendarInteger = std::same_as<T, std::int32_t> || std::same_as<T, INT>;

template <typename T>
concept SupportedInteger = std::same_as<T, int> || std::same_as<T, short> ||
    std::same_as<T, long> || std::same_as<T, long long> ||
    std::same_as<T, std::int16_t> || std::same_as<T, std::int32_t> ||
    std::same_as<T, std::int64_t> || std::same_as<T, INT>;

template <typename T>
concept Ostreamable = requires(std::ostream &out, T &&value) {
  out << std::forward<T>(value);
};

template <CalendarInteger Integer>
using CalculationInt = std::conditional_t<std::same_as<Integer, std::int32_t>,
                                          std::int64_t, Integer>;

constexpr bool calendar_is_valid(Calendar calendar) {
  switch (calendar) {
  case Gregorian:
  case Julian:
  case RevisedJulian:
    return true;
  }
  return false;
}

template <typename Exception = std::runtime_error, typename Arg,
          typename... Args>
requires(std::derived_from<Exception, std::exception> &&Ostreamable<Arg> &&
         (Ostreamable<Args> && ...)) void THROW(Arg &&arg, Args &&...args) {
  std::ostringstream out;
  out << std::forward<Arg>(arg);
  ((out << ' ' << std::forward<Args>(args)), ...);
  throw Exception(out.str());
}

INT string_to_big_int(const std::string &s) {
  if (s.empty()) {
    THROW<std::invalid_argument>("empty year");
  }
  for (const unsigned char ch : s) {
    if (ch < '0' || ch > '9') {
      THROW<std::invalid_argument>("invalid decimal year: '", s, "'");
    }
  }
  INT result = 0;
  for (const char ch : s) {
    result *= 10;
    result += ch - '0';
  }
  return result;
}

template <SupportedInteger Integer>
Integer floor_div(const Integer &a,
                  const Integer &b) { // Floor division: returns floor(a / b)
  if (b == 0)
    THROW<std::invalid_argument>("Division by zero");
  if constexpr (std::is_integral_v<Integer>) {
    if (a == std::numeric_limits<Integer>::min() && b == -1)
      THROW<std::overflow_error>("integer division overflow");
  }
  Integer q = a / b;
  Integer r = a % b;
  return (r != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
}

template <SupportedInteger Integer>
Integer floor_mod(const Integer &a, const Integer &b) {
  if (b <= 0)
    THROW<std::invalid_argument>("Modulus must be positive");
  Integer r = a % b;
  if (r < 0)
    r += b;
  return r;
}

template <SupportedInteger Integer>
std::pair<Integer, Integer>
euclidean_div(const Integer &a,
              const Integer &b) { // Euclidean division: a = q * b + r, where 0
                                  // <= r < abs(b)
  if (b == 0)
    THROW<std::invalid_argument>("Division by zero");
  Integer quotient, remainder;
  if constexpr (std::is_integral_v<Integer>) {
    if (a == std::numeric_limits<Integer>::min() && b == -1) {
      THROW<std::overflow_error>("integer division overflow");
    }
    quotient = a / b;
    remainder = a % b;
  } else {
    boost::multiprecision::divide_qr(a, b, quotient, remainder);
  }
  if (remainder < 0) {
    if (b > 0) {
      quotient -= 1;
      remainder += b;
    } else {
      quotient += 1;
      remainder -= b;
    }
  }
  return {quotient, remainder};
}

template <SupportedInteger Integer>
bool is_leap(const Integer &year, const Calendar calendar) {
  if (!calendar_is_valid(calendar))
    THROW<std::invalid_argument>("invalid Calendar value: ",
                                 static_cast<int>(calendar));
  switch (calendar) {
  case Julian:
    return floor_mod(year, Integer(4)) == 0;

  case Gregorian:
    return floor_mod(year, Integer(4)) == 0 &&
           (floor_mod(year, Integer(100)) != 0 ||
            floor_mod(year, Integer(400)) == 0);

  case RevisedJulian:
    return floor_mod(year, Integer(4)) == 0 &&
           (floor_mod(year, Integer(100)) != 0 ||
            floor_mod(year, Integer(900)) == 200 ||
            floor_mod(year, Integer(900)) == 600);
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

template <CalendarInteger Integer>
Integer gregorian_to_cjdn(const Integer &year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#3_1
{
  using Calc = CalculationInt<Integer>;
  const int c0 = floor_div(m - 3, 12);
  const int x1 = m - 12 * c0 - 3;
  const Calc x4 = Calc(year) + c0;
  auto [x3, x2] = euclidean_div(x4, Calc(100));
  Calc result = Calc(d) + 1721119;
  result += floor_div(Calc(146097) * x3, Calc(4));
  result += floor_div(Calc(36525) * x2, Calc(100));
  result += floor_div(Calc(153) * x1 + 2, Calc(5));
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(result), std::int64_t>) {
    assert(result <= std::numeric_limits<std::int32_t>::max());
  }
  return static_cast<Integer>(result);
}

template <CalendarInteger Integer>
Integer julian_to_cjdn(const Integer &year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#5_1
{
  using Calc = CalculationInt<Integer>;
  const int c0 = floor_div(m - 3, 12);
  const Calc j1 = floor_div(Calc(1461) * (Calc(year) + c0), Calc(4));
  // This expression is always small enough for int
  const int j2 = floor_div(153 * m - 1836 * c0 - 457, 5);
  Calc result = j1 + j2 + d + 1721117;
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(result), std::int64_t>) {
    assert(result <= std::numeric_limits<std::int32_t>::max());
  }
  return static_cast<Integer>(result);
}

template <CalendarInteger Integer>
Integer revisedjulian_to_cjdn(const Integer &year, Month m, Day d)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#4_1
{
  using Calc = CalculationInt<Integer>;
  const int c0 = floor_div(m - 3, 12);
  const int x1 = m - c0 * 12 - 3;
  const Calc x4 = Calc(year) + c0;
  const Calc x3 = floor_div(x4, Calc(100));
  const int x2 =
      static_cast<int>(floor_mod(x4, Calc(100))); // safe: 0 <= x2 < 100
  Calc result = Calc(d) + 1721119;
  result += floor_div(Calc(328718) * x3 + 6, Calc(9));
  result += floor_div(Calc(36525) * x2, Calc(100));
  result += floor_div(153 * x1 + 2, 5);
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(result), std::int64_t>) {
    assert(result <= std::numeric_limits<std::int32_t>::max());
  }
  return static_cast<Integer>(result);
}

template <CalendarInteger Integer>
std::tuple<Integer, Month, Day> cjdn_to_gregorian(const Integer &cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#3_2
{
  using Calc = CalculationInt<Integer>;
  auto [x3, r3] = euclidean_div(Calc(4) * Calc(cjdn) - 6884477, Calc(146097));
  const int r3i = static_cast<int>(r3); // safe: 0 <= r3 < 146097
  auto [x2, r2] = euclidean_div(100 * floor_div(r3i, 4) + 99, 36525);
  auto [x1, r1] = euclidean_div(5 * floor_div(r2, 100) + 2, 153);
  const int c0 = floor_div(x1 + 2, 12);
  const Day d = floor_div(r1, 5) + 1;
  const Month m = x1 - 12 * c0 + 3;
  const Calc y = x3 * 100 + x2 + c0;
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(y), std::int64_t>) {
    assert(y <= std::numeric_limits<std::int32_t>::max());
  }
  return {static_cast<Integer>(y), m, d};
}

template <CalendarInteger Integer>
std::tuple<Integer, Month, Day> cjdn_to_julian(const Integer &cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#5_2
{
  using Calc = CalculationInt<Integer>;
  const Calc y2 = Calc(cjdn) - 1721118;
  const Calc k2 = y2 * 4 + 3;
  const int k1 =
      5 * floor_div(static_cast<int>(floor_mod(
                        k2, Calc(1461))), // safe: 0 <= static_cast < 1461
                    4) +
      2;
  const int x1 = floor_div(k1, 153);
  const int c0 = floor_div(x1 + 2, 12);
  const Calc y = floor_div(k2, Calc(1461)) + c0;
  const Month m = x1 - 12 * c0 + 3;
  const Day d = floor_div(floor_mod(k1, 153), 5) + 1;
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(y), std::int64_t>) {
    assert(y <= std::numeric_limits<std::int32_t>::max());
  }
  return {static_cast<Integer>(y), m, d};
}

template <CalendarInteger Integer>
std::tuple<Integer, Month, Day> cjdn_to_revisedjulian(const Integer &cjdn)
// Dr Louis Strous's method:
// https://aa.quae.nl/en/reken/juliaansedag.html#4_2
{
  using Calc = CalculationInt<Integer>;
  const Calc k3 = Calc(9) * (Calc(cjdn) - 1721120) + 2;
  const Calc x3 = floor_div(k3, Calc(328718));
  const int k2 =
      100 * floor_div(static_cast<int>(floor_mod(k3, Calc(328718))), 9) + 99;
  const int x2 = floor_div(k2, 36525);
  const int k1 = floor_div(floor_mod(k2, 36525), 100) * 5 + 2;
  const int x1 = floor_div(k1, 153);
  const int c0 = floor_div(x1 + 2, 12);
  const Calc y = x3 * 100 + x2 + c0;
  const Month m = x1 - 12 * c0 + 3;
  const Day d = floor_div(floor_mod(k1, 153), 5) + 1;
  if constexpr (std::same_as<Integer, std::int32_t> &&
                std::same_as<decltype(y), std::int64_t>) {
    assert(y <= std::numeric_limits<std::int32_t>::max());
  }
  return {static_cast<Integer>(y), m, d};
}

} // namespace

namespace calendar_converter {

bool is_leap_year(const Year &year, Calendar calendar) {
  INT y{string_to_big_int(year)};
  return is_leap(y, calendar);
}

bool is_leap_year(long long year, Calendar calendar) {
  return is_leap(year, calendar);
}

int days_in_month(Month month, bool is_leap_year) {
  return m_length(month, is_leap_year);
}

class Date::Implementation {
public:
  static void check_valid_operation(const Date &obj) {
    if (!obj.pImpl)
      THROW("operation with moved object");
  }

  static void check_valid_comparison(const Date &lhs, const Date &rhs) {
    if (!lhs.pImpl || !rhs.pImpl)
      THROW("comparison with moved object");
  }

  virtual ~Implementation() noexcept = default;

  virtual std::unique_ptr<Implementation> clone() const = 0;

  virtual std::tuple<Year, Month, Day> ymd(const Calendar) const = 0;

  virtual INT cjdn() const = 0;

  virtual WeekDay weekday() const = 0;

  virtual Year year(const Calendar calendar) const {
    return std::get<0>(ymd(calendar));
  }

  virtual Month month(const Calendar calendar) const {
    return std::get<1>(ymd(calendar));
  }

  virtual Day day(const Calendar calendar) const {
    return std::get<2>(ymd(calendar));
  }

  virtual std::tuple<Month, Day> md(const Calendar calendar) const {
    const auto result = ymd(calendar);
    return {std::get<1>(result), std::get<2>(result)};
  }
}; // class Date::Implementation

template <typename T>
class DateCommonImplementation : public Date::Implementation {
  static_assert(CalendarInteger<T>);
  T cjdn_; // Chronological Julian Day Number

public:
  explicit DateCommonImplementation(T cjdn) : cjdn_(std::move(cjdn)) {}

  virtual std::unique_ptr<Date::Implementation> clone() const override {
    return std::make_unique<DateCommonImplementation>(*this);
  }

  virtual std::tuple<Year, Month, Day>
  ymd(const Calendar calendar) const override {
    if (!calendar_is_valid(calendar))
      THROW<std::invalid_argument>("invalid Calendar value: ",
                                   static_cast<int>(calendar));
    T y;
    Month m;
    Day d;
    switch (calendar) {
    case Julian:
      std::tie(y, m, d) = cjdn_to_julian(cjdn_);
      break;
    case Gregorian:
      std::tie(y, m, d) = cjdn_to_gregorian(cjdn_);
      break;
    case RevisedJulian:
      std::tie(y, m, d) = cjdn_to_revisedjulian(cjdn_);
      break;
    }
    if constexpr (std::is_integral_v<T>) {
      return {std::to_string(y), m, d};
    } else {
      return {y.str(), m, d};
    }
  }

  virtual INT cjdn() const override {
    if constexpr (std::is_integral_v<T>) {
      return INT(cjdn_);
    } else {
      return cjdn_;
    }
  }

  virtual WeekDay weekday() const override {
    using Calc = CalculationInt<T>;
    const Calc q = cjdn_ + 1;
    const int x = static_cast<int>(floor_mod(q, Calc(7)));
    return static_cast<WeekDay>(x);
  }
}; // class DateCommonImplementation

/*static*/ Date Date::today_utc(Calendar calendar) {
  const std::chrono::year_month_day std_ymd{
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
  const auto now = Date(static_cast<int>(std_ymd.year()),
                        static_cast<unsigned>(std_ymd.month()),
                        static_cast<unsigned>(std_ymd.day()), Gregorian);
  const auto [y, m, d] = now.ymd(calendar);
  return Date(y, m, d, calendar);
}

Date::Date(const Year &year, Month m, Day d, Calendar calendar) {
  using ShortDate = DateCommonImplementation<std::int32_t>;
  using LongDate = DateCommonImplementation<INT>;
  if (!calendar_is_valid(calendar))
    THROW<std::invalid_argument>("invalid Calendar value: ",
                                 static_cast<int>(calendar));
  if (m < 1 || m > 12)
    THROW<std::invalid_argument>("invalid month value: ", m);
  if (year.size() > MAX_YEAR_DIGITS)
    THROW<std::invalid_argument>(
        "year is too long: must be <= ", MAX_YEAR_DIGITS, " digits");
  INT y{string_to_big_int(year)};
  if (y < MIN_YEAR)
    THROW<std::invalid_argument>("invalid year value: must be >= ", MIN_YEAR);
  if (d < 1 || d > days_in_month(m, is_leap(y, calendar)))
    THROW<std::invalid_argument>("invalid day value: ", d);
  INT cjdn;
  switch (calendar) {
    static_assert(std::same_as<decltype(cjdn), INT>);
    static_assert(std::same_as<decltype(y), INT>);
  case Julian:
    cjdn = julian_to_cjdn(y, m, d);
    break;
  case Gregorian:
    cjdn = gregorian_to_cjdn(y, m, d);
    break;
  case RevisedJulian:
    cjdn = revisedjulian_to_cjdn(y, m, d);
    break;
  }
  if (cjdn < MIN_CJDN)
    THROW<std::invalid_argument>("the minimum acceptable date is [dd-mm-yy]: "
                                 "01-01-01 in Gregorian, and "
                                 "03-01-01 in Julian calendars");
  if (cjdn > std::numeric_limits<int32_t>::max()) {
    pImpl = std::make_unique<LongDate>(cjdn);
  } else {
    pImpl = std::make_unique<ShortDate>(static_cast<std::int32_t>(cjdn));
  }
}

Date::Date(long long y, Month m, Day d, Calendar c)
    : Date(std::to_string(y), m, d, c) {}

void Date::swap(Date &other) noexcept { pImpl.swap(other.pImpl); }

Date::Date(const Date &other) {
  Implementation::check_valid_operation(other);
  pImpl = other.pImpl->clone();
}

Date &Date::operator=(Date other) {
  swap(other);
  return *this;
}

bool Date::operator==(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() == rhs.pImpl->cjdn();
}

bool Date::operator!=(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() != rhs.pImpl->cjdn();
}

bool Date::operator<(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() < rhs.pImpl->cjdn();
}

bool Date::operator>(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() > rhs.pImpl->cjdn();
}

bool Date::operator<=(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() <= rhs.pImpl->cjdn();
}

bool Date::operator>=(const Date &rhs) const {
  Implementation::check_valid_comparison(*this, rhs);
  return pImpl->cjdn() >= rhs.pImpl->cjdn();
}

std::string Date::cjdn() const {
  Implementation::check_valid_operation(*this);
  return pImpl->cjdn().str();
}

Year Date::year(Calendar c) const {
  Implementation::check_valid_operation(*this);
  return pImpl->year(c);
}

Month Date::month(Calendar c) const {
  Implementation::check_valid_operation(*this);
  return pImpl->month(c);
}

Day Date::day(Calendar c) const {
  Implementation::check_valid_operation(*this);
  return pImpl->day(c);
}

WeekDay Date::weekday() const {
  Implementation::check_valid_operation(*this);
  return pImpl->weekday();
}

std::tuple<Year, Month, Day> Date::ymd(Calendar c) const {
  Implementation::check_valid_operation(*this);
  return pImpl->ymd(c);
}

std::tuple<Month, Day> Date::md(Calendar c) const {
  Implementation::check_valid_operation(*this);
  return pImpl->md(c);
}

Date::~Date() noexcept = default;
Date::Date(Date &&) noexcept = default;

} // namespace calendar_converter
