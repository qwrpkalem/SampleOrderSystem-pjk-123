#include "domain/SampleJson.h"

namespace sos {

nlohmann::json toJson(const Sample& sample) {
    return nlohmann::json{
        {"id", sample.id()},
        {"name", sample.name()},
        {"averageProductionTime", sample.averageProductionTime()},
        {"yield", sample.yield()},
        {"stock", sample.stock()},
    };
}

Sample sampleFromJson(const nlohmann::json& json) {
    return Sample(json.at("id").get<std::string>(), json.at("name").get<std::string>(),
                  json.at("averageProductionTime").get<double>(), json.at("yield").get<double>(),
                  json.at("stock").get<int>());
}

}  // namespace sos
