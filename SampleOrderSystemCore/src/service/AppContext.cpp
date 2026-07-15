#include "service/AppContext.h"

namespace sos {

AppContext::AppContext(std::filesystem::path filePath)
    : repository_(std::move(filePath)), orderBook_(sampleCatalog_, productionQueue_),
      productionLine_(orderBook_, sampleCatalog_, productionQueue_) {
    SystemState state = repository_.load();
    for (const auto& sample : state.samples) {
        sampleCatalog_.registerSample(sample);
    }
    orderBook_.restoreOrders(state.orders);
    productionQueue_.restore(state.productionJobs);

    productionLine_.processCompletedJobs();
}

SampleCatalog& AppContext::sampleCatalog() {
    return sampleCatalog_;
}

OrderBook& AppContext::orderBook() {
    return orderBook_;
}

ProductionQueue& AppContext::productionQueue() {
    return productionQueue_;
}

void AppContext::save() {
    SystemState state;
    state.samples = sampleCatalog_.list();
    state.orders = orderBook_.list();
    state.productionJobs = productionQueue_.list();
    repository_.save(state);
}

}  // namespace sos
