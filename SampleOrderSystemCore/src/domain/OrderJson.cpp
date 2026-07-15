#include "domain/OrderJson.h"

#include <stdexcept>

namespace sos {

namespace {

std::string statusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::Reserved:
            return "RESERVED";
        case OrderStatus::Rejected:
            return "REJECTED";
        case OrderStatus::Producing:
            return "PRODUCING";
        case OrderStatus::Confirmed:
            return "CONFIRMED";
        case OrderStatus::Release:
            return "RELEASE";
    }
    throw std::invalid_argument("Unknown OrderStatus value");
}

OrderStatus statusFromString(const std::string& status) {
    if (status == "RESERVED") return OrderStatus::Reserved;
    if (status == "REJECTED") return OrderStatus::Rejected;
    if (status == "PRODUCING") return OrderStatus::Producing;
    if (status == "CONFIRMED") return OrderStatus::Confirmed;
    if (status == "RELEASE") return OrderStatus::Release;
    throw std::invalid_argument("Unknown OrderStatus string: " + status);
}

}  // namespace

nlohmann::json toJson(const Order& order) {
    return nlohmann::json{
        {"id", order.id()},
        {"sampleId", order.sampleId()},
        {"customerName", order.customerName()},
        {"quantity", order.quantity()},
        {"status", statusToString(order.status())},
    };
}

Order orderFromJson(const nlohmann::json& json) {
    return Order(json.at("id").get<std::string>(), json.at("sampleId").get<std::string>(),
                 json.at("customerName").get<std::string>(), json.at("quantity").get<int>(),
                 statusFromString(json.at("status").get<std::string>()));
}

}  // namespace sos
