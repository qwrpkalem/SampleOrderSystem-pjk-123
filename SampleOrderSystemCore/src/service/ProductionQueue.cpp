#include "service/ProductionQueue.h"

namespace sos {

ProductionQueue::ProductionQueue(NowProvider nowProvider) : nowProvider_(std::move(nowProvider)) {}

void ProductionQueue::enqueue(std::string orderId, std::string sampleId, int productionQuantity,
                               double durationMinutes) {
    auto startTime = jobs_.empty() ? nowProvider_() : jobs_.back().completionTime;
    auto completionTime =
        startTime + std::chrono::duration_cast<std::chrono::system_clock::duration>(
                        std::chrono::duration<double, std::ratio<60>>(durationMinutes));

    jobs_.push_back(ProductionJob{std::move(orderId), std::move(sampleId), productionQuantity, completionTime});
}

ProductionJob ProductionQueue::dequeue() {
    ProductionJob job = jobs_.front();
    jobs_.pop_front();
    return job;
}

bool ProductionQueue::empty() const {
    return jobs_.empty();
}

std::vector<ProductionJob> ProductionQueue::list() const {
    return std::vector<ProductionJob>(jobs_.begin(), jobs_.end());
}

const ProductionJob& ProductionQueue::front() const {
    return jobs_.front();
}

}  // namespace sos
