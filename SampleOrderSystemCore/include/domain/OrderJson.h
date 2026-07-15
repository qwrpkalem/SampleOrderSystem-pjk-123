#pragma once

#include <nlohmann/json.hpp>

#include "domain/Order.h"

namespace sos {

nlohmann::json toJson(const Order& order);
Order orderFromJson(const nlohmann::json& json);

}  // namespace sos
