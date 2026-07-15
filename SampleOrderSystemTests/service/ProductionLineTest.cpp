#include <gtest/gtest.h>

#include "service/OrderBook.h"
#include "service/ProductionLine.h"
#include "service/ProductionQueue.h"
#include "service/SampleCatalog.h"

TEST(ProductionLineTest, ProcessCompletedJobsUpdatesStockAndConfirmsOrder) {
    auto currentTime = std::chrono::system_clock::now();
    auto clock = [&currentTime]() { return currentTime; };

    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::ProductionQueue productionQueue(clock);
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);
    orderBook.approve("O-001");
    // shortage = 2, productionQuantity = ceil(2/0.9) = 3, duration = 12.5 * 3 = 37.5 minutes

    currentTime += std::chrono::minutes(38);  // advance past completion time

    sos::ProductionLine productionLine(orderBook, catalog, productionQueue, clock);
    productionLine.processCompletedJobs();

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 6);

    EXPECT_TRUE(productionQueue.empty());
}

TEST(ProductionLineTest, ProcessCompletedJobsLeavesUnfinishedJobInQueue) {
    auto currentTime = std::chrono::system_clock::now();
    auto clock = [&currentTime]() { return currentTime; };

    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::ProductionQueue productionQueue(clock);
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("O-001", "S-001", "Acme Labs", 5);
    orderBook.approve("O-001");
    // duration = 37.5 minutes, only 10 minutes have passed -> not yet complete

    currentTime += std::chrono::minutes(10);

    sos::ProductionLine productionLine(orderBook, catalog, productionQueue, clock);
    productionLine.processCompletedJobs();

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 3);

    EXPECT_FALSE(productionQueue.empty());
}
