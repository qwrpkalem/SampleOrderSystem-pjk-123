#include <gtest/gtest.h>

#include "service/ProductionJobJson.h"

TEST(ProductionJobJsonTest, RoundTripPreservesAllFields) {
    auto completionTime = std::chrono::system_clock::now();
    // JSON round-trip is lossy below millisecond-in-double precision issues, so truncate for comparison.
    completionTime = std::chrono::time_point_cast<std::chrono::seconds>(completionTime);

    sos::ProductionJob original{"O-001", "S-001", 12, completionTime};

    auto json = sos::toJson(original);
    sos::ProductionJob restored = sos::productionJobFromJson(json);

    EXPECT_EQ(restored.orderId, original.orderId);
    EXPECT_EQ(restored.sampleId, original.sampleId);
    EXPECT_EQ(restored.productionQuantity, original.productionQuantity);
    EXPECT_EQ(restored.completionTime, original.completionTime);
}
