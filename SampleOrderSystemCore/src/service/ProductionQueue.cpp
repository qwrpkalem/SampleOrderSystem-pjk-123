#include "service/ProductionQueue.h"

namespace sos {

void ProductionQueue::enqueue(ProductionJob job) {
    jobs_.push_back(std::move(job));
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

}  // namespace sos
