#include "service/OrderBook.h"

#include <algorithm>
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

}  // namespace

OrderBook::OrderBook(SampleCatalog& sampleCatalog, ProductionQueue& productionQueue)
    : sampleCatalog_(sampleCatalog), productionQueue_(productionQueue) {}

Order OrderBook::placeOrder(std::string id, std::string sampleId, std::string customerName, int quantity) {
    if (!sampleCatalog_.exists(sampleId)) {
        throw std::invalid_argument("Unknown sample id: " + sampleId);
    }

    Order order(std::move(id), std::move(sampleId), std::move(customerName), quantity, OrderStatus::Reserved);
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

void OrderBook::approve(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);

    auto samples = sampleCatalog_.list();
    auto sampleIt = std::find_if(samples.begin(), samples.end(),
                                  [it](const Sample& sample) { return sample.id() == it->sampleId(); });

    if (sampleIt->stock() >= it->quantity()) {
        sampleCatalog_.decreaseStock(it->sampleId(), it->quantity());
        *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Confirmed);
    } else {
        int shortage = it->quantity() - sampleIt->stock();
        int productionQuantity = calculateProductionQuantity(shortage, sampleIt->yield());
        double durationMinutes = calculateProductionDuration(sampleIt->averageProductionTime(), productionQuantity);

        productionQueue_.enqueue(it->id(), it->sampleId(), productionQuantity, durationMinutes);
        *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Producing);
    }
}

void OrderBook::completeProduction(const std::string& orderId) {
    auto it = findOrder(orders_, orderId);
    *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Confirmed);
}

void OrderBook::restoreOrders(std::vector<Order> orders) {
    orders_ = std::move(orders);
}

}  // namespace sos
