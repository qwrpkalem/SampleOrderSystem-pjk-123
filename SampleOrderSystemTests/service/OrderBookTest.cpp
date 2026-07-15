#include <gtest/gtest.h>

#include "service/OrderBook.h"
#include "service/SampleCatalog.h"

TEST(OrderBookTest, PlaceOrderCreatesReservedOrderInList) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::OrderBook orderBook(catalog);

    sos::Order placed = orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    EXPECT_EQ(placed.status(), sos::OrderStatus::Reserved);

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].id(), "O-001");
    EXPECT_EQ(orders[0].sampleId(), "S-001");
    EXPECT_EQ(orders[0].customerName(), "Acme Labs");
    EXPECT_EQ(orders[0].quantity(), 5);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Reserved);
}

TEST(OrderBookTest, PlaceOrderWithUnknownSampleIdThrows) {
    sos::SampleCatalog catalog;
    sos::OrderBook orderBook(catalog);

    EXPECT_THROW(orderBook.placeOrder("O-001", "UNKNOWN", "Acme Labs", 5), std::invalid_argument);
}

TEST(OrderBookTest, RejectTransitionsReservedOrderToRejected) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::OrderBook orderBook(catalog);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.reject("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Rejected);
}

TEST(OrderBookTest, ApproveWithSufficientStockConfirmsAndDecreasesStock) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::OrderBook orderBook(catalog);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.approve("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 5);
}

TEST(OrderBookTest, ApproveWithInsufficientStockSetsProducingAndKeepsStock) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::OrderBook orderBook(catalog);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.approve("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 3);
}
