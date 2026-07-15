#pragma once

#include <string>

namespace sos {

class Sample {
public:
    Sample(std::string id, std::string name, double averageProductionTime, double yield, int stock);

    const std::string& id() const;
    const std::string& name() const;
    double averageProductionTime() const;
    double yield() const;
    int stock() const;

private:
    std::string id_;
    std::string name_;
    double averageProductionTime_;
    double yield_;
    int stock_;
};

}  // namespace sos
