#pragma once

#include <chrono>
#include <deque>
#include <functional>
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
    using NowProvider = std::function<std::chrono::system_clock::time_point()>;

    explicit ProductionQueue(NowProvider nowProvider = &std::chrono::system_clock::now);

    void enqueue(std::string orderId, std::string sampleId, int productionQuantity, double durationMinutes);
    ProductionJob dequeue();
    bool empty() const;
    std::vector<ProductionJob> list() const;
    const ProductionJob& front() const;
    void restore(std::vector<ProductionJob> jobs);

private:
    NowProvider nowProvider_;
    std::deque<ProductionJob> jobs_;
};

}  // namespace sos
