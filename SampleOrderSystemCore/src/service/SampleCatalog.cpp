#include "service/SampleCatalog.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace sos {

void SampleCatalog::registerSample(Sample sample) {
    bool idAlreadyExists = std::any_of(samples_.begin(), samples_.end(),
                                       [&sample](const Sample& existing) { return existing.id() == sample.id(); });
    if (idAlreadyExists) {
        throw std::invalid_argument("Sample id already registered: " + sample.id());
    }
    samples_.push_back(std::move(sample));
}

std::vector<Sample> SampleCatalog::list() const {
    return samples_;
}

bool SampleCatalog::exists(const std::string& id) const {
    return std::any_of(samples_.begin(), samples_.end(), [&id](const Sample& sample) { return sample.id() == id; });
}

std::vector<Sample> SampleCatalog::search(const std::string& nameQuery) const {
    std::vector<Sample> results;
    std::copy_if(samples_.begin(), samples_.end(), std::back_inserter(results),
                 [&nameQuery](const Sample& sample) { return sample.name().find(nameQuery) != std::string::npos; });
    return results;
}

}  // namespace sos
