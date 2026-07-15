#pragma once

#include "service/OrderBook.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

struct Summary {
    int sampleTypeCount = 0;
    int totalStock = 0;
    int totalOrderCount = 0;
    int pendingProductionJobCount = 0;
};

Summary buildSummary(const SampleCatalog& sampleCatalog, const OrderBook& orderBook,
                      const ProductionQueue& productionQueue);

}  // namespace sos
