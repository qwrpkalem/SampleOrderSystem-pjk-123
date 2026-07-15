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
    auto orderIt = std::find_if(orders_.begin(), orders_.end(),
                                 [&orderId](const Order& order) { return order.id() == orderId; });
    if (orderIt == orders_.end()) {
        throw std::invalid_argument("Unknown order id: " + orderId);
    }

    auto samples = sampleCatalog_.list();
    auto sampleIt = std::find_if(samples.begin(), samples.end(),
                                  [&orderIt](const Sample& sample) { return sample.id() == orderIt->sampleId(); });
    if (sampleIt == samples.end()) {
        throw std::invalid_argument("Unknown sample id: " + orderIt->sampleId());
    }

    int committed = committedQuantityForSample(orders_, orderIt->sampleId(), orderIt->id());
    // A prior order can already claim more than the raw stock (its own shortage is covered by
    // production, not by stock that doesn't exist), so the stock left over for this order can
    // never go below zero.
    int available = std::max(0, sampleIt->stock() - committed);

    ApprovalPreview preview;
    preview.currentStock = sampleIt->stock();
    preview.committedQuantity = committed;
    preview.availableStock = available;
    preview.orderQuantity = orderIt->quantity();
    preview.sufficient = available >= preview.orderQuantity;
    if (preview.sufficient) {
        preview.shortage = 0;
        preview.productionQuantity = 0;
    } else {
        preview.shortage = preview.orderQuantity - available;
        preview.productionQuantity = calculateProductionQuantity(preview.shortage, sampleIt->yield());
    }
    return preview;
}

void OrderBook::approve(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);

    auto samples = sampleCatalog_.list();
    auto sampleIt = std::find_if(samples.begin(), samples.end(),
                                  [it](const Sample& sample) { return sample.id() == it->sampleId(); });

    // Stock is only ever decreased when an order is released, never at approval time. Any
    // stock already committed to other Producing/Confirmed orders for this sample must be
    // subtracted first so it isn't double-allocated to this order too.
    int committed = committedQuantityForSample(orders_, it->sampleId(), it->id());
    int available = std::max(0, sampleIt->stock() - committed);

    if (available >= it->quantity()) {
        *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Confirmed);
    } else {
        int shortage = it->quantity() - available;
        int productionQuantity = calculateProductionQuantity(shortage, sampleIt->yield());
        double durationMinutes = calculateProductionDuration(sampleIt->averageProductionTime(), productionQuantity);

        productionQueue_.enqueue(it->id(), it->sampleId(), shortage, productionQuantity, durationMinutes);
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
