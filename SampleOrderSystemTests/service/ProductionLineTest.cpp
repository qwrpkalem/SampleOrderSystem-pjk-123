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
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);
    orderBook.approve(placed.id());
    // shortage = 2, productionQuantity = ceil(2/0.9) = 3, duration = 12.5 * 3 = 37.5 minutes
    // Stock is unaffected by approval; it remains at 3.

    currentTime += std::chrono::minutes(38);  // advance past completion time

    sos::ProductionLine productionLine(orderBook, catalog, productionQueue, clock);
    productionLine.processCompletedJobs();

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    // The full production quantity is added to stock on completion (3 original + 3 produced).
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
    sos::Order placed = orderBook.placeOrder("S-001", "Acme Labs", 5);
    orderBook.approve(placed.id());
    // duration = 37.5 minutes, only 10 minutes have passed -> not yet complete
    // Stock is unaffected by approval; it remains at 3.

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

TEST(ProductionLineTest, RestartRecoveryCompletesJobsThatFinishedWhileProgramWasOff) {
    // Verifies that a job whose completionTime already passed while the program was
    // off gets completed immediately once its persisted state is restored at restart.
    auto restartTime = std::chrono::system_clock::now();
    auto clock = [&restartTime]() { return restartTime; };
    auto pastCompletionTime = restartTime - std::chrono::hours(1);

    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 0));

    sos::ProductionQueue productionQueue(clock);
    std::vector<sos::ProductionJob> savedJobs;
    savedJobs.push_back(sos::ProductionJob{"O-001", "S-001", 2, 3, pastCompletionTime});
    productionQueue.restore(savedJobs);

    sos::OrderBook orderBook(catalog, productionQueue);
    std::vector<sos::Order> savedOrders;
    savedOrders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);
    orderBook.restoreOrders(savedOrders);

    sos::ProductionLine productionLine(orderBook, catalog, productionQueue, clock);
    productionLine.processCompletedJobs();

    auto orders = orderBook.list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    // The full production quantity is added to stock on completion (0 original + 3 produced).
    EXPECT_EQ(samples[0].stock(), 3);

    EXPECT_TRUE(productionQueue.empty());
}
