#include "persistence/Repository.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "domain/OrderJson.h"
#include "domain/SampleJson.h"

namespace sos {

Repository::Repository(std::filesystem::path filePath) : filePath_(std::move(filePath)) {}

SystemState Repository::load() const {
    if (!std::filesystem::exists(filePath_)) {
        return SystemState{};
    }

    std::ifstream input(filePath_);
    nlohmann::json json;
    input >> json;

    SystemState state;
    for (const auto& sampleJson : json.at("samples")) {
        state.samples.push_back(sampleFromJson(sampleJson));
    }
    for (const auto& orderJson : json.at("orders")) {
        state.orders.push_back(orderFromJson(orderJson));
    }
    return state;
}

void Repository::save(const SystemState& state) const {
    nlohmann::json json;
    json["samples"] = nlohmann::json::array();
    for (const auto& sample : state.samples) {
        json["samples"].push_back(toJson(sample));
    }
    json["orders"] = nlohmann::json::array();
    for (const auto& order : state.orders) {
        json["orders"].push_back(toJson(order));
    }

    std::ofstream output(filePath_);
    output << json.dump(2);
}

}  // namespace sos
