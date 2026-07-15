#pragma once

#include <string>
#include <vector>

#include "domain/Order.h"
#include "service/SampleCatalog.h"

namespace sos {

class OrderBook {
public:
    explicit OrderBook(const SampleCatalog& sampleCatalog);

    Order placeOrder(std::string id, std::string sampleId, std::string customerName, int quantity);
    std::vector<Order> list() const;

private:
    const SampleCatalog& sampleCatalog_;
    std::vector<Order> orders_;
};

}  // namespace sos
