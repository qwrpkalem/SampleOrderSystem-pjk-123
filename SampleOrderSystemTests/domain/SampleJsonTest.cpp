#include <gtest/gtest.h>

#include "domain/SampleJson.h"

TEST(SampleJsonTest, RoundTripPreservesAllFields) {
    sos::Sample original("S-001", "Wafer-A", 12.5, 0.9, 10);

    auto json = sos::toJson(original);
    sos::Sample restored = sos::sampleFromJson(json);

    EXPECT_EQ(restored.id(), original.id());
    EXPECT_EQ(restored.name(), original.name());
    EXPECT_DOUBLE_EQ(restored.averageProductionTime(), original.averageProductionTime());
    EXPECT_DOUBLE_EQ(restored.yield(), original.yield());
    EXPECT_EQ(restored.stock(), original.stock());
}
