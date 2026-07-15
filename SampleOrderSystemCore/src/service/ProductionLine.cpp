#include "service/ProductionLine.h"

namespace sos {

ProductionLine::ProductionLine(OrderBook& orderBook, SampleCatalog& sampleCatalog, ProductionQueue& productionQueue,
                                NowProvider nowProvider)
    : orderBook_(orderBook),
      sampleCatalog_(sampleCatalog),
      productionQueue_(productionQueue),
      nowProvider_(std::move(nowProvider)) {}

void ProductionLine::processCompletedJobs() {
    while (!productionQueue_.empty()) {
        ProductionJob job = productionQueue_.front();
        if (job.completionTime > nowProvider_()) {
            break;
        }

        productionQueue_.dequeue();
        // `shortage` worth of the newly produced goods goes straight to fulfilling this order
        // (its stock was already fully committed to it at approval time); only the surplus
        // caused by yield-adjusted rounding becomes real, available stock.
        int surplus = job.productionQuantity - job.shortage;
        if (surplus > 0) {
            sampleCatalog_.increaseStock(job.sampleId, surplus);
        }
        orderBook_.completeProduction(job.orderId);
    }
}

}  // namespace sos
