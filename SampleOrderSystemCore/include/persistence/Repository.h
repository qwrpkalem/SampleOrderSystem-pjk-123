#pragma once

#include <filesystem>
#include <vector>

#include "domain/Order.h"
#include "domain/Sample.h"
#include "service/ProductionQueue.h"

namespace sos {

struct SystemState {
    std::vector<Sample> samples;
    std::vector<Order> orders;
    std::vector<ProductionJob> productionJobs;
};

class Repository {
public:
    explicit Repository(std::filesystem::path filePath);

    SystemState load() const;
    void save(const SystemState& state) const;

private:
    std::filesystem::path filePath_;
};

}  // namespace sos
