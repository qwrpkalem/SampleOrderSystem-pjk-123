#include <gtest/gtest.h>

#include "domain/Sample.h"

TEST(SampleTest, ConstructorStoresAllFields) {
    sos::Sample sample("S-001", "Wafer-A", 12.5, 0.9, 10);

    EXPECT_EQ(sample.id(), "S-001");
    EXPECT_EQ(sample.name(), "Wafer-A");
    EXPECT_DOUBLE_EQ(sample.averageProductionTime(), 12.5);
    EXPECT_DOUBLE_EQ(sample.yield(), 0.9);
    EXPECT_EQ(sample.stock(), 10);
}
