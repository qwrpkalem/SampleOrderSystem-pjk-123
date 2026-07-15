#pragma once

#include <string>
#include <vector>

#include "domain/Order.h"

namespace sos {

struct OrderStatusSummary {
    int reservedCount = 0;
    int producingCount = 0;
    int confirmedCount = 0;
    int releaseCount = 0;
};

OrderStatusSummary summarizeOrdersByStatus(const std::vector<Order>& orders);

enum class StockLevel {
    Sufficient,
    Insufficient,
    Depleted,
};

StockLevel evaluateStockLevel(int stock, int demand);

int totalDemandForSample(const std::string& sampleId, const std::vector<Order>& orders);

}  // namespace sos
