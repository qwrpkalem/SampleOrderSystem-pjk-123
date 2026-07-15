#pragma once

#include <nlohmann/json.hpp>

#include "service/ProductionQueue.h"

namespace sos {

nlohmann::json toJson(const ProductionJob& job);
ProductionJob productionJobFromJson(const nlohmann::json& json);

}  // namespace sos
