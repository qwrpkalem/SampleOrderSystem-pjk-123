#include "domain/Order.h"

namespace sos {

Order::Order(std::string id, std::string sampleId, std::string customerName, int quantity, OrderStatus status)
    : id_(std::move(id)),
      sampleId_(std::move(sampleId)),
      customerName_(std::move(customerName)),
      quantity_(quantity),
      status_(status) {}

const std::string& Order::id() const {
    return id_;
}

const std::string& Order::sampleId() const {
    return sampleId_;
}

const std::string& Order::customerName() const {
    return customerName_;
}

int Order::quantity() const {
    return quantity_;
}

OrderStatus Order::status() const {
    return status_;
}

}  // namespace sos
