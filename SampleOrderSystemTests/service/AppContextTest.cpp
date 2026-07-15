#include <gtest/gtest.h>

#include <filesystem>

#include "service/AppContext.h"

namespace {

std::filesystem::path uniqueTempFilePath(const char* testName) {
    auto path = std::filesystem::temp_directory_path() / (std::string("sos_appctx_test_") + testName + ".json");
    std::filesystem::remove(path);
    return path;
}

}  // namespace

TEST(AppContextTest, LoadsSamplesOrdersAndProductionJobsFromRepository) {
    auto path = uniqueTempFilePath("load");
    {
        sos::Repository repository(path);
        sos::SystemState state;
        state.samples.emplace_back("S-001", "Wafer-A", 12.5, 0.9, 3);
        state.orders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);
        state.productionJobs.push_back(
            sos::ProductionJob{"O-001", "S-001", 2, 3, std::chrono::system_clock::now() - std::chrono::hours(1)});
        repository.save(state);
    }

    sos::AppContext appContext(path);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].id(), "S-001");

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].id(), "O-001");
}

TEST(AppContextTest, CatchesUpOverdueProductionJobsOnLoad) {
    auto path = uniqueTempFilePath("catchup");
    {
        sos::Repository repository(path);
        sos::SystemState state;
        state.samples.emplace_back("S-001", "Wafer-A", 12.5, 0.9, 3);
        state.orders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);
        state.productionJobs.push_back(
            sos::ProductionJob{"O-001", "S-001", 2, 3, std::chrono::system_clock::now() - std::chrono::hours(1)});
        repository.save(state);
    }

    sos::AppContext appContext(path);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 6);
}

TEST(AppContextTest, ProcessCompletedProductionJobsAppliesProductionThatFinishedDuringTheSameSession) {
    auto path = uniqueTempFilePath("process_within_session");
    auto currentTime = std::chrono::system_clock::now();
    auto clock = [&currentTime]() { return currentTime; };

    sos::AppContext appContext(path, clock);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::Order placed = appContext.orderBook().placeOrder("S-001", "Acme Labs", 5);
    appContext.orderBook().approve(placed.id());
    // shortage = 2, productionQuantity = ceil(2/0.9) = 3, duration = 12.5 * 3 = 37.5 minutes

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders[0].status(), sos::OrderStatus::Producing);

    currentTime += std::chrono::minutes(38);  // time passes within the same running session
    appContext.processCompletedProductionJobs();

    orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Confirmed);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 6);
}

TEST(AppContextTest, SavePersistsCurrentStateBackToRepository) {
    auto path = uniqueTempFilePath("save");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));

    appContext.save();

    sos::Repository repository(path);
    sos::SystemState reloaded = repository.load();
    ASSERT_EQ(reloaded.samples.size(), 1u);
    EXPECT_EQ(reloaded.samples[0].id(), "S-001");
}
