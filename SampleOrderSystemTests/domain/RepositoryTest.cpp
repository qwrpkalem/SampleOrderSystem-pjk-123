#include <gtest/gtest.h>

#include <filesystem>

#include "persistence/Repository.h"

namespace {

std::filesystem::path uniqueTempFilePath(const char* testName) {
    auto path = std::filesystem::temp_directory_path() / (std::string("sos_repo_test_") + testName + ".json");
    std::filesystem::remove(path);
    return path;
}

}  // namespace

TEST(RepositoryTest, LoadWithoutExistingFileReturnsEmptyState) {
    auto path = uniqueTempFilePath("load_missing_file");
    sos::Repository repository(path);

    sos::SystemState state = repository.load();

    EXPECT_TRUE(state.samples.empty());
    EXPECT_TRUE(state.orders.empty());
    EXPECT_TRUE(state.productionJobs.empty());
}

TEST(RepositoryTest, SaveThenLoadRoundTripsSamplesAndOrders) {
    auto path = uniqueTempFilePath("save_then_load");
    sos::Repository repository(path);

    sos::SystemState state;
    state.samples.emplace_back("S-001", "Wafer-A", 12.5, 0.9, 10);
    state.orders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Producing);

    repository.save(state);
    sos::SystemState restored = repository.load();

    ASSERT_EQ(restored.samples.size(), 1u);
    EXPECT_EQ(restored.samples[0].id(), "S-001");

    ASSERT_EQ(restored.orders.size(), 1u);
    EXPECT_EQ(restored.orders[0].id(), "O-001");
    EXPECT_EQ(restored.orders[0].status(), sos::OrderStatus::Producing);
}

TEST(RepositoryTest, SaveThenLoadRoundTripsProductionJobs) {
    auto path = uniqueTempFilePath("save_then_load_production_jobs");
    sos::Repository repository(path);

    auto completionTime = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    sos::SystemState state;
    state.productionJobs.push_back(sos::ProductionJob{"O-001", "S-001", 2, 12, completionTime});

    repository.save(state);
    sos::SystemState restored = repository.load();

    ASSERT_EQ(restored.productionJobs.size(), 1u);
    EXPECT_EQ(restored.productionJobs[0].orderId, "O-001");
    EXPECT_EQ(restored.productionJobs[0].sampleId, "S-001");
    EXPECT_EQ(restored.productionJobs[0].productionQuantity, 12);
    EXPECT_EQ(restored.productionJobs[0].completionTime, completionTime);
}
