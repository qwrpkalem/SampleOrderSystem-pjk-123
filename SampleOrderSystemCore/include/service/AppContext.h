#pragma once

#include <filesystem>

#include "persistence/Repository.h"
#include "service/OrderBook.h"
#include "service/ProductionLine.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

class AppContext {
public:
    explicit AppContext(std::filesystem::path filePath);

    SampleCatalog& sampleCatalog();
    OrderBook& orderBook();
    ProductionQueue& productionQueue();

    void save();

private:
    Repository repository_;
    SampleCatalog sampleCatalog_;
    ProductionQueue productionQueue_;
    OrderBook orderBook_;
    ProductionLine productionLine_;
};

}  // namespace sos
