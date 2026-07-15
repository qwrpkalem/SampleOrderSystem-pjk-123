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
    *it = Order(it->id(), it->sampleId(), it->customerName(), it->quantity(), OrderStatus::Release);
}

}  // namespace sos
