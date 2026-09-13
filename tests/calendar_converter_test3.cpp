#include <gtest/gtest.h>

#include "calendar_converter.h"

#include <chrono>
#include <string>
#include <tuple>
#include <vector>

#define CalendarConverter calendar_converter_test3

using namespace calendar_converter;

namespace {

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

struct DateCase {
  const char *year;
  Month month;
  Day day;
};

auto toDate(const DateCase &in, Calendar calendar) {
  return Date(in.year, in.month, in.day, calendar);
}

void expect_ymd(const Date &date, Calendar calendar, const std::string &year,
                Month month, Day day) {
  EXPECT_EQ(date.year(calendar), year);
  EXPECT_EQ(date.month(calendar), month);
  EXPECT_EQ(date.day(calendar), day);

  const auto [y, m, d] = date.ymd(calendar);

  EXPECT_EQ(y, year);
  EXPECT_EQ(m, month);
  EXPECT_EQ(d, day);

  const auto [mm, dd] = date.md(calendar);

  EXPECT_EQ(mm, month);
  EXPECT_EQ(dd, day);
}

void expect_all_equals(const DateCase &date_gregorian,
                       const DateCase &date_julian,
                       const DateCase &date_revisedjulian) {
  auto d1 = toDate(date_gregorian, Gregorian);
  auto d2 = toDate(date_julian, Julian);
  auto d3 = toDate(date_revisedjulian, RevisedJulian);

  EXPECT_EQ(d1, d2);
  EXPECT_EQ(d2, d3);
  {
    auto [y, m, d] = d1.ymd(Julian);
    EXPECT_EQ(d2, Date(y, m, d, Julian));
  }
  {
    auto [y, m, d] = d1.ymd(RevisedJulian);
    EXPECT_EQ(d3, Date(y, m, d, RevisedJulian));
  }

  {
    auto [y, m, d] = d2.ymd(Gregorian);
    EXPECT_EQ(d1, Date(y, m, d, Gregorian));
  }
  {
    auto [y, m, d] = d2.ymd(RevisedJulian);
    EXPECT_EQ(d3, Date(y, m, d, RevisedJulian));
  }

  {
    auto [y, m, d] = d3.ymd(Gregorian);
    EXPECT_EQ(d1, Date(y, m, d, Gregorian));
  }
  {
    auto [y, m, d] = d3.ymd(Julian);
    EXPECT_EQ(d2, Date(y, m, d, Julian));
  }
}

// -----------------------------------------------------------------------------
// Calendar
// -----------------------------------------------------------------------------

TEST(CalendarConverter, CalendarValues) {
  EXPECT_EQ(Julian, Calendar::J);
  EXPECT_EQ(RevisedJulian, Calendar::R);
  EXPECT_EQ(Gregorian, Calendar::G);
}

// -----------------------------------------------------------------------------
// days_in_month
// -----------------------------------------------------------------------------

TEST(CalendarConverter, ThirtyOneDayMonths) {
  for (const Month month : {1, 3, 5, 7, 8, 10, 12}) {
    EXPECT_EQ(days_in_month(month, false), 31);
    EXPECT_EQ(days_in_month(month, true), 31);
  }
}

TEST(CalendarConverter, ThirtyDayMonths) {
  for (const Month month : {4, 6, 9, 11}) {
    EXPECT_EQ(days_in_month(month, false), 30);
    EXPECT_EQ(days_in_month(month, true), 30);
  }
}

TEST(CalendarConverter, MonthLengthTestFebruary) {
  EXPECT_EQ(days_in_month(2, false), 28);
  EXPECT_EQ(days_in_month(2, true), 29);
}

TEST(CalendarConverter, InvalidMonthBelowRange) {
  EXPECT_THROW(days_in_month(0, false), std::invalid_argument);
}

TEST(CalendarConverter, InvalidMonthAboveRange) {
  EXPECT_THROW(days_in_month(13, false), std::invalid_argument);
}

// -----------------------------------------------------------------------------
// leap years
// -----------------------------------------------------------------------------

TEST(CalendarConverter, LeapYears) {
  EXPECT_FALSE(is_leap_year("1", Gregorian));
  EXPECT_FALSE(is_leap_year("3", Gregorian));
  EXPECT_TRUE(is_leap_year("4", Gregorian));
  EXPECT_FALSE(is_leap_year("5", Gregorian));

  EXPECT_FALSE(is_leap_year("100", Gregorian));
  EXPECT_FALSE(is_leap_year("1900", Gregorian));

  EXPECT_TRUE(is_leap_year("400", Gregorian));
  EXPECT_TRUE(is_leap_year("2000", Gregorian));

  EXPECT_TRUE(is_leap_year("4", Julian));
  EXPECT_FALSE(is_leap_year("5", Julian));
  EXPECT_TRUE(is_leap_year("100", Julian));

  EXPECT_TRUE(is_leap_year("4", RevisedJulian));
  EXPECT_FALSE(is_leap_year("5", RevisedJulian));
  EXPECT_TRUE(is_leap_year("2024", RevisedJulian));
  EXPECT_FALSE(is_leap_year("2023", RevisedJulian));

  EXPECT_TRUE(is_leap_year("2000", RevisedJulian));
  EXPECT_TRUE(is_leap_year("2400", RevisedJulian));

  EXPECT_FALSE(is_leap_year("1900", RevisedJulian));
  EXPECT_FALSE(is_leap_year("2100", RevisedJulian));
  EXPECT_FALSE(is_leap_year("2800", RevisedJulian));

  EXPECT_TRUE(is_leap_year("2900", RevisedJulian));
}

// -----------------------------------------------------------------------------
// Invalid year
// -----------------------------------------------------------------------------

TEST(CalendarConverter, InvalidString) {
  EXPECT_THROW(is_leap_year("abc", Gregorian), std::invalid_argument);

  EXPECT_THROW(Date("abc", 1, 1, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, EmptyString) {
  EXPECT_THROW(Date("", 1, 1, Gregorian), std::invalid_argument);
}

// -----------------------------------------------------------------------------
// Invalid Calendar
// -----------------------------------------------------------------------------

TEST(CalendarConverter, InvalidFormatForDate) {
  const auto invalid = static_cast<Calendar>(255);

  EXPECT_THROW(Date("2024", 1, 1, invalid), std::invalid_argument);
}

TEST(CalendarConverter, InvalidFormatForLeapYear) {
  const auto invalid = static_cast<Calendar>(255);

  EXPECT_THROW(is_leap_year("2024", invalid), std::invalid_argument);
}

TEST(CalendarConverter, InvalidFormatForYmd) {
  const Date date("2024", 1, 1, Gregorian);

  const auto invalid = static_cast<Calendar>(255);

  EXPECT_THROW(date.ymd(invalid), std::invalid_argument);
}

// -----------------------------------------------------------------------------
// Date validation
// -----------------------------------------------------------------------------

TEST(CalendarConverter, InvalidMonthZero) {
  EXPECT_THROW(Date("2024", 0, 1, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidMonthThirteen) {
  EXPECT_THROW(Date("2024", 13, 1, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidDayZero) {
  EXPECT_THROW(Date("2024", 1, 0, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidDayTooLarge) {
  EXPECT_THROW(Date("2024", 1, 32, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidFebruaryInCommonYear) {
  EXPECT_THROW(Date("2023", 2, 29, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidAprilThirtyOne) {
  EXPECT_THROW(Date("2024", 4, 31, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidJuneThirtyOne) {
  EXPECT_THROW(Date("2024", 6, 31, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidSeptemberThirtyOne) {
  EXPECT_THROW(Date("2024", 9, 31, Gregorian), std::invalid_argument);
}

TEST(CalendarConverter, InvalidNovemberThirtyOne) {
  EXPECT_THROW(Date("2024", 11, 31, Gregorian), std::invalid_argument);
}

// -----------------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------------

TEST(CalendarConverter, StringYearConstructor) {
  const Date date("2024", 2, 29, Gregorian);

  expect_ymd(date, Gregorian, "2024", 2, 29);
}

TEST(CalendarConverter, IntegerYearConstructor) {
  const Date date(2024LL, 2, 29, Gregorian);

  expect_ymd(date, Gregorian, "2024", 2, 29);
}

// -----------------------------------------------------------------------------
// CJDN known values
// -----------------------------------------------------------------------------

TEST(CalendarConverter, CJDN_known_Gregorian20000101) {
  const Date date("2000", 1, 1, Gregorian);

  EXPECT_EQ(date.cjdn(), "2451545");
}

TEST(CalendarConverter, CJDN_known_Julian20000101) {
  const Date date("2000", 1, 1, Julian);

  EXPECT_EQ(date.cjdn(), "2451558");
}

TEST(CalendarConverter, CJDN_known_Gregorian15821015) {
  const Date date("1582", 10, 15, Gregorian);

  EXPECT_EQ(date.cjdn(), "2299161");
}

TEST(CalendarConverter, CJDN_known_Julian15821005) {
  const Date date("1582", 10, 5, Julian);

  EXPECT_EQ(date.cjdn(), "2299161");
}

// -----------------------------------------------------------------------------
// Round trips
// -----------------------------------------------------------------------------

class RoundTrip : public ::testing::TestWithParam<Calendar> {};

TEST_P(RoundTrip, ) {
  const Calendar calendar = GetParam();

  const std::vector<DateCase> dates{
      {"4", 2, 29},   {"100", 3, 15},  {"1000", 12, 31}, {"1582", 10, 15},
      {"1900", 3, 1}, {"2000", 2, 29}, {"2024", 2, 29},  {"9999", 12, 31}};

  for (const auto &test : dates) {
    SCOPED_TRACE(std::string(test.year) + "-" + std::to_string(test.month) +
                 "-" + std::to_string(test.day));

    const Date date(test.year, test.month, test.day, calendar);

    expect_ymd(date, calendar, test.year, test.month, test.day);
  }
}

INSTANTIATE_TEST_SUITE_P(CalendarConverter, RoundTrip,
                         ::testing::Values(Julian, RevisedJulian, Gregorian));

// -----------------------------------------------------------------------------
// Calendar equivalence
// -----------------------------------------------------------------------------

TEST(CalendarConverter, GregorianJulian1582Reform) {
  const Date gregorian("1582", 10, 15, Gregorian);

  const Date julian("1582", 10, 5, Julian);

  EXPECT_EQ(gregorian.cjdn(), julian.cjdn());

  EXPECT_EQ(gregorian, julian);
}

TEST(CalendarConverter, GregorianJulian2000) {
  const Date gregorian("2000", 1, 14, Gregorian);

  const Date julian("2000", 1, 1, Julian);

  EXPECT_EQ(gregorian, julian);
}

TEST(CalendarConverter, GregorianRevisedJulianBefore2800) {
  const Date gregorian("2024", 2, 29, Gregorian);

  const Date revised("2024", 2, 29, RevisedJulian);

  EXPECT_EQ(gregorian, revised);
}

TEST(CalendarConverter, GregorianAndRevisedJulianDifferAfter2800) {
  const Date gregorian("2800", 3, 1, Gregorian);

  const Date revised("2800", 3, 1, RevisedJulian);

  EXPECT_NE(gregorian, revised);
}

// -----------------------------------------------------------------------------
// Cross-calendar round trips
// -----------------------------------------------------------------------------

class CrossCalendar : public ::testing::TestWithParam<Calendar> {};

TEST_P(CrossCalendar, ) {
  const Calendar target = GetParam();

  const std::vector<DateCase> dates{{"1000", 6, 15},
                                    {"1582", 10, 15},
                                    {"2000", 1, 1},
                                    {"2024", 2, 29},
                                    {"9999", 12, 31}};

  for (const auto &test : dates) {
    const Date original(test.year, test.month, test.day, target);

    const auto [y, m, d] = original.ymd(Julian);

    const Date reconstructed(y, m, d, Julian);

    EXPECT_EQ(original, reconstructed);
  }
}

INSTANTIATE_TEST_SUITE_P(CalendarConverter, CrossCalendar,
                         ::testing::Values(Julian, RevisedJulian, Gregorian));

// -----------------------------------------------------------------------------
// Comparisons
// -----------------------------------------------------------------------------

TEST(CalendarConverter, ComparisonEqualDates) {
  const Date a("2024", 1, 1, Gregorian);
  const Date b("2024", 1, 1, Gregorian);

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(CalendarConverter, ComparisonLess) {
  const Date a("2024", 1, 1, Gregorian);
  const Date b("2024", 1, 2, Gregorian);

  EXPECT_TRUE(a < b);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a >= b);
}

TEST(CalendarConverter, ComparisonGreater) {
  const Date a("2024", 1, 2, Gregorian);
  const Date b("2024", 1, 1, Gregorian);

  EXPECT_TRUE(a > b);
  EXPECT_TRUE(a >= b);
  EXPECT_FALSE(a < b);
  EXPECT_FALSE(a <= b);
}

TEST(CalendarConverter, ComparisonDifferentCalendarsSameDay) {
  const Date gregorian("1582", 10, 15, Gregorian);

  const Date julian("1582", 10, 5, Julian);

  EXPECT_EQ(gregorian, julian);
}

// -----------------------------------------------------------------------------
// Weekday
// -----------------------------------------------------------------------------

TEST(CalendarConverter, WeekdayGregorian20000101IsSaturday) {
  const Date date("2000", 1, 1, Gregorian);

  EXPECT_EQ(date.weekday(), WeekDay::Sat);
}

TEST(CalendarConverter, WeekdayGregorian15821015IsFriday) {
  const Date date("1582", 10, 15, Gregorian);

  EXPECT_EQ(date.weekday(), WeekDay::Fri);
}

TEST(CalendarConverter, WeekdayAllSevenWeekdays) {
  const Date sunday("2024", 9, 1, Gregorian);

  EXPECT_EQ(sunday.weekday(), WeekDay::Sun);

  const Date monday("2024", 9, 2, Gregorian);

  EXPECT_EQ(monday.weekday(), WeekDay::Mon);

  const Date tuesday("2024", 9, 3, Gregorian);

  EXPECT_EQ(tuesday.weekday(), WeekDay::Tue);

  const Date wednesday("2024", 9, 4, Gregorian);

  EXPECT_EQ(wednesday.weekday(), WeekDay::Wed);

  const Date thursday("2024", 9, 5, Gregorian);

  EXPECT_EQ(thursday.weekday(), WeekDay::Thu);

  const Date friday("2024", 9, 6, Gregorian);

  EXPECT_EQ(friday.weekday(), WeekDay::Fri);

  const Date saturday("2024", 9, 7, Gregorian);

  EXPECT_EQ(saturday.weekday(), WeekDay::Sat);
}

TEST(CalendarConverter, WeekdayConsecutiveDaysAdvanceByOne) {
  const Date d1("1", 12, 31, Gregorian);

  const Date d2("2", 1, 1, Gregorian);

  const Date d3("2", 1, 2, Gregorian);

  EXPECT_EQ(static_cast<int>(d2.weekday()),
            (static_cast<int>(d1.weekday()) + 1) % 7);

  EXPECT_EQ(static_cast<int>(d3.weekday()),
            (static_cast<int>(d2.weekday()) + 1) % 7);
}

// -----------------------------------------------------------------------------
// ymd / md / accessors
// -----------------------------------------------------------------------------

TEST(CalendarConverter, AccessorGregorian) {
  const Date date("2024", 2, 29, Gregorian);

  EXPECT_EQ(date.year(Gregorian), "2024");
  EXPECT_EQ(date.month(Gregorian), 2);
  EXPECT_EQ(date.day(Gregorian), 29);

  const auto [y, m, d] = date.ymd(Gregorian);

  EXPECT_EQ(y, "2024");
  EXPECT_EQ(m, 2);
  EXPECT_EQ(d, 29);

  const auto [mm, dd] = date.md(Gregorian);

  EXPECT_EQ(mm, 2);
  EXPECT_EQ(dd, 29);
}

TEST(CalendarConverter, AccessorConvertSameCJDNToAllCalendars) {
  const Date date("2024", 2, 29, Gregorian);

  const auto [jy, jm, jd] = date.ymd(Julian);

  const auto [ry, rm, rd] = date.ymd(RevisedJulian);

  const auto [gy, gm, gd] = date.ymd(Gregorian);

  EXPECT_EQ(gy, "2024");
  EXPECT_EQ(gm, 2);
  EXPECT_EQ(gd, 29);

  EXPECT_EQ(ry, "2024");
  EXPECT_EQ(rm, 2);
  EXPECT_EQ(rd, 29);

  EXPECT_EQ(jy, "2024");
  EXPECT_EQ(jm, 2);
  EXPECT_EQ(jd, 16);
}

// -----------------------------------------------------------------------------
// Large years
// -----------------------------------------------------------------------------

TEST(CalendarConverter, LargePositiveYear) {
  const std::string year = "100000000000000000000000000000000000000";

  const Date date(year, 1, 1, Gregorian);

  expect_ymd(date, Gregorian, year, 1, 1);
}

TEST(CalendarConverter, LargePositiveYearJulian) {
  const std::string year = "100000000000000000000000000000000000000";

  const Date date(year, 6, 15, Julian);

  expect_ymd(date, Julian, year, 6, 15);
}

TEST(CalendarConverter, LargePositiveYearRevisedJulian) {
  const std::string year = "100000000000000000000000000000000000000";

  const Date date(year, 6, 15, RevisedJulian);

  expect_ymd(date, RevisedJulian, year, 6, 15);
}

// -----------------------------------------------------------------------------
// Leading zeros
// -----------------------------------------------------------------------------

TEST(CalendarConverter, LeadingZerosRepresentSameYear) {
  const Date a("2024", 1, 1, Gregorian);

  const Date b("00002024", 1, 1, Gregorian);

  EXPECT_EQ(a, b);
  EXPECT_EQ(b.year(Gregorian), "2024");
}

// -----------------------------------------------------------------------------
// Static constructor
// -----------------------------------------------------------------------------

TEST(CalendarConverter, StaticConstructorRepresentsTodayInGregorian) {
  const auto date = Date::today_utc(Gregorian);

  const auto now = std::chrono::system_clock::now();

  const auto today = std::chrono::floor<std::chrono::days>(now);

  const std::chrono::year_month_day ymd(today);

  EXPECT_EQ(date.year(Gregorian), std::to_string(static_cast<int>(ymd.year())));

  EXPECT_EQ(date.month(Gregorian), static_cast<unsigned>(ymd.month()));

  EXPECT_EQ(date.day(Gregorian), static_cast<unsigned>(ymd.day()));
}

// -----------------------------------------------------------------------------
// Date constructor / all valid month boundaries
// -----------------------------------------------------------------------------

TEST(CalendarConverter, DateBoundaryFirstDayOfEveryMonth) {
  for (Month month = 1; month <= 12; ++month) {
    SCOPED_TRACE(month);

    const Date date("2024", month, 1, Gregorian);

    EXPECT_EQ(date.month(Gregorian), month);
    EXPECT_EQ(date.day(Gregorian), 1);
  }
}

TEST(CalendarConverter, DateBoundaryLastDayOfEveryMonth) {
  for (Month month = 1; month <= 12; ++month) {
    SCOPED_TRACE(month);

    const Day last = days_in_month(month, true);

    const Date date("2024", month, last, Gregorian);

    EXPECT_EQ(date.month(Gregorian), month);

    EXPECT_EQ(date.day(Gregorian), last);
  }
}

// -----------------------------------------------------------------------------
// Calendar differences around leap rules
// -----------------------------------------------------------------------------

TEST(CalendarConverter, CalendarDifferenceGregorianVsJulian1900) {
  const Date gregorian("1900", 3, 1, Gregorian);

  const Date julian("1900", 3, 1, Julian);

  EXPECT_NE(gregorian, julian);
}

TEST(CalendarConverter, CalendarDifferenceGregorianVsJulian2000) {
  const Date gregorian("2000", 3, 1, Gregorian);

  const Date julian("2000", 3, 1, Julian);

  EXPECT_NE(gregorian, julian);
}

TEST(CalendarConverter, CalendarDifferenceGregorianVsRevisedJulian2800) {
  const Date gregorian1("2800", 2, 29, Gregorian);
  const Date gregorian2("2800", 3, 1, Gregorian);

  const Date revised("2800", 3, 1, RevisedJulian);

  EXPECT_EQ(gregorian1, revised);
  EXPECT_NE(gregorian2, revised);
}

TEST(CalendarConverter, CalendarDifferenceGregorianVsRevisedJulian2900) {
  const Date gregorian("2900", 3, 1, Gregorian);

  const Date revised("2900", 3, 1, RevisedJulian);

  EXPECT_EQ(gregorian, revised);
}

// -----------------------------------------------------------------------------
// Day-to-day continuity
// -----------------------------------------------------------------------------

TEST(CalendarConverter, ContinuityGregorianAcrossYearZero) {
  const Date a("1", 12, 31, Gregorian);

  const Date b("2", 1, 1, Gregorian);

  const Date c("2", 1, 2, Gregorian);

  EXPECT_LT(a, b);
  EXPECT_LT(b, c);

  EXPECT_NE(a.cjdn(), b.cjdn());

  EXPECT_NE(b.cjdn(), c.cjdn());

  EXPECT_EQ(a.weekday() == WeekDay::Sat || a.weekday() == WeekDay::Sun ||
                a.weekday() == WeekDay::Mon || a.weekday() == WeekDay::Tue ||
                a.weekday() == WeekDay::Wed || a.weekday() == WeekDay::Thu ||
                a.weekday() == WeekDay::Fri,
            true);
}

// -----------------------------------------------------------------------------
// Default calendar is Julian
// -----------------------------------------------------------------------------

TEST(CalendarConverter, ConstructorDefaultsToGregorian) {
  const Date date("2000", 1, 1);

  const Date explicitGregorian("2000", 1, 1, Gregorian);

  EXPECT_EQ(date, explicitGregorian);
}

TEST(CalendarConverter, AccessorsDefaultToGregorian) {
  const Date date("2000", 1, 1, Gregorian);

  EXPECT_EQ(date.year(), date.year(Gregorian));

  EXPECT_EQ(date.month(), date.month(Gregorian));

  EXPECT_EQ(date.day(), date.day(Gregorian));

  EXPECT_EQ(date.ymd(), date.ymd(Gregorian));

  EXPECT_EQ(date.md(), date.md(Gregorian));
}

// -----------------------------------------------------------------------------
// Compare With Python Script Results from this repo:
// https://github.com/abramov7613/Python_Calendar_Calcs
// -----------------------------------------------------------------------------

TEST(CalendarConverter, CompareWithPythonScriptResults) {
  expect_all_equals(
   //  year,                            month,   day
      {"207893457786238756453485973422", 1,       1},  // Gregorian
      {"207889188926735958943086517857", 11,      14}, // Julian
      {"207893615895513108815355762637", 12,      18}  // Revised julian
  );

  expect_all_equals(
      {"987345672893847983767829374874", 7, 22},
      {"987325398855390204548450158665", 8, 19},
      {"987346423800151330289759850106", 8, 26}
  );

  expect_all_equals(
    {"1044442540695650765744096228907733", 1, 16},
    {"1044421094236909581950138444590917", 9, 11},
    {"1044443335025852257334734514684773", 8, 17}
  );

  expect_all_equals(
    {"9884981343632437670579096617301944358513256175527657480931773304", 5, 14},
    {"9884778366602794294035552898685572655343676847885449520819228504", 2, 6},
    {"9884988861460289835989623709746393173260515614664071921012238119", 1, 8}
  );

  expect_all_equals(
    {"25962566964180523780524535438459230955357026", 6, 14},
    {"25962033851922532393999267994199714338704965", 9, 26},
    {"25962586709499432526412941705027717058362456", 2, 23}
  );

  expect_all_equals(
    {"8494066886169304933760534451721540641380348059575546612066705", 6, 26},
    {"8493892470011478048648958260049020691880525054481920830822104", 7, 12},
    {"8494073346164563916615849463779331606235209506429734377527230", 12, 20}
  );

  expect_all_equals(
    {"8262772108665523000235228", 3, 8},
    {"8262602441887110977175675", 2, 8},
    {"8262778392754094865422866", 5, 26}
  );

  expect_all_equals(
    {"6918224033886875698653772203253621760", 6, 21},
    {"6918081975898500198126079107315156593", 9, 15},
    {"6918229295405893433365362878066229568", 5, 24}
  );

  expect_all_equals(
    {"913236213336559234642259", 2, 22},
    {"913217461052917826855100", 3, 3},
    {"913236907880372880806474", 11, 8}
  );

  expect_all_equals(
    {"4083729466488254526508544971301711539974", 11, 17},
    {"4083645611673747580830382578181150929881", 8, 29},
    {"4083732572288261286295449330467448814562", 6, 21}
  );

  expect_all_equals(
    {"3754093557920913255007637210128", 11, 5},
    {"3754016471810894345084536437290", 11, 11},
    {"3754096413022822734343462300051", 10, 29}
  );

  expect_all_equals(
    {"746698449041419644998312451736976558506391", 3, 26},
    {"746683116424396207223261151686632883423054", 5, 8},
    {"746699016928217022552663748526665392838979", 2, 14}
  );

  expect_all_equals(
    {"77989314508573740988676752743708732515158577858255374809099", 4, 29},
    {"77987713085277877051490126937697568064798923678011878805509", 10, 27},
    {"77989373821810701372456305944896942248678277995286095864361", 8, 4}
  );

  expect_all_equals(
    {"720681794841448676912925476894430625261511595", 8, 7},
    {"720666996447304088644405704297369137979678710", 5, 10},
    {"720682342941792163920540600591244379323218941", 8, 23}
  );

  expect_all_equals(
    {"851932022864408838665815535488713421", 2, 20},
    {"851914529393713470927855251802153077", 12, 14},
    {"851932670784527956274859355583396012", 11, 2}
  );

  expect_all_equals(
    {"277104942359197039376321", 6, 8},
    {"277099252319312866952514", 12, 11},
    {"277105153105902695285823", 12, 9}
  );

  expect_all_equals(
    {"1702682926702178897251650162989696794860201067875614405455954", 12, 7},
    {"1702647964013745587623369841631113844207329195163748376412721", 10, 3},
    {"1702684221644140321769700020048150385549480982727453881552795", 11, 18}
  );

  expect_all_equals(
    {"39324971294934629287800097416466", 6, 18},
    {"39324163800657525907321908502761", 10, 26},
    {"39325001202766946756442891391923", 9, 15}
  );

  expect_all_equals(
    {"5769626688334943633907831794934162679302649286089007527286546224", 10, 2},
    {"5769508215507667762375308020153979226270220073577999539452310361", 5, 21},
    {"5769631076310874625626899436371348149951259418975924344290427428", 10, 24}
  );

  expect_all_equals(
    {"62389185896437443994084919527352844806511", 10, 9},
    {"62387904804324580802216457824693145569452", 2, 4},
    {"62389233345303870868673468135064871036323", 5, 7}
  );

  expect_all_equals(
    {"845070415061654923385342046229023413892581804479452991176391168", 9, 30},
    {"845053062486396983859194503271195302528846843867451359698132926", 8, 20},
    {"845071057763313382653562363751980955785187238758838725645595149", 5, 10}
  );

  expect_all_equals(
    {"3095208158163715213185119052023675537869536038666", 12, 28},
    {"3095144601528023966466162478737186345353358019446", 6, 28},
    {"3095210512163312256635137901857767391430595875317", 4, 12}
  );

  expect_all_equals(
    {"414444856496343270444443076349367376068216603598", 9, 17},
    {"414436346335018910213017112425828374684724443093", 7, 5},
    {"414445171694215379929830585128226724633990358166", 11, 30}
  );

  expect_all_equals(
    {"55710402364523258046026615227437973668047819915356379533478547623", 12, 7},
    {"55709258413756019375430187576201270629574143368746207944576422807", 10, 7},
    {"55710444733972424598556478230540349745699232408602806072624208554", 1, 30}
  );

  expect_all_equals(
    {"3102833733013145687189288797821168", 2, 19},
    {"3102770019794808661610496409960843", 4, 9},
    {"3102836092812223478142086628552066", 7, 8}
  );

  expect_all_equals(
    {"61576692598189405137188877709744602361983051418041829712380", 10, 6},
    {"61575428189717163054947867671187934094994098993988071153249", 2, 5},
    {"61576739429130058059606525350638704407355043507805257667215", 9, 10}
  );

  expect_all_equals(
    {"45318862978277721179328730183218533067038607580914014656256722072115", 4, 28},
    {"45317932406142643608051947252413949524265157096158759748358236307801", 9, 24},
    {"45318897444646294148957088935043945136451498751010861280121749433502", 8, 23}
  );

  expect_all_equals(
    {"199060709774339454403848168078093883598456374", 3, 7},
    {"199056622285432383778501066472999877563885563", 12, 29},
    {"199060861166041288756891204851382901916500714", 12, 18}
  );

  expect_all_equals(
    {"48001797399193590279702305848610483625162120", 7, 23},
    {"48000811736002641746020997793048910514615402", 8, 28},
    {"48001833906015090162269034550952497608640075", 3, 10}
  );

  expect_all_equals(
    {"1271195811078401060515394415457917870169361981901827076", 3, 26},
    {"1271169708495011360288279109617764730172027908760514923", 9, 25},
    {"1271196777861335884894543500231504544703362378413412920", 3, 13}
  );

  expect_all_equals(
    {"952356391145080558266875069909197309413647750008620768384352", 11, 19},
    {"952336835572367106920709425657248455259450344510673979456870", 8, 31},
    {"952357115440974869713584914574738829194515707990652486012204", 5, 27}
  );

  expect_all_equals(
    {"8328439559505851869220637872824705961811808135881107862740600", 5, 11},
    {"8328268544319825055013877695455654119800271959122670879006252", 7, 4},
    {"8328445893536509990963187125252221966948400756157557495182284", 12, 22}
  );

  expect_all_equals(
    {"993206442700793037983842519502781573837154583896490785371", 4, 21},
    {"993186048317986040180187820477740448958841705978956976525", 8, 12},
    {"993207198064389419071155949131307774700488685736505552200", 11, 30}
  );

  expect_all_equals(
    {"3864061723135464526872668", 8, 11},
    {"3863982378952237925958359", 2, 12},
    {"3864064661871495969830254", 7, 1}
  );

  expect_all_equals(
    {"547540532124230022098846385", 10, 19},
    {"547529288992153549203115403", 5, 2},
    {"547540948545396587536411486", 4, 25}
  );

  expect_all_equals(
    {"35735748571807591292128154424976940006986194513244587240845654", 10, 13},
    {"35735014778202420705037966988541108858320753318285383039875616", 11, 21},
    {"35735775749927265152086608273073503761435302096488061316335315", 4, 4}
  );

  expect_all_equals(
    {"326717130618330859051701118972379695434", 10, 29},
    {"326710421847681612011474184657821741026", 4, 20},
    {"326717379096609062809069936394242030612", 6, 19}
  );

  expect_all_equals(
    {"565157510006412332478098045517568400101787389024200943", 12, 8},
    {"565145905129410147419936277590555719025809939591161432", 8, 17},
    {"565157939825824417009772975167028969319475576001632225", 8, 11}
  );

  expect_all_equals(
    {"9467553240050146281503001004974821024067807", 4, 24},
    {"9467358834439467633735413674358702444580660", 2, 18},
    {"9467560440411276528512809946834564767048891", 6, 13}
  );

  expect_all_equals(
    {"45966554275849155848758841372397801154494329127682096467534", 5, 25},
    {"45965610404098111718248599917749497298207789203059351455286", 8, 10},
    {"45966589234806587331941880298499636464548200907086576661877", 8, 14}
  );

  expect_all_equals(
    {"61606082676265671598927820556673089912730606177264486526949803696023", 4, 3},
    {"61604817664301066554336466802657552477619461811634563231538572693893", 12, 23},
    {"61606129529558369493225971956825010915755929349866973510083163406933", 4, 14}
  );

  expect_all_equals(
    {"7737964814360694458167479", 11, 7},
    {"7737805923912760973681685", 11, 11},
    {"7737970699317415995088532", 5, 14}
  );

  expect_all_equals(
    {"79186608299498349405540949149770650268181203810685", 12, 21},
    {"79184982291114376133479233729870244300003212410197", 1, 19},
    {"79186668523313518865647640554674815670327015860226", 10, 9}
  );

  expect_all_equals(
    {"13946923066270321315237691", 7, 6},
    {"13946636681813108372294873", 6, 30},
    {"13946933673327940817608504", 3, 18}
  );

  expect_all_equals(
    {"1861108878274414800115561996077", 4, 19},
    {"1861070662486359884000569890081", 6, 29},
    {"1861110293704113108707425018152", 6, 12}
  );

  expect_all_equals(
    {"300593760369546586648203654763872337743", 9, 27},
    {"300587588013070825938005539699092792110", 7, 11},
    {"300593988980210110965845712852914285456", 1, 6}
  );

  expect_all_equals(
    {"58621057581481393960488996449644655097150572824659708133110596", 8, 16},
    {"58619853863666579147471327271072793810598269938154068303374803", 10, 22},
    {"58621102164572053341321473290733711404270275085087205760034078", 12, 29}
  );

  expect_all_equals(
    {"6039292957299621662660986854182424460222112351117", 7, 26},
    {"6039168947177295181723355211741886833436481506921", 6, 13},
    {"6039297550364921782841249770258555203309241913624", 4, 18}
  );

  expect_all_equals(
    {"5332801483367490607198869834075945484698439868725105197358036976", 6, 28},
    {"5332691980256949180287017701225143103887665773450593388216407448", 1, 7},
    {"5332805539124616295091384998190653285872610965561807724376026072", 1, 13}
  );

  expect_all_equals(
    {"55949831523620098343226101687924288960270263144225683", 7, 24},
    {"55948682656443021955169772609860881890681756567980422", 4, 5},
    {"55949874075162395707607078106999064242038344197182278", 9, 30}
  );

  expect_all_equals(
    {"1852237975954886111026135", 9, 9},
    {"1852199942320882930613178", 4, 12},
    {"1852239384637994394483469", 3, 13}
  );

  expect_all_equals(
    {"70918285430563154665357273626850", 4, 12},
    {"70916829202936243717622872040122", 11, 1},
    {"70918339366068230872892809676955", 4, 2}
  );

  expect_all_equals(
    {"6602416169813682001325607043078105933721983", 9, 14},
    {"6602280596586375765555559289340055048589874", 6, 5},
    {"6602421191151249318662961040734944833710677", 6, 11}
  );

  expect_all_equals(
    {"215649945412952073141080797421789995337837181440274730010923", 7, 12},
    {"215645517282656119299743198226771067411854884988910453322422", 4, 13},
    {"215650109421270307122847190713910765869109683887038704203643", 6, 8}
  );

  expect_all_equals(
    {"81483436509348991243492861180880183342101101681795284771799", 8, 10},
    {"81481763338168101120469380834654703256200853199214549755685", 3, 15},
    {"81483498479971614091185445320523571352632424959727799720239", 4, 13}
  );

  expect_all_equals(
    {"23526151074712917783332980694193356499861", 2, 14},
    {"23525667991528632097136197676109286821082", 11, 24},
    {"23526168967063712927588682643113627213175", 1, 15}
  );

  expect_all_equals(
    {"3384900355401081436810473113465435471401037526452092", 4, 16},
    {"3384830850260313447458587888144830431658298285435122", 1, 11},
    {"3384902929720372897181852845084264882503769504194052", 4, 22}
  );

  expect_all_equals(
    {"665477465736360652057534599000207088167044958785063832327", 7, 30},
    {"665463800901335264775151487406798459684741734042583646211", 12, 5},
    {"665477971852139021633167860286932336105314362228865803214", 5, 7}
  );

  expect_all_equals(
    {"30788760682474393799329093361691", 8, 4},
    {"30788128469729373791242864838213", 12, 11},
    {"30788784098259871377674209334267", 11, 26}
  );

  expect_all_equals(
    {"481397442756353299006867341479235", 6, 4},
    {"481387557798596495037688555702203", 1, 1},
    {"481397808873696094619899641860825", 1, 8}
  );

  expect_all_equals(
    {"165202889325947311079696806661", 12, 2},
    {"165199497069492979512734184550", 11, 27},
    {"165203014967750715477471707714", 7, 1}
  );

  expect_all_equals(
    {"164971641653820648450664588815409", 4, 4},
    {"164968254145778475542072172704755", 12, 20},
    {"164971767119753190797485002258382", 4, 9}
  );
}

} // namespace
