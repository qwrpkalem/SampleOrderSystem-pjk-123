#pragma once

#include <string>
#include <vector>

#include "domain/Order.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

class OrderBook {
public:
    OrderBook(SampleCatalog& sampleCatalog, ProductionQueue& productionQueue);

    Order placeOrder(std::string id, std::string sampleId, std::string customerName, int quantity);
    std::vector<Order> list() const;
    void reject(const std::string& orderId);
    void approve(const std::string& orderId);
    void completeProduction(const std::string& orderId);

private:
    SampleCatalog& sampleCatalog_;
    ProductionQueue& productionQueue_;
    std::vector<Order> orders_;
};

}  // namespace sos
