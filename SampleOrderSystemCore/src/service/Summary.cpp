#include "service/Summary.h"

namespace sos {

Summary buildSummary(const SampleCatalog& sampleCatalog, const OrderBook& orderBook,
                      const ProductionQueue& productionQueue) {
    Summary summary;

    auto samples = sampleCatalog.list();
    summary.sampleTypeCount = static_cast<int>(samples.size());
    for (const auto& sample : samples) {
        summary.totalStock += sample.stock();
    }

    summary.totalOrderCount = static_cast<int>(orderBook.list().size());
    summary.pendingProductionJobCount = static_cast<int>(productionQueue.list().size());

    return summary;
}

}  // namespace sos
