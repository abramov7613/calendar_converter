#include "calendar_converter.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <tuple>
#include <utility>

#define CalendarConverter calendar_converter_test1

namespace {

using namespace calendar_converter;

struct DateParts {
  std::string year;
  Month month;
  Day day;
};

DateParts parts(const Date &date, Calendar calendar) {
  auto [year, month, day] = date.ymd(calendar);
  return {year, month, day};
}

void expect_date(const Date &date, Calendar calendar, const char *expected_year,
                 Month expected_month, Day expected_day) {
  const auto actual = parts(date, calendar);

  EXPECT_EQ(actual.year, expected_year);
  EXPECT_EQ(actual.month, expected_month);
  EXPECT_EQ(actual.day, expected_day);
}

void expect_round_trip(const Date &date, Calendar calendar) {
  const auto original = parts(date, calendar);
  const auto converted =
      Date(original.year, original.month, original.day, calendar);

  EXPECT_EQ(converted, date);
  EXPECT_EQ(converted.cjdn(), date.cjdn());
}

TEST(CalendarConverter, DaysInMonth) {
  EXPECT_EQ(days_in_month(1, false), 31);
  EXPECT_EQ(days_in_month(2, false), 28);
  EXPECT_EQ(days_in_month(3, false), 31);
  EXPECT_EQ(days_in_month(4, false), 30);
  EXPECT_EQ(days_in_month(5, false), 31);
  EXPECT_EQ(days_in_month(6, false), 30);
  EXPECT_EQ(days_in_month(7, false), 31);
  EXPECT_EQ(days_in_month(8, false), 31);
  EXPECT_EQ(days_in_month(9, false), 30);
  EXPECT_EQ(days_in_month(10, false), 31);
  EXPECT_EQ(days_in_month(11, false), 30);
  EXPECT_EQ(days_in_month(12, false), 31);

  EXPECT_EQ(days_in_month(2, true), 29);
}

TEST(CalendarConverter, JulianLeapYears) {
  EXPECT_TRUE(is_leap_year(4LL, Julian));
  EXPECT_TRUE(is_leap_year(100LL, Julian));
  EXPECT_TRUE(is_leap_year(400LL, Julian));

  EXPECT_FALSE(is_leap_year(1LL, Julian));
  EXPECT_FALSE(is_leap_year(3LL, Julian));
  EXPECT_FALSE(is_leap_year(5LL, Julian));
}

TEST(CalendarConverter, GregorianLeapYears) {
  EXPECT_TRUE(is_leap_year(4LL, Gregorian));
  EXPECT_TRUE(is_leap_year(400LL, Gregorian));
  EXPECT_TRUE(is_leap_year(2000LL, Gregorian));

  EXPECT_FALSE(is_leap_year(100LL, Gregorian));
  EXPECT_FALSE(is_leap_year(1700LL, Gregorian));
  EXPECT_FALSE(is_leap_year(1800LL, Gregorian));
  EXPECT_FALSE(is_leap_year(1900LL, Gregorian));
  EXPECT_FALSE(is_leap_year(2100LL, Gregorian));
}

TEST(CalendarConverter, RevisedJulianLeapYears) {
  EXPECT_TRUE(is_leap_year(4LL, RevisedJulian));

  EXPECT_FALSE(is_leap_year(100LL, RevisedJulian));
  EXPECT_TRUE(is_leap_year(200LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(300LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(400LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(500LL, RevisedJulian));
  EXPECT_TRUE(is_leap_year(600LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(700LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(800LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(900LL, RevisedJulian));

  EXPECT_TRUE(is_leap_year(1100LL, RevisedJulian));
  EXPECT_TRUE(is_leap_year(1500LL, RevisedJulian));
}

TEST(CalendarConverter, LeapYearStringInput) {
  EXPECT_TRUE(is_leap_year("2000", Gregorian));
  EXPECT_FALSE(is_leap_year("1900", Gregorian));

  EXPECT_TRUE(is_leap_year("200", RevisedJulian));
  EXPECT_TRUE(is_leap_year("600", RevisedJulian));
  EXPECT_FALSE(is_leap_year("100", RevisedJulian));
}

TEST(CalendarConverter, GregorianKnownDates) {
  Date date("2000", 1, 1, Gregorian);

  expect_date(date, Gregorian, "2000", 1, 1);
  EXPECT_EQ(date.cjdn(), "2451545");

  Date another("2024", 2, 29, Gregorian);
  expect_date(another, Gregorian, "2024", 2, 29);
}

TEST(CalendarConverter, JulianKnownDates) {
  Date date("2000", 1, 1, Julian);

  expect_date(date, Julian, "2000", 1, 1);

  Date leap_day("1900", 2, 29, Julian);
  expect_date(leap_day, Julian, "1900", 2, 29);
}

TEST(CalendarConverter, GregorianJulianCalendarDifference) {
  Date gregorian("1900", 3, 1, Gregorian);

  expect_date(gregorian, Julian, "1900", 2, 17);

  Date julian("1900", 2, 17, Julian);

  EXPECT_EQ(julian, gregorian);
  EXPECT_EQ(julian.cjdn(), gregorian.cjdn());
}

TEST(CalendarConverter, GregorianJulianDifferenceAround1900) {
  Date gregorian("1900", 2, 28, Gregorian);

  expect_date(gregorian, Julian, "1900", 2, 16);

  Date gregorian_march("1900", 3, 1, Gregorian);

  expect_date(gregorian_march, Julian, "1900", 2, 17);
}

TEST(CalendarConverter, GregorianJulianDifferenceAround2000) {
  Date gregorian("2000", 1, 1, Gregorian);

  expect_date(gregorian, Julian, "1999", 12, 19);

  Date julian("1999", 12, 19, Julian);

  EXPECT_EQ(julian, gregorian);
}

TEST(CalendarConverter, RevisedJulianConversion) {
  Date date("2000", 1, 1, RevisedJulian);

  expect_date(date, RevisedJulian, "2000", 1, 1);

  Date gregorian("2000", 1, 1, Gregorian);

  EXPECT_EQ(date, gregorian);
}

TEST(CalendarConverter, MinimumSupportedDate) {
  Date date1("1", 1, 3, Julian);
  Date date2("1", 1, 1, Gregorian);
  Date date3("1", 1, 1, RevisedJulian);
  const std::string min_cjdn = "1721426";

  EXPECT_EQ(date1.cjdn(), min_cjdn);
  expect_date(date1, Julian, "1", 1, 3);

  EXPECT_EQ(date2.cjdn(), min_cjdn);
  expect_date(date2, Gregorian, "1", 1, 1);

  EXPECT_EQ(date3.cjdn(), min_cjdn);
  expect_date(date3, RevisedJulian, "1", 1, 1);
}

TEST(CalendarConverter, YearWithLeadingZeros) {
  Date date("0001", 1, 1, Gregorian);

  EXPECT_EQ(date.cjdn(), "1721426");
  expect_date(date, Gregorian, "1", 1, 1);
}

TEST(CalendarConverter, VeryLargeYear) {
  Date date("1000000000000000000000000000000000000", 1, 1, Gregorian);

  expect_date(date, Gregorian, "1000000000000000000000000000000000000", 1, 1);

  EXPECT_FALSE(date.cjdn().empty());
}

TEST(CalendarConverter, LargeYearRoundTrip) {
  const std::string year =
      "1000000000000000000000000000000000000000000000000000001";

  Date date(year, 12, 31, Gregorian);

  expect_round_trip(date, Gregorian);
}

TEST(CalendarConverter, GregorianRoundTripKnownDates) {
  const std::array<DateParts, 12> dates{{
      {"1", 1, 1},
      {"4", 2, 29},
      {"100", 12, 31},
      {"400", 2, 29},
      {"1000", 3, 1},
      {"1500", 2, 28},
      {"1582", 10, 15},
      {"1600", 2, 29},
      {"1700", 3, 1},
      {"1900", 3, 1},
      {"2000", 2, 29},
      {"2024", 2, 29},
  }};

  for (const auto &p : dates) {
    Date date(p.year, p.month, p.day, Gregorian);
    expect_round_trip(date, Gregorian);
  }
}

TEST(CalendarConverter, JulianRoundTripKnownDates) {
  const std::array<DateParts, 9> dates{{
      {"4", 2, 29},
      {"100", 2, 29},
      {"400", 2, 29},
      {"1000", 3, 1},
      {"1500", 2, 29},
      {"1700", 2, 29},
      {"1800", 2, 29},
      {"1900", 2, 29},
      {"2000", 2, 29},
  }};

  for (const auto &p : dates) {
    Date date(p.year, p.month, p.day, Julian);
    expect_round_trip(date, Julian);
  }
}

TEST(CalendarConverter, RevisedJulianRoundTripKnownDates) {
  const std::array<DateParts, 12> dates{{
      {"1", 1, 1},
      {"4", 2, 29},
      {"100", 2, 28},
      {"200", 2, 29},
      {"400", 2, 28},
      {"600", 2, 29},
      {"800", 2, 28},
      {"900", 2, 28},
      {"1000", 3, 1},
      {"1500", 2, 29},
      {"1900", 3, 1},
      {"2000", 2, 29},
  }};

  for (const auto &p : dates) {
    Date date(p.year, p.month, p.day, RevisedJulian);
    expect_round_trip(date, RevisedJulian);
  }
}

TEST(CalendarConverter, CrossCalendarRoundTrip) {
  const std::array<DateParts, 7> dates{{
      {"1000", 6, 15},
      {"1500", 2, 28},
      {"1582", 10, 15},
      {"1700", 3, 1},
      {"1900", 3, 1},
      {"2000", 2, 29},
      {"2024", 12, 31},
  }};

  constexpr std::array<Calendar, 3> calendars{Julian, Gregorian, RevisedJulian};

  for (const Calendar source_calendar : calendars) {
    for (const auto &p : dates) {
      Date original(p.year, p.month, p.day, source_calendar);

      for (const Calendar target_calendar : calendars) {
        const auto converted = original.ymd(target_calendar);

        Date restored(std::get<0>(converted), std::get<1>(converted),
                      std::get<2>(converted), target_calendar);

        EXPECT_EQ(restored, original)
            << "source calendar = " << static_cast<int>(source_calendar)
            << ", target calendar = " << static_cast<int>(target_calendar)
            << ", year = " << p.year << ", month = " << p.month
            << ", day = " << p.day;
      }
    }
  }
}

TEST(CalendarConverter, MonthAndDayAccessors) {
  Date date("2024", 2, 29, Gregorian);

  EXPECT_EQ(date.year(Gregorian), "2024");
  EXPECT_EQ(date.month(Gregorian), 2);
  EXPECT_EQ(date.day(Gregorian), 29);

  auto [month, day] = date.md(Gregorian);

  EXPECT_EQ(month, 2);
  EXPECT_EQ(day, 29);
}

TEST(CalendarConverter, Comparisons) {
  Date d1("2024", 1, 1, Gregorian);
  Date d2("2024", 1, 2, Gregorian);
  Date d3("2024", 1, 1, Gregorian);

  EXPECT_TRUE(d1 == d3);
  EXPECT_FALSE(d1 != d3);

  EXPECT_TRUE(d1 != d2);
  EXPECT_TRUE(d1 < d2);
  EXPECT_TRUE(d2 > d1);
  EXPECT_TRUE(d1 <= d2);
  EXPECT_TRUE(d1 <= d3);
  EXPECT_TRUE(d2 >= d1);
  EXPECT_TRUE(d3 >= d1);
}

TEST(CalendarConverter, SameAbsoluteDateAcrossCalendarsComparesEqual) {
  Date gregorian("2000", 1, 1, Gregorian);
  Date julian("1999", 12, 19, Julian);

  EXPECT_EQ(gregorian, julian);
  EXPECT_EQ(gregorian.cjdn(), julian.cjdn());
}

TEST(CalendarConverter, WeekdayKnownDates) {
  Date saturday("2000", 1, 1, Gregorian);
  EXPECT_EQ(saturday.weekday(), WeekDay::Sat);

  Date monday("2000", 1, 3, Gregorian);
  EXPECT_EQ(monday.weekday(), WeekDay::Mon);

  Date thursday("2024", 2, 29, Gregorian);
  EXPECT_EQ(thursday.weekday(), WeekDay::Thu);
}

TEST(CalendarConverter, WeekdayIsCalendarIndependent) {
  Date gregorian("2000", 1, 1, Gregorian);

  EXPECT_EQ(gregorian.weekday(), WeekDay::Sat);

  const auto julian_parts = gregorian.ymd(Julian);

  Date julian(std::get<0>(julian_parts), std::get<1>(julian_parts),
              std::get<2>(julian_parts), Julian);

  EXPECT_EQ(julian.weekday(), WeekDay::Sat);
}

TEST(CalendarConverter, AllWeekdaysRepeatEverySevenDays) {
  Date sunday("2024", 9, 1, Gregorian);

  EXPECT_EQ(sunday.weekday(), WeekDay::Sun);

  Date monday("2024", 9, 2, Gregorian);
  Date tuesday("2024", 9, 3, Gregorian);
  Date wednesday("2024", 9, 4, Gregorian);
  Date thursday("2024", 9, 5, Gregorian);
  Date friday("2024", 9, 6, Gregorian);
  Date saturday("2024", 9, 7, Gregorian);

  EXPECT_EQ(monday.weekday(), WeekDay::Mon);
  EXPECT_EQ(tuesday.weekday(), WeekDay::Tue);
  EXPECT_EQ(wednesday.weekday(), WeekDay::Wed);
  EXPECT_EQ(thursday.weekday(), WeekDay::Thu);
  EXPECT_EQ(friday.weekday(), WeekDay::Fri);
  EXPECT_EQ(saturday.weekday(), WeekDay::Sat);
}

TEST(CalendarConverter, InvalidMonth) {
  EXPECT_THROW(Date("2024", 0, 1, Gregorian), std::invalid_argument);

  EXPECT_THROW(Date("2024", 13, 1, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidDay) {
  EXPECT_THROW(Date("2023", 2, 29, Gregorian), std::invalid_argument);

  EXPECT_NO_THROW(Date("2024", 2, 29, Gregorian));

  EXPECT_THROW(Date("2024", 4, 31, Gregorian), std::invalid_argument);

  EXPECT_THROW(Date("2024", 1, 0, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, JulianAllowsFebruary29In1900) {
  EXPECT_NO_THROW(Date("1900", 2, 29, Julian));

  EXPECT_THROW(Date("1900", 2, 29, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, GregorianCenturyRules) {
  EXPECT_THROW(Date("1700", 2, 29, Gregorian), std::invalid_argument);

  EXPECT_THROW(Date("1800", 2, 29, Gregorian), std::invalid_argument);

  EXPECT_THROW(Date("1900", 2, 29, Gregorian), std::invalid_argument);

  EXPECT_NO_THROW(Date("2000", 2, 29, Gregorian));
}

TEST(CalendarConverter, RevisedJulianCenturyRules) {
  EXPECT_TRUE(is_leap_year(200LL, RevisedJulian));
  EXPECT_TRUE(is_leap_year(600LL, RevisedJulian));

  EXPECT_FALSE(is_leap_year(100LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(300LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(400LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(500LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(700LL, RevisedJulian));
  EXPECT_FALSE(is_leap_year(800LL, RevisedJulian));
}

TEST(CalendarConverter, CopyConstructor) {
  Date original("2024", 2, 29, Gregorian);
  Date copy(original);

  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy.cjdn(), original.cjdn());

  EXPECT_EQ(copy.year(Gregorian), "2024");
  EXPECT_EQ(copy.month(Gregorian), 2);
  EXPECT_EQ(copy.day(Gregorian), 29);
}

TEST(CalendarConverter, CopyAssignment) {
  Date original("2024", 2, 29, Gregorian);
  Date copy("2000", 1, 1, Gregorian);

  copy = original;

  EXPECT_EQ(copy, original);
  EXPECT_EQ(copy.cjdn(), original.cjdn());
}

TEST(CalendarConverter, MoveConstructor) {
  Date original("2024", 2, 29, Gregorian);
  const auto original_cjdn = original.cjdn();

  Date moved(std::move(original));

  EXPECT_EQ(moved.cjdn(), original_cjdn);
  EXPECT_EQ(moved.year(Gregorian), "2024");
  EXPECT_EQ(moved.month(Gregorian), 2);
  EXPECT_EQ(moved.day(Gregorian), 29);
}

TEST(CalendarConverter, MoveAssignment) {
  Date original("2024", 2, 29, Gregorian);
  const auto original_cjdn = original.cjdn();

  Date target("2000", 1, 1, Gregorian);

  target = std::move(original);

  EXPECT_EQ(target.cjdn(), original_cjdn);
  EXPECT_EQ(target.year(Gregorian), "2024");
  EXPECT_EQ(target.month(Gregorian), 2);
  EXPECT_EQ(target.day(Gregorian), 29);
}

TEST(CalendarConverter, Swap) {
  Date first("2024", 2, 29, Gregorian);
  Date second("2000", 1, 1, Gregorian);

  const auto first_cjdn = first.cjdn();
  const auto second_cjdn = second.cjdn();

  first.swap(second);

  EXPECT_EQ(first.cjdn(), second_cjdn);
  EXPECT_EQ(second.cjdn(), first_cjdn);
}

TEST(CalendarConverter, FreeSwap) {
  Date first("2024", 2, 29, Gregorian);
  Date second("2000", 1, 1, Gregorian);

  swap(first, second);

  EXPECT_EQ(first.year(Gregorian), "2000");
  EXPECT_EQ(second.year(Gregorian), "2024");
}

TEST(CalendarConverter, GregorianRoundTripRandomized) {
  std::mt19937_64 rng(0xC0FFEE);

  std::uniform_int_distribution<int> year_distribution(1, 50000);
  std::uniform_int_distribution<int> month_distribution(1, 12);

  for (int i = 0; i < 10000; ++i) {
    const int year = year_distribution(rng);
    const int month = month_distribution(rng);

    const bool leap = is_leap_year(static_cast<long long>(year), Gregorian);

    const int max_day = days_in_month(month, leap);

    std::uniform_int_distribution<int> day_distribution(1, max_day);

    const int day = day_distribution(rng);

    Date date(year, month, day, Gregorian);

    expect_round_trip(date, Gregorian);
  }
}

TEST(CalendarConverter, JulianRoundTripRandomized) {
  std::mt19937_64 rng(0x12345678);

  std::uniform_int_distribution<int> year_distribution(1, 50000);
  std::uniform_int_distribution<int> month_distribution(1, 12);

  for (int i = 0; i < 10000; ++i) {
    const int year = year_distribution(rng);
    const int month = month_distribution(rng);

    const bool leap = is_leap_year(static_cast<long long>(year), Julian);

    const int max_day = days_in_month(month, leap);

    std::uniform_int_distribution<int> day_distribution(1, max_day);

    const int day = day_distribution(rng);

    Date date(year, month, day, Julian);

    expect_round_trip(date, Julian);
  }
}

TEST(CalendarConverter, RevisedJulianRoundTripRandomized) {
  std::mt19937_64 rng(0x87654321);

  std::uniform_int_distribution<int> year_distribution(1, 50000);
  std::uniform_int_distribution<int> month_distribution(1, 12);

  for (int i = 0; i < 10000; ++i) {
    const int year = year_distribution(rng);
    const int month = month_distribution(rng);

    const bool leap = is_leap_year(static_cast<long long>(year), RevisedJulian);

    const int max_day = days_in_month(month, leap);

    std::uniform_int_distribution<int> day_distribution(1, max_day);

    const int day = day_distribution(rng);

    Date date(year, month, day, RevisedJulian);

    expect_round_trip(date, RevisedJulian);
  }
}

TEST(CalendarConverter, WeekdayRandomizedConsistency) {
  std::mt19937_64 rng(0xDEADBEEF);

  std::uniform_int_distribution<int> year_distribution(1, 10000);
  std::uniform_int_distribution<int> month_distribution(1, 12);

  for (int i = 0; i < 5000; ++i) {
    const int year = year_distribution(rng);
    const int month = month_distribution(rng);

    const bool leap = is_leap_year(static_cast<long long>(year), Gregorian);

    const int max_day = days_in_month(month, leap);

    std::uniform_int_distribution<int> day_distribution(1, max_day);

    const int day = day_distribution(rng);

    Date date(year, month, day, Gregorian);

    auto next = Date::today_utc();

    if (day < max_day) {
      next = Date(year, month, day + 1, Gregorian);
    } else if (month < 12) {
      next = Date(year, month + 1, 1, Gregorian);
    } else {
      next = Date(year + 1, 1, 1, Gregorian);
    }

    const int weekday = static_cast<int>(date.weekday());

    const int next_weekday = static_cast<int>(next.weekday());

    EXPECT_EQ(next_weekday, (weekday + 1) % 7);
  }
}

TEST(CalendarConverter, DateOrderingFollowsCJDN) {
  Date a("2024", 1, 1, Gregorian);
  Date b("2024", 1, 2, Gregorian);
  Date c("2024", 1, 3, Gregorian);

  EXPECT_LT(a.cjdn(), b.cjdn());
  EXPECT_LT(b.cjdn(), c.cjdn());

  EXPECT_LT(a, b);
  EXPECT_LT(b, c);
  EXPECT_LT(a, c);

  EXPECT_GT(c, b);
  EXPECT_GT(b, a);
  EXPECT_GT(c, a);
}

TEST(CalendarConverter, DifferentCalendarRepresentationsHaveSameOrdering) {
  Date gregorian("2000", 1, 1, Gregorian);
  Date julian("1999", 12, 19, Julian);

  Date next_gregorian("2000", 1, 2, Gregorian);
  Date next_julian("1999", 12, 20, Julian);

  EXPECT_EQ(gregorian, julian);
  EXPECT_EQ(next_gregorian, next_julian);

  EXPECT_LT(gregorian, next_gregorian);
  EXPECT_LT(julian, next_julian);
}

TEST(CalendarConverter, GregorianRevisedJulianAgreementBefore100) {
  for (int year = 1; year < 100; ++year) {
    for (int month = 1; month <= 12; ++month) {
      const bool leap = is_leap_year(static_cast<long long>(year), Gregorian);

      const int max_day = days_in_month(month, leap);

      for (int day = 1; day <= max_day; ++day) {
        Date gregorian(year, month, day, Gregorian);

        Date revised_julian(year, month, day, RevisedJulian);

        EXPECT_EQ(gregorian, revised_julian)
            << "date = " << year << "-" << month << "-" << day;
      }
    }
  }
}

} // namespace
