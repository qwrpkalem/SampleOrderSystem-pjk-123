#include "domain/Sample.h"

namespace sos {

Sample::Sample(std::string id, std::string name, double averageProductionTime, double yield, int stock)
    : id_(std::move(id)),
      name_(std::move(name)),
      averageProductionTime_(averageProductionTime),
      yield_(yield),
      stock_(stock) {}

const std::string& Sample::id() const {
    return id_;
}

const std::string& Sample::name() const {
    return name_;
}

double Sample::averageProductionTime() const {
    return averageProductionTime_;
}

double Sample::yield() const {
    return yield_;
}

int Sample::stock() const {
    return stock_;
}

}  // namespace sos
