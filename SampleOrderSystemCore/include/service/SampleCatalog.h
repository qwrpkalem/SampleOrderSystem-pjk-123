#pragma once

#include <string>
#include <vector>

#include "domain/Sample.h"

namespace sos {

class SampleCatalog {
public:
    void registerSample(Sample sample);
    std::vector<Sample> list() const;
    std::vector<Sample> search(const std::string& nameQuery) const;
    bool exists(const std::string& id) const;
    void decreaseStock(const std::string& id, int amount);

private:
    std::vector<Sample> samples_;
};

}  // namespace sos
