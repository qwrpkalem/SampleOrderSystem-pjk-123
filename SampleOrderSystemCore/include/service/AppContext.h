#pragma once

#include <chrono>
#include <filesystem>
#include <functional>

#include "persistence/Repository.h"
#include "service/OrderBook.h"
#include "service/ProductionLine.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

class AppContext {
public:
    using NowProvider = std::function<std::chrono::system_clock::time_point()>;

    explicit AppContext(std::filesystem::path filePath, NowProvider nowProvider = &std::chrono::system_clock::now);

    SampleCatalog& sampleCatalog();
    OrderBook& orderBook();
    ProductionQueue& productionQueue();

    void processCompletedProductionJobs();
    void save();

private:
    Repository repository_;
    SampleCatalog sampleCatalog_;
    ProductionQueue productionQueue_;
    OrderBook orderBook_;
    ProductionLine productionLine_;
};

}  // namespace sos
