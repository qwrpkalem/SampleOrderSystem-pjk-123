#include "service/ProductionCalculator.h"

#include <cmath>

namespace sos {

int calculateProductionQuantity(int shortage, double yield) {
    return static_cast<int>(std::ceil(shortage / yield));
}

double calculateProductionDuration(double averageProductionTime, int productionQuantity) {
    return averageProductionTime * productionQuantity;
}

}  // namespace sos
