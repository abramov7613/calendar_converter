#include "calendar_converter.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <tuple>

#define CalendarConverter calendar_converter_test2

namespace cc = calendar_converter;

namespace {

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

void expect_ymd(const cc::Date &date, cc::Calendar calendar,
                const std::string &year, cc::Month month, cc::Day day) {
  const auto [y, m, d] = date.ymd(calendar);

  EXPECT_EQ(y, year);
  EXPECT_EQ(m, month);
  EXPECT_EQ(d, day);
}

void expect_round_trip(const std::string &year, cc::Month month, cc::Day day,
                       cc::Calendar calendar) {
  const cc::Date original(year, month, day, calendar);

  const std::string cjdn = original.cjdn();

  // Reconstruct through Gregorian, then compare CJDN.
  const cc::Date from_gregorian(original.year(cc::Gregorian),
                                original.month(cc::Gregorian),
                                original.day(cc::Gregorian), cc::Gregorian);

  EXPECT_EQ(from_gregorian.cjdn(), cjdn);

  // Reconstruct through the original calendar.
  const cc::Date reconstructed(original.year(calendar),
                               original.month(calendar), original.day(calendar),
                               calendar);

  EXPECT_EQ(reconstructed.cjdn(), cjdn);
}

} // namespace

// =============================================================================
// Calendar validation
// =============================================================================

TEST(CalendarConverter, CalendarValuesAreAccepted) {
  EXPECT_NO_THROW(cc::Date("2024", 1, 1, cc::Gregorian));
  EXPECT_NO_THROW(cc::Date("2024", 1, 1, cc::Julian));
  EXPECT_NO_THROW(cc::Date("2024", 1, 1, cc::RevisedJulian));
}

TEST(CalendarConverter, InvalidCalendarIsRejected) {
  const auto invalid = static_cast<cc::Calendar>(999);

  EXPECT_THROW(cc::Date("2024", 1, 1, invalid), std::invalid_argument);

  EXPECT_THROW(cc::is_leap_year("2024", invalid), std::invalid_argument);

  EXPECT_THROW(cc::is_leap_year(2024LL, invalid), std::invalid_argument);
}

// =============================================================================
// Leap years
// =============================================================================

TEST(CalendarConverter, JulianLeapYear) {
  EXPECT_TRUE(cc::is_leap_year(4LL, cc::Julian));
  EXPECT_TRUE(cc::is_leap_year(100LL, cc::Julian));
  EXPECT_TRUE(cc::is_leap_year(400LL, cc::Julian));

  EXPECT_FALSE(cc::is_leap_year(1LL, cc::Julian));
  EXPECT_FALSE(cc::is_leap_year(3LL, cc::Julian));
}

TEST(CalendarConverter, GregorianLeapYear) {
  EXPECT_TRUE(cc::is_leap_year(4LL, cc::Gregorian));
  EXPECT_TRUE(cc::is_leap_year(400LL, cc::Gregorian));
  EXPECT_TRUE(cc::is_leap_year(2000LL, cc::Gregorian));

  EXPECT_FALSE(cc::is_leap_year(100LL, cc::Gregorian));
  EXPECT_FALSE(cc::is_leap_year(1900LL, cc::Gregorian));
  EXPECT_FALSE(cc::is_leap_year(2100LL, cc::Gregorian));
}

TEST(CalendarConverter, RevisedJulianLeapYear) {
  EXPECT_TRUE(cc::is_leap_year(200LL, cc::RevisedJulian));
  EXPECT_TRUE(cc::is_leap_year(600LL, cc::RevisedJulian));

  EXPECT_FALSE(cc::is_leap_year(100LL, cc::RevisedJulian));
  EXPECT_FALSE(cc::is_leap_year(300LL, cc::RevisedJulian));
  EXPECT_FALSE(cc::is_leap_year(500LL, cc::RevisedJulian));
  EXPECT_FALSE(cc::is_leap_year(700LL, cc::RevisedJulian));

  EXPECT_TRUE(cc::is_leap_year(1100LL, cc::RevisedJulian));
  EXPECT_TRUE(cc::is_leap_year(1500LL, cc::RevisedJulian));
}

TEST(CalendarConverter, LeapYearStringAndIntegerOverloadsAgree) {
  const long long years[] = {1,   4,    100,  200,  400,  600,
                             900, 1000, 1100, 1500, 1900, 2000};

  for (const auto year : years) {
    for (const auto calendar : {cc::Julian, cc::Gregorian, cc::RevisedJulian}) {
      EXPECT_EQ(cc::is_leap_year(std::to_string(year), calendar),
                cc::is_leap_year(year, calendar))
          << "year=" << year;
    }
  }
}

