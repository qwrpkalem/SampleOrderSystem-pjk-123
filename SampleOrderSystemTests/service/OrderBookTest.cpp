#include <gtest/gtest.h>

#include "service/OrderBook.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

TEST(OrderBookTest, PlaceOrderCreatesReservedOrderInList) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);

    EXPECT_EQ(placed.status(), sos::OrderStatus::Reserved);
    EXPECT_FALSE(placed.id().empty());

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].id(), placed.id());
    EXPECT_EQ(orders[0].sampleId(), "S-001");
    EXPECT_EQ(orders[0].customerName(), "Acme Labs");
    EXPECT_EQ(orders[0].quantity(), 5);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Reserved);
}

TEST(OrderBookTest, PlaceOrderAssignsIncrementingIdsAutomatically) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    sos::Order first = orderBook.placeOrder("S-001", "Acme Labs", 5);
    sos::Order second = orderBook.placeOrder("S-001", "Acme Labs", 3);

    EXPECT_NE(first.id(), second.id());
    EXPECT_EQ(first.id(), "O-0001");
    EXPECT_EQ(second.id(), "O-0002");
}

TEST(OrderBookTest, PlaceOrderWithUnknownSampleIdThrows) {
    sos::SampleCatalog catalog;
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    EXPECT_THROW(orderBook.placeOrder("UNKNOWN", "Acme Labs", 5), std::invalid_argument);
}

TEST(OrderBookTest, RejectTransitionsReservedOrderToRejected) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);

    orderBook.reject(placed.id());

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Rejected);
}

TEST(OrderBookTest, ApproveWithSufficientStockConfirmsAndDecreasesStock) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);

    orderBook.approve(placed.id());

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
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);

    orderBook.approve(placed.id());

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    // The stock on hand is fully committed to this order at approval time, so it must not
    // remain available for any other order on the same sample.
    EXPECT_EQ(samples[0].stock(), 0);

    ASSERT_FALSE(productionQueue.empty());
    auto jobs = productionQueue.list();
    ASSERT_EQ(jobs.size(), 1u);
    EXPECT_EQ(jobs[0].orderId, placed.id());
    EXPECT_EQ(jobs[0].sampleId, "S-001");
    // shortage = 5 - 3 = 2, productionQuantity = ceil(2 / 0.9) = 3
    EXPECT_EQ(jobs[0].productionQuantity, 3);
}

TEST(OrderBookTest, CompleteProductionTransitionsProducingOrderToConfirmed) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);
    orderBook.approve(placed.id());

    orderBook.completeProduction(placed.id());

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);
}

TEST(OrderBookTest, RestoreOrdersReplacesListWithGivenOrdersAsIs) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    std::vector<sos::Order> savedOrders;
    savedOrders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);

    orderBook.restoreOrders(savedOrders);

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].id(), "O-001");
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);
}

TEST(OrderBookTest, PlaceOrderAfterRestoreContinuesIdNumberingPastRestoredOrders) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);

    std::vector<sos::Order> savedOrders;
    savedOrders.emplace_back("O-0005", "S-001", "Acme Labs", 5, sos::OrderStatus::Confirmed);
    orderBook.restoreOrders(savedOrders);

    sos::Order next = orderBook.placeOrder("S-001", "Acme Labs", 3);

    EXPECT_EQ(next.id(), "O-0006");
}

TEST(OrderBookTest, ReleaseTransitionsConfirmedOrderToRelease) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);
    orderBook.approve(placed.id());

    orderBook.release(placed.id());

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Release);
}

TEST(OrderBookTest, ReleaseNonConfirmedOrderThrows) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);
    // still RESERVED, not CONFIRMED

    EXPECT_THROW(orderBook.release(placed.id()), std::invalid_argument);
}
