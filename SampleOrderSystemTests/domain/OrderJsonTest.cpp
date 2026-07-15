#include <gtest/gtest.h>

#include "domain/OrderJson.h"

TEST(OrderJsonTest, RoundTripPreservesAllFieldsAndStatus) {
    sos::Order original("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);

    auto json = sos::toJson(original);
    sos::Order restored = sos::orderFromJson(json);

    EXPECT_EQ(restored.id(), original.id());
    EXPECT_EQ(restored.sampleId(), original.sampleId());
    EXPECT_EQ(restored.customerName(), original.customerName());
    EXPECT_EQ(restored.quantity(), original.quantity());
    EXPECT_EQ(restored.status(), original.status());
}
