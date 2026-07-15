#include <gtest/gtest.h>

#include <regex>

#include "service/DateTimeFormat.h"

TEST(DateTimeFormatTest, FormatDateTimeProducesYearMonthDayHourMinuteSecondPattern) {
    auto now = std::chrono::system_clock::now();

    std::string formatted = sos::formatDateTime(now);

    std::regex pattern(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");
    EXPECT_TRUE(std::regex_match(formatted, pattern)) << "formatted=" << formatted;
}
