#pragma once

#include <chrono>
#include <string>

namespace sos {

std::string formatDateTime(std::chrono::system_clock::time_point timePoint);

}  // namespace sos