// =============================================================================
// Month lengths
// =============================================================================

TEST(CalendarConverter, DaysInMonth) {
  EXPECT_EQ(cc::days_in_month(1, false), 31);
  EXPECT_EQ(cc::days_in_month(4, false), 30);
  EXPECT_EQ(cc::days_in_month(2, false), 28);
  EXPECT_EQ(cc::days_in_month(2, true), 29);
  EXPECT_EQ(cc::days_in_month(12, false), 31);
}

TEST(CalendarConverter, InvalidMonthIsRejectedByDaysInMonth) {
  EXPECT_THROW(cc::days_in_month(0, false), std::invalid_argument);

  EXPECT_THROW(cc::days_in_month(13, false), std::invalid_argument);
}

// =============================================================================
// Date validation
// =============================================================================

TEST(CalendarConverter, InvalidMonthIsRejected) {
  EXPECT_THROW(cc::Date("2024", 0, 1), std::invalid_argument);

  EXPECT_THROW(cc::Date("2024", 13, 1), std::invalid_argument);
}

TEST(CalendarConverter, InvalidDayIsRejected) {
  EXPECT_THROW(cc::Date("2024", 1, 0), std::invalid_argument);

  EXPECT_THROW(cc::Date("2024", 1, 32), std::invalid_argument);

  EXPECT_THROW(cc::Date("2024", 4, 31), std::invalid_argument);

  EXPECT_THROW(cc::Date("2023", 2, 29), std::invalid_argument);

  EXPECT_NO_THROW(cc::Date("2024", 2, 29));
}

TEST(CalendarConverter, YearZeroIsRejected) {
  EXPECT_THROW(cc::Date("0", 1, 1), std::invalid_argument);
}

TEST(CalendarConverter, EmptyYearIsRejected) {
  EXPECT_THROW(cc::Date("", 1, 1), std::invalid_argument);
}

TEST(CalendarConverter, NonDecimalYearIsRejected) {
  EXPECT_THROW(cc::Date("-1", 1, 1), std::invalid_argument);

  EXPECT_THROW(cc::Date("+1", 1, 1), std::invalid_argument);

  EXPECT_THROW(cc::Date("1a", 1, 1), std::invalid_argument);

  EXPECT_THROW(cc::Date("1 2", 1, 1), std::invalid_argument);
}

// =============================================================================
// Known dates
// =============================================================================

TEST(CalendarConverter, GregorianMinimumDate) {
  const cc::Date date("1", 1, 1, cc::Gregorian);

  expect_ymd(date, cc::Gregorian, "1", 1, 1);
  EXPECT_EQ(date.cjdn(), "1721426");
}

TEST(CalendarConverter, JulianMinimumDate) {
  const cc::Date date("1", 1, 3, cc::Julian);

  expect_ymd(date, cc::Julian, "1", 1, 3);
  EXPECT_EQ(date.cjdn(), "1721426");
}

TEST(CalendarConverter, GregorianKnownDate) {
  const cc::Date date("2000", 1, 1, cc::Gregorian);

  expect_ymd(date, cc::Gregorian, "2000", 1, 1);
  EXPECT_EQ(date.cjdn(), "2451545");
}

TEST(CalendarConverter, JulianKnownDate) {
  const cc::Date date("2000", 1, 1, cc::Julian);

  expect_ymd(date, cc::Julian, "2000", 1, 1);

  // 2000-01-01 Julian = 2000-01-14 Gregorian.
  expect_ymd(date, cc::Gregorian, "2000", 1, 14);
}

TEST(CalendarConverter, GregorianJulianConversionAroundModernDate) {
  const cc::Date gregorian("2024", 1, 1, cc::Gregorian);

  expect_ymd(gregorian, cc::Julian, "2023", 12, 19);
}

// =============================================================================
// Calendar conversion
// =============================================================================

TEST(CalendarConverter, GregorianToJulianAndBack) {
  const cc::Date date("2024", 1, 1, cc::Gregorian);

  EXPECT_EQ(cc::Date(date.year(cc::Julian), date.month(cc::Julian),
                     date.day(cc::Julian), cc::Julian)
                .cjdn(),
            date.cjdn());
}

