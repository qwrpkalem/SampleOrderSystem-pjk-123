#pragma once

#include <chrono>
#include <functional>

#include "service/OrderBook.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

class ProductionLine {
public:
    using NowProvider = std::function<std::chrono::system_clock::time_point()>;

    ProductionLine(OrderBook& orderBook, SampleCatalog& sampleCatalog, ProductionQueue& productionQueue,
                   NowProvider nowProvider = &std::chrono::system_clock::now);

    void processCompletedJobs();

private:
    OrderBook& orderBook_;
    SampleCatalog& sampleCatalog_;
    ProductionQueue& productionQueue_;
    NowProvider nowProvider_;
};

}  // namespace sos
