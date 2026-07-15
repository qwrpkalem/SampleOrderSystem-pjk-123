#include <gtest/gtest.h>

#include "service/OrderBook.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

TEST(OrderBookTest, PlaceOrderCreatesReservedOrderInList) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

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
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    EXPECT_THROW(orderBook.placeOrder("O-001", "UNKNOWN", "Acme Labs", 5), std::invalid_argument);
}

TEST(OrderBookTest, RejectTransitionsReservedOrderToRejected) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.reject("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Rejected);
}

TEST(OrderBookTest, ApproveWithSufficientStockConfirmsAndDecreasesStock) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.approve("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 5);

    EXPECT_TRUE(productionQueue.empty());
}

TEST(OrderBookTest, ApproveWithInsufficientStockSetsProducingAndEnqueuesProductionJob) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);

    orderBook.approve("O-001");

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 3);

    ASSERT_FALSE(productionQueue.empty());
    auto jobs = productionQueue.list();
    ASSERT_EQ(jobs.size(), 1u);
    EXPECT_EQ(jobs[0].orderId, "O-001");
    EXPECT_EQ(jobs[0].sampleId, "S-001");
    // shortage = 5 - 3 = 2, productionQuantity = ceil(2 / 0.9) = 3
    EXPECT_EQ(jobs[0].productionQuantity, 3);
}