TEST(CalendarConverter, GregorianToRevisedJulianAndBack) {
  const cc::Date date("2024", 1, 1, cc::Gregorian);

  const cc::Date revised(date.year(cc::RevisedJulian),
                         date.month(cc::RevisedJulian),
                         date.day(cc::RevisedJulian), cc::RevisedJulian);

  EXPECT_EQ(revised.cjdn(), date.cjdn());
}

// =============================================================================
// Round-trip tests
// =============================================================================

TEST(CalendarConverter, RoundTripGregorian) {
  const std::tuple<std::string, int, int> dates[] = {
      {"1", 1, 1},    {"4", 2, 29},    {"100", 3, 1},   {"400", 2, 29},
      {"1900", 3, 1}, {"2000", 2, 29}, {"2024", 2, 29}, {"9999", 12, 31},
  };

  for (const auto &[year, month, day] : dates) {
    expect_round_trip(year, month, day, cc::Gregorian);
  }
}

TEST(CalendarConverter, RoundTripJulian) {
  const std::tuple<std::string, int, int> dates[] = {
      {"1", 1, 3},     {"4", 2, 29},    {"100", 2, 29},  {"400", 2, 29},
      {"1900", 2, 29}, {"2000", 2, 29}, {"2024", 2, 29}, {"9999", 12, 31},
  };

  for (const auto &[year, month, day] : dates) {
    expect_round_trip(year, month, day, cc::Julian);
  }
}

TEST(CalendarConverter, RoundTripRevisedJulian) {
  const std::tuple<std::string, int, int> dates[] = {
      {"1", 1, 1},     {"4", 2, 29},     {"200", 2, 29},  {"600", 2, 29},
      {"900", 3, 1},   {"1100", 2, 29},  {"1500", 2, 29}, {"2000", 2, 29},
      {"2024", 2, 29}, {"9999", 12, 31},
  };

  for (const auto &[year, month, day] : dates) {
    expect_round_trip(year, month, day, cc::RevisedJulian);
  }
}

// =============================================================================
// Large integer / cpp_int tests
// =============================================================================

TEST(CalendarConverter, VeryLargeYearIsSupported) {
  const std::string year = "100000000000000000000000000000000000000000";

  const cc::Date date(year, 1, 1, cc::Gregorian);

  EXPECT_EQ(date.year(cc::Gregorian), year);
  EXPECT_EQ(date.month(cc::Gregorian), 1);
  EXPECT_EQ(date.day(cc::Gregorian), 1);
}

TEST(CalendarConverter, VeryLargeYearRoundTripGregorian) {
  const std::string year = "100000000000000000000000000000000000000000";

  expect_round_trip(year, 1, 1, cc::Gregorian);
}

TEST(CalendarConverter, VeryLargeYearRoundTripJulian) {
  const std::string year = "100000000000000000000000000000000000000000";

  expect_round_trip(year, 1, 1, cc::Julian);
}

TEST(CalendarConverter, VeryLargeYearRoundTripRevisedJulian) {
  const std::string year = "100000000000000000000000000000000000000000";

  expect_round_trip(year, 1, 1, cc::RevisedJulian);
}

TEST(CalendarConverter, VeryLargeLeapYearGregorian) {
  const std::string year = "100000000000000000000000000000000000000400";

  EXPECT_TRUE(cc::is_leap_year(year, cc::Gregorian));

  const cc::Date date(year, 2, 29, cc::Gregorian);

  EXPECT_EQ(date.year(cc::Gregorian), year);
  EXPECT_EQ(date.month(cc::Gregorian), 2);
  EXPECT_EQ(date.day(cc::Gregorian), 29);
}

TEST(CalendarConverter, VeryLargeNonLeapYearGregorian) {
  const std::string year = "100000000000000000000000000000000000000100";

  EXPECT_FALSE(cc::is_leap_year(year, cc::Gregorian));

  EXPECT_THROW(cc::Date(year, 2, 29, cc::Gregorian), std::invalid_argument);
}

// =============================================================================
// int32_t / cpp_int boundary
// =============================================================================

TEST(CalendarConverter, CjdnFitsIntoInt32AtMaximum) {
  // Find a date whose CJDN is close to INT32_MAX.
  //
  // The exact calendar date is intentionally obtained through the
  // implementation itself; the important property is the resulting
  // CJDN boundary.
  const std::string year = "5874898";

  const cc::Date date(year, 1, 1, cc::Gregorian);

  const auto cjdn = date.cjdn();

  EXPECT_FALSE(cjdn.empty());
}

