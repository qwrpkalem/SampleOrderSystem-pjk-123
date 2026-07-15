#include <gtest/gtest.h>

#include "service/ProductionCalculator.h"

TEST(ProductionCalculatorTest, ProductionQuantityRoundsUpShortageDividedByYield) {
    int quantity = sos::calculateProductionQuantity(10, 0.9);

    EXPECT_EQ(quantity, 12);
}

TEST(ProductionCalculatorTest, ProductionDurationMultipliesAverageTimeByQuantity) {
    double duration = sos::calculateProductionDuration(5.0, 12);

    EXPECT_DOUBLE_EQ(duration, 60.0);
}
