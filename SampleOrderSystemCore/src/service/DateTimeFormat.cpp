#include "service/DateTimeFormat.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace sos {

std::string formatDateTime(std::chrono::system_clock::time_point timePoint) {
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

}  // namespace sos
