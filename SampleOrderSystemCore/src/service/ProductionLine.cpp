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
        // Stock is only ever decreased at release time, so the full produced quantity is added
        // to stock here; the order's own quantity remains a claim on that stock (tracked via its
        // Confirmed status) until it is actually released.
        sampleCatalog_.increaseStock(job.sampleId, job.productionQuantity);
        orderBook_.completeProduction(job.orderId);
    }
}

}  // namespace sos
