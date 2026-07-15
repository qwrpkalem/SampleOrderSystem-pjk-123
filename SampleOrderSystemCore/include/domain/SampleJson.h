#pragma once

#include <nlohmann/json.hpp>

#include "domain/Sample.h"

namespace sos {

nlohmann::json toJson(const Sample& sample);
Sample sampleFromJson(const nlohmann::json& json);

}  // namespace sos
