#include "service/ProductionJobJson.h"

namespace sos {

nlohmann::json toJson(const ProductionJob& job) {
    auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(job.completionTime.time_since_epoch()).count();
    return nlohmann::json{
        {"orderId", job.orderId},
        {"sampleId", job.sampleId},
        {"shortage", job.shortage},
        {"productionQuantity", job.productionQuantity},
        {"completionTimeEpochSeconds", epochSeconds},
    };
}

ProductionJob productionJobFromJson(const nlohmann::json& json) {
    auto epochSeconds = json.at("completionTimeEpochSeconds").get<int64_t>();
    auto completionTime = std::chrono::system_clock::time_point(std::chrono::seconds(epochSeconds));

    return ProductionJob{json.at("orderId").get<std::string>(), json.at("sampleId").get<std::string>(),
                          json.at("shortage").get<int>(), json.at("productionQuantity").get<int>(), completionTime};
}

}  // namespace sos
