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
    // Stock is only decreased at release time, so any order that has not been released yet
    // (Reserved, Producing, or Confirmed) still has a claim on the sample's current stock.
    int total = 0;
    for (const auto& order : orders) {
        if (order.sampleId() != sampleId) {
            continue;
        }
        if (order.status() == OrderStatus::Reserved || order.status() == OrderStatus::Producing ||
            order.status() == OrderStatus::Confirmed) {
            total += order.quantity();
        }
    }
    return total;
}

}  // namespace sos