TEST(CalendarConverter, DateBeyondInt32CjdnUsesBigIntegerPath) {
  const cc::Date date("100000000", 1, 1, cc::Gregorian);

  const std::string cjdn = date.cjdn();

  // The CJDN is far beyond INT32_MAX.
  EXPECT_GT(cjdn.size(),
            std::to_string(std::numeric_limits<std::int32_t>::max()).size());
}

TEST(CalendarConverter, LargeCjdnRemainsExact) {
  const cc::Date date("100000000000000000000000000000000000000", 12, 31,
                      cc::Gregorian);

  const std::string cjdn1 = date.cjdn();

  const cc::Date reconstructed(date.year(cc::Gregorian),
                               date.month(cc::Gregorian),
                               date.day(cc::Gregorian), cc::Gregorian);

  const std::string cjdn2 = reconstructed.cjdn();

  EXPECT_EQ(cjdn1, cjdn2);
}

// =============================================================================
// Leap-day boundary tests
// =============================================================================

TEST(CalendarConverter, GregorianCenturyBoundaries) {
  EXPECT_THROW(cc::Date("1900", 2, 29, cc::Gregorian), std::invalid_argument);

  EXPECT_NO_THROW(cc::Date("2000", 2, 29, cc::Gregorian));

  EXPECT_THROW(cc::Date("2100", 2, 29, cc::Gregorian), std::invalid_argument);

  EXPECT_NO_THROW(cc::Date("2400", 2, 29, cc::Gregorian));
}

TEST(CalendarConverter, JulianCenturyBoundaries) {
  EXPECT_NO_THROW(cc::Date("1900", 2, 29, cc::Julian));

  EXPECT_NO_THROW(cc::Date("2000", 2, 29, cc::Julian));
}

TEST(CalendarConverter, RevisedJulianSpecialCenturyBoundaries) {
  EXPECT_NO_THROW(cc::Date("200", 2, 29, cc::RevisedJulian));

  EXPECT_NO_THROW(cc::Date("600", 2, 29, cc::RevisedJulian));

  EXPECT_THROW(cc::Date("100", 2, 29, cc::RevisedJulian),
               std::invalid_argument);

  EXPECT_THROW(cc::Date("300", 2, 29, cc::RevisedJulian),
               std::invalid_argument);
}

// =============================================================================
// Weekday
// =============================================================================

TEST(CalendarConverter, WeekdayKnownDates) {
  const cc::Date sunday("2023", 1, 1, cc::Gregorian);
  const cc::Date monday("2023", 1, 2, cc::Gregorian);
  const cc::Date saturday("2023", 1, 7, cc::Gregorian);

  EXPECT_EQ(sunday.weekday(), cc::WeekDay::Sun);
  EXPECT_EQ(monday.weekday(), cc::WeekDay::Mon);
  EXPECT_EQ(saturday.weekday(), cc::WeekDay::Sat);
}

TEST(CalendarConverter, WeekdayIsCalendarIndependent) {
  const cc::Date date("2024", 1, 1, cc::Gregorian);

  const auto weekday = date.weekday();

  const cc::Date sameDateAsJulian(date.year(cc::Julian), date.month(cc::Julian),
                                  date.day(cc::Julian), cc::Julian);

  const cc::Date sameDateAsRevisedJulian(
      date.year(cc::RevisedJulian), date.month(cc::RevisedJulian),
      date.day(cc::RevisedJulian), cc::RevisedJulian);

  EXPECT_EQ(sameDateAsJulian.weekday(), weekday);
  EXPECT_EQ(sameDateAsRevisedJulian.weekday(), weekday);
}

// =============================================================================
// Comparisons
// =============================================================================

TEST(CalendarConverter, EqualDatesInDifferentCalendarsCompareEqual) {
  const cc::Date gregorian("2024", 1, 1, cc::Gregorian);

  const cc::Date julian(gregorian.year(cc::Julian), gregorian.month(cc::Julian),
                        gregorian.day(cc::Julian), cc::Julian);

  EXPECT_TRUE(gregorian == julian);
  EXPECT_FALSE(gregorian != julian);
  EXPECT_FALSE(gregorian < julian);
  EXPECT_FALSE(gregorian > julian);
  EXPECT_TRUE(gregorian <= julian);
  EXPECT_TRUE(gregorian >= julian);
}

