#pragma once

#include <string>
#include <vector>

#include "domain/Order.h"
#include "service/SampleCatalog.h"

namespace sos {

class OrderBook {
public:
    explicit OrderBook(SampleCatalog& sampleCatalog);

    Order placeOrder(std::string id, std::string sampleId, std::string customerName, int quantity);
    std::vector<Order> list() const;
    void reject(const std::string& orderId);
    void approve(const std::string& orderId);

private:
    SampleCatalog& sampleCatalog_;
    std::vector<Order> orders_;
};

}  // namespace sos
