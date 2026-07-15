#pragma once

#include <string>
#include <vector>

#include "domain/Order.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

namespace sos {

struct ApprovalPreview {
    int currentStock;
    int committedQuantity;  // already claimed by other Producing/Confirmed orders on this sample
    int availableStock;     // max(0, currentStock - committedQuantity)
    int orderQuantity;
    int shortage;            // 0 if stock is sufficient
    int productionQuantity;  // 0 if stock is sufficient; otherwise ceil(shortage / yield)
    bool sufficient;
};

class OrderBook {
public:
    OrderBook(SampleCatalog& sampleCatalog, ProductionQueue& productionQueue);

    Order placeOrder(std::string sampleId, std::string customerName, int quantity);
    std::vector<Order> list() const;
    void reject(const std::string& orderId);
    ApprovalPreview previewApproval(const std::string& orderId) const;
    void approve(const std::string& orderId);
    void completeProduction(const std::string& orderId);
    void restoreOrders(std::vector<Order> orders);
    void release(const std::string& orderId);

private:
    SampleCatalog& sampleCatalog_;
    ProductionQueue& productionQueue_;
    std::vector<Order> orders_;
    int nextOrderNumber_ = 1;

    std::string generateOrderId();
};

}  // namespace sos