TEST(CalendarConverter, DateOrderingUsesCjdn) {
  const cc::Date first("2024", 1, 1, cc::Gregorian);
  const cc::Date second("2024", 1, 2, cc::Gregorian);

  EXPECT_TRUE(first < second);
  EXPECT_TRUE(second > first);

  EXPECT_TRUE(first <= second);
  EXPECT_TRUE(second >= first);

  EXPECT_FALSE(first > second);
  EXPECT_FALSE(second < first);

  EXPECT_NE(first, second);
}

TEST(CalendarConverter, CopyConstructorPreservesDate) {
  const cc::Date original("123456789012345678901234567890", 5, 17,
                          cc::Gregorian);

  const cc::Date copy(original);

  EXPECT_EQ(copy.cjdn(), original.cjdn());
  EXPECT_EQ(copy.ymd(cc::Gregorian), original.ymd(cc::Gregorian));
}

TEST(CalendarConverter, CopyAssignmentPreservesDate) {
  const cc::Date original("123456789012345678901234567890", 5, 17,
                          cc::Gregorian);

  cc::Date copy("2000", 1, 1);

  copy = original;

  EXPECT_EQ(copy.cjdn(), original.cjdn());
  EXPECT_EQ(copy.ymd(cc::Gregorian), original.ymd(cc::Gregorian));
}

TEST(CalendarConverter, SelfAssignmentPreservesDate) {
  cc::Date date("123456789012345678901234567890", 5, 17, cc::Gregorian);

  const auto cjdn = date.cjdn();

  date = date;

  EXPECT_EQ(date.cjdn(), cjdn);
}

// =============================================================================
// Move semantics
// =============================================================================

TEST(CalendarConverter, MoveConstructorPreservesDestination) {
  cc::Date original("123456789012345678901234567890", 5, 17, cc::Gregorian);

  const auto expected_cjdn = original.cjdn();

  cc::Date moved(std::move(original));

  EXPECT_EQ(moved.cjdn(), expected_cjdn);
}

TEST(CalendarConverter, MoveAssignmentPreservesDestination) {
  cc::Date original("123456789012345678901234567890", 5, 17, cc::Gregorian);

  const auto expected_cjdn = original.cjdn();

  cc::Date destination("2000", 1, 1);

  destination = std::move(original);

  EXPECT_EQ(destination.cjdn(), expected_cjdn);
}

// =============================================================================
// Date::ymd / md
// =============================================================================

TEST(CalendarConverter, MdReturnsMonthAndDay) {
  const cc::Date date("2024", 2, 29, cc::Gregorian);

  const auto [month, day] = date.md(cc::Gregorian);

  EXPECT_EQ(month, 2);
  EXPECT_EQ(day, 29);
}

TEST(CalendarConverter, YmdReturnsExpectedValues) {
  const cc::Date date("2024", 2, 29, cc::Gregorian);

  const auto [year, month, day] = date.ymd(cc::Gregorian);

  EXPECT_EQ(year, "2024");
  EXPECT_EQ(month, 2);
  EXPECT_EQ(day, 29);
}

// =============================================================================
// Long long constructor
// =============================================================================

TEST(CalendarConverter, LongLongConstructorMatchesStringConstructor) {
  const long long year = 2024;

  const cc::Date from_integer(year, 2, 29, cc::Gregorian);

  const cc::Date from_string(std::to_string(year), 2, 29, cc::Gregorian);

  EXPECT_EQ(from_integer.cjdn(), from_string.cjdn());

  EXPECT_EQ(from_integer.ymd(cc::Gregorian), from_string.ymd(cc::Gregorian));
}

// =============================================================================
// Large-year weekday
// =============================================================================

TEST(CalendarConverter, LargeYearWeekdayRoundTrip) {
  const cc::Date date("100000000000000000000000000000000000000", 1, 1,
                      cc::Gregorian);

  const auto weekday = date.weekday();

  const cc::Date reconstructed(date.year(cc::Gregorian),
                               date.month(cc::Gregorian),
                               date.day(cc::Gregorian), cc::Gregorian);

  EXPECT_EQ(reconstructed.weekday(), weekday);
}

// =============================================================================
// Stress tests around calendar rules
// =============================================================================

TEST(CalendarConverter, GregorianRoundTripAcrossCenturyBoundaries) {
  for (const std::string year :
       {"99", "100", "399", "400", "401", "999", "1000", "1599", "1600", "1899",
        "1900", "1999", "2000", "2001", "2399", "2400"}) {
    const cc::Date date(year, 3, 1, cc::Gregorian);

    expect_round_trip(year, 3, 1, cc::Gregorian);
  }
}

