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
        sampleCatalog_.increaseStock(job.sampleId, job.productionQuantity);
        orderBook_.completeProduction(job.orderId);
    }
}

}  // namespace sos
