#include <gtest/gtest.h>

#include "domain/Order.h"

TEST(OrderTest, ConstructorDefaultsToReservedStatus) {
    sos::Order order("O-001", "S-001", "Acme Labs", 5);

    EXPECT_EQ(order.id(), "O-001");
    EXPECT_EQ(order.sampleId(), "S-001");
    EXPECT_EQ(order.customerName(), "Acme Labs");
    EXPECT_EQ(order.quantity(), 5);
    EXPECT_EQ(order.status(), sos::OrderStatus::Reserved);
}
