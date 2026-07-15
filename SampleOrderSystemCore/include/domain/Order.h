#pragma once

#include <string>

namespace sos {

enum class OrderStatus {
    Reserved,
    Rejected,
    Producing,
    Confirmed,
    Release,
};

class Order {
public:
    Order(std::string id, std::string sampleId, std::string customerName, int quantity,
          OrderStatus status = OrderStatus::Reserved);

    const std::string& id() const;
    const std::string& sampleId() const;
    const std::string& customerName() const;
    int quantity() const;
    OrderStatus status() const;

private:
    std::string id_;
    std::string sampleId_;
    std::string customerName_;
    int quantity_;
    OrderStatus status_;
};

}  // namespace sos
