#include "service/OrderBook.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "service/ProductionCalculator.h"

namespace sos {

namespace {

std::vector<Order>::iterator findOrder(std::vector<Order>& orders, const std::string& orderId) {
    auto it = std::find_if(orders.begin(), orders.end(), [&orderId](const Order& order) { return order.id() == orderId; });
    if (it == orders.end()) {
        throw std::invalid_argument("Unknown order id: " + orderId);
    }
    return it;
}

std::vector<Order>::const_iterator findOrder(const std::vector<Order>& orders, const std::string& orderId) {
    auto it = std::find_if(orders.begin(), orders.end(), [&orderId](const Order& order) { return order.id() == orderId; });
    if (it == orders.end()) {
        throw std::invalid_argument("Unknown order id: " + orderId);
    }
    return it;
}

const std::string kOrderIdPrefix = "O-";

// Stock is only ever decreased at release time. Until then, any order that has already been
// approved (Producing or Confirmed) has an implicit claim on the sample's stock that has not
// been physically deducted yet. To avoid double-allocating the same physical stock to two
// different orders, that claim must be subtracted from the raw stock number before deciding
// whether a *new* approval has enough stock available.
int committedQuantityForSample(const std::vector<Order>& orders, const std::string& sampleId,
                                const std::string& excludingOrderId) {
    int committed = 0;
    for (const auto& order : orders) {
        if (order.id() == excludingOrderId) {
            continue;
        }
        if (order.sampleId() == sampleId &&
            (order.status() == OrderStatus::Producing || order.status() == OrderStatus::Confirmed)) {
            committed += order.quantity();
        }
    }
    return committed;
}

struct StockAssessment {
    int committedQuantity;
    int availableStock;      // max(0, sample stock - committedQuantity)
    bool sufficient;
    int shortage;             // 0 when sufficient
    int productionQuantity;   // 0 when sufficient; otherwise ceil(shortage / yield)
};

// Shared by previewApproval() and approve() so the two can never diverge on how
// sufficiency/shortage/production quantity is computed.
StockAssessment assessStock(const std::vector<Order>& orders, const Order& order, const Sample& sample) {
    StockAssessment assessment;
    assessment.committedQuantity = committedQuantityForSample(orders, order.sampleId(), order.id());
    assessment.availableStock = std::max(0, sample.stock() - assessment.committedQuantity);
    assessment.sufficient = assessment.availableStock >= order.quantity();
    if (assessment.sufficient) {
        assessment.shortage = 0;
        assessment.productionQuantity = 0;
    } else {
        assessment.shortage = order.quantity() - assessment.availableStock;
        assessment.productionQuantity = calculateProductionQuantity(assessment.shortage, sample.yield());
    }
    return assessment;
}

}  // namespace

OrderBook::OrderBook(SampleCatalog& sampleCatalog, ProductionQueue& productionQueue)
    : sampleCatalog_(sampleCatalog), productionQueue_(productionQueue) {}

std::string OrderBook::generateOrderId() {
    std::ostringstream oss;
    oss << kOrderIdPrefix << std::setw(4) << std::setfill('0') << nextOrderNumber_;
    ++nextOrderNumber_;
    return oss.str();
}

Order OrderBook::placeOrder(std::string sampleId, std::string customerName, int quantity) {
    if (!sampleCatalog_.exists(sampleId)) {
        throw std::invalid_argument("Unknown sample id: " + sampleId);
    }

    Order order(generateOrderId(), std::move(sampleId), std::move(customerName), quantity, OrderStatus::Reserved);
    orders_.push_back(order);
    return order;
}

std::vector<Order> OrderBook::list() const {
    return orders_;
}

void OrderBook::reject(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);
    *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Rejected);
}

ApprovalPreview OrderBook::previewApproval(const std::string& orderId) const {
    auto orderIt = findOrder(orders_, orderId);
    const Sample& sample = sampleCatalog_.get(orderIt->sampleId());
    StockAssessment assessment = assessStock(orders_, *orderIt, sample);

    ApprovalPreview preview;
    preview.currentStock = sample.stock();
    preview.committedQuantity = assessment.committedQuantity;
    preview.availableStock = assessment.availableStock;
    preview.orderQuantity = orderIt->quantity();
    preview.sufficient = assessment.sufficient;
    preview.shortage = assessment.shortage;
    preview.productionQuantity = assessment.productionQuantity;
    return preview;
}

void OrderBook::approve(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);
    const Sample& sample = sampleCatalog_.get(it->sampleId());
    // Stock is only ever decreased when an order is released, never at approval time. See
    // assessStock() for how sufficiency/shortage/production quantity are derived.
    StockAssessment assessment = assessStock(orders_, *it, sample);

    if (assessment.sufficient) {
        *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Confirmed);
    } else {
        double durationMinutes =
            calculateProductionDuration(sample.averageProductionTime(), assessment.productionQuantity);
        productionQueue_.enqueue(it->id(), it->sampleId(), assessment.shortage, assessment.productionQuantity,
                                  durationMinutes);
        *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Producing);
    }
}

void OrderBook::completeProduction(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);
    *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Confirmed);
}

void OrderBook::restoreOrders(std::vector<Order> orders) {
    for (const auto& order : orders) {
        const std::string& id = order.id();
        if (id.size() > kOrderIdPrefix.size() && id.compare(0, kOrderIdPrefix.size(), kOrderIdPrefix) == 0) {
            std::string numberPart = id.substr(kOrderIdPrefix.size());
            bool isNumeric = std::all_of(numberPart.begin(), numberPart.end(), ::isdigit);
            if (isNumeric && !numberPart.empty()) {
                int number = std::stoi(numberPart);
                nextOrderNumber_ = std::max(nextOrderNumber_, number + 1);
            }
        }
    }
    orders_ = std::move(orders);
}

void OrderBook::release(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);
    if (it->status() != OrderStatus::Confirmed) {
        throw std::invalid_argument("Order is not in Confirmed status: " + orderId);
    }
    // Stock only ever decreases here, at the moment of actual release.
    sampleCatalog_.decreaseStock(it->sampleId(), it->quantity());
    *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Release);
}

}  // namespace sos
