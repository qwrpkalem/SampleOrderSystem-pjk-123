#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <vector>

namespace sos {

struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int productionQuantity;
    std::chrono::system_clock::time_point completionTime;
};

class ProductionQueue {
public:
    void enqueue(ProductionJob job);
    ProductionJob dequeue();
    bool empty() const;
    std::vector<ProductionJob> list() const;

private:
    std::deque<ProductionJob> jobs_;
};

}  // namespace sos
