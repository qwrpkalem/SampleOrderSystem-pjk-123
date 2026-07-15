#include "service/OrderBook.h"

#include <stdexcept>

namespace sos {

OrderBook::OrderBook(const SampleCatalog& sampleCatalog) : sampleCatalog_(sampleCatalog) {}

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

}  // namespace sos
