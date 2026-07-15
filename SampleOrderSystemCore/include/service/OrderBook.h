#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "domain/Order.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

class OrderBook {
public:
    using NowProvider = std::function<std::chrono::system_clock::time_point()>;

    OrderBook(SampleCatalog& sampleCatalog, ProductionQueue& productionQueue,
              NowProvider nowProvider = &std::chrono::system_clock::now);

    Order placeOrder(std::string id, std::string sampleId, std::string customerName, int quantity);
    std::vector<Order> list() const;
    void reject(const std::string& orderId);
    void approve(const std::string& orderId);

private:
    SampleCatalog& sampleCatalog_;
    ProductionQueue& productionQueue_;
    NowProvider nowProvider_;
    std::vector<Order> orders_;
};

}  // namespace sos
