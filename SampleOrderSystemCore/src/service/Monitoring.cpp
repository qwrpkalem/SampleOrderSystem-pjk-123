#include "service/Monitoring.h"

namespace sos {

OrderStatusSummary summarizeOrdersByStatus(const std::vector<Order>& orders) {
    OrderStatusSummary summary;
    for (const auto& order : orders) {
        switch (order.status()) {
            case OrderStatus::Reserved:
                ++summary.reservedCount;
                break;
            case OrderStatus::Producing:
                ++summary.producingCount;
                break;
            case OrderStatus::Confirmed:
                ++summary.confirmedCount;
                break;
            case OrderStatus::Release:
                ++summary.releaseCount;
                break;
            case OrderStatus::Rejected:
                break;
        }
    }
    return summary;
}

StockLevel evaluateStockLevel(int stock, int demand) {
    if (stock == 0) {
        return StockLevel::Depleted;
    }
    if (stock >= demand) {
        return StockLevel::Sufficient;
    }
    return StockLevel::Insufficient;
}

int totalDemandForSample(const std::string& sampleId, const std::vector<Order>& orders) {
    int total = 0;
    for (const auto& order : orders) {
        if (order.sampleId() == sampleId && order.status() == OrderStatus::Reserved) {
            total += order.quantity();
        }
    }
    return total;
}

}  // namespace sos