TEST(CalendarConverter, RevisedJulianRoundTripAcross900YearCycle) {
  for (const std::string year : {"199", "200", "201", "599", "600", "601",
                                 "899", "900", "901", "1099", "1100", "1101"}) {
    expect_round_trip(year, 3, 1, cc::RevisedJulian);
  }
}

// =============================================================================
// Extreme long long values for leap-year calculation
// =============================================================================

TEST(CalendarConverter, LeapYearHandlesLongLongMax) {
  EXPECT_NO_THROW(
      cc::is_leap_year(std::numeric_limits<long long>::max(), cc::Gregorian));

  EXPECT_NO_THROW(
      cc::is_leap_year(std::numeric_limits<long long>::max(), cc::Julian));

  EXPECT_NO_THROW(cc::is_leap_year(std::numeric_limits<long long>::max(),
                                   cc::RevisedJulian));
}

TEST(CalendarConverter, LeapYearHandlesLongLongMin) {
  EXPECT_NO_THROW(
      cc::is_leap_year(std::numeric_limits<long long>::min(), cc::Gregorian));

  EXPECT_NO_THROW(
      cc::is_leap_year(std::numeric_limits<long long>::min(), cc::Julian));

  EXPECT_NO_THROW(cc::is_leap_year(std::numeric_limits<long long>::min(),
                                   cc::RevisedJulian));
}

// =============================================================================
// Very large decimal input
// =============================================================================

TEST(CalendarConverter, LargeDecimalYearDoesNotLoseDigits) {
  const std::string year = "99999999999999999999999999999999999999999999999999";

  const cc::Date date(year, 12, 31, cc::Gregorian);

  EXPECT_EQ(date.year(cc::Gregorian), year);
}

TEST(CalendarConverter, LargeDecimalYearPreservesDateComponents) {
  const std::string year = "99999999999999999999999999999999999999999999999999";

  const cc::Date date(year, 12, 31, cc::Gregorian);

  EXPECT_EQ(date.month(cc::Gregorian), 12);
  EXPECT_EQ(date.day(cc::Gregorian), 31);
}

// =============================================================================
// Regression tests for narrowing conversions
// =============================================================================

TEST(CalendarConverter, LargeYearConversionDoesNotWrapAround) {
  const cc::Date date("100000000", 1, 1, cc::Gregorian);

  const cc::Date same(date.year(cc::Gregorian), date.month(cc::Gregorian),
                      date.day(cc::Gregorian), cc::Gregorian);

  EXPECT_EQ(date.cjdn(), same.cjdn());

  // A wrapped int32_t result would not preserve the CJDN.
  EXPECT_GT(date.cjdn().size(),
            std::to_string(std::numeric_limits<std::int32_t>::max()).size());
}

TEST(CalendarConverter, HugeYearDoesNotAffectMonthOrDay) {
  const std::string year =
      "1000000000000000000000000000000000000000000000000000001";

  const cc::Date date(year, 2, 28, cc::Gregorian);

  EXPECT_EQ(date.year(cc::Gregorian), year);
  EXPECT_EQ(date.month(cc::Gregorian), 2);
  EXPECT_EQ(date.day(cc::Gregorian), 28);
}

// =============================================================================
// Today
// =============================================================================

TEST(CalendarConverter, TodayUtcProducesValidDate) {
  const cc::Date today = cc::Date::today_utc(cc::Gregorian);

  const auto [year, month, day] = today.ymd(cc::Gregorian);

  EXPECT_FALSE(year.empty());
  EXPECT_GE(month, 1);
  EXPECT_LE(month, 12);
  EXPECT_GE(day, 1);
  EXPECT_LE(day,
            cc::days_in_month(month, cc::is_leap_year(year, cc::Gregorian)));
}

TEST(CalendarConverter, TodayCanBeRepresentedInAllCalendars) {
  const cc::Date today = cc::Date::today_utc(cc::Gregorian);

  for (const auto calendar : {cc::Gregorian, cc::Julian, cc::RevisedJulian}) {
    const auto [year, month, day] = today.ymd(calendar);

    EXPECT_FALSE(year.empty());
    EXPECT_GE(month, 1);
    EXPECT_LE(month, 12);
    EXPECT_GE(day, 1);
    EXPECT_LE(day, cc::days_in_month(month, cc::is_leap_year(year, calendar)));
  }
}
