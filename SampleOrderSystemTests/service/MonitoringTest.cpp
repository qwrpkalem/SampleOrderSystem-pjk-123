#include <gtest/gtest.h>

#include "service/Monitoring.h"

TEST(MonitoringTest, SummarizeOrdersByStatusExcludesRejected) {
    std::vector<sos::Order> orders;
    orders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Reserved);
    orders.emplace_back("O-002", "S-001", "Acme Labs", 3, sos::OrderStatus::Reserved);
    orders.emplace_back("O-003", "S-001", "Acme Labs", 2, sos::OrderStatus::Producing);
    orders.emplace_back("O-004", "S-001", "Acme Labs", 1, sos::OrderStatus::Confirmed);
    orders.emplace_back("O-005", "S-001", "Acme Labs", 4, sos::OrderStatus::Release);
    orders.emplace_back("O-006", "S-001", "Acme Labs", 9, sos::OrderStatus::Rejected);

    sos::OrderStatusSummary summary = sos::summarizeOrdersByStatus(orders);

    EXPECT_EQ(summary.reservedCount, 2);
    EXPECT_EQ(summary.producingCount, 1);
    EXPECT_EQ(summary.confirmedCount, 1);
    EXPECT_EQ(summary.releaseCount, 1);
}

TEST(MonitoringTest, EvaluateStockLevelIsDepletedWhenStockIsZeroRegardlessOfDemand) {
    EXPECT_EQ(sos::evaluateStockLevel(0, 5), sos::StockLevel::Depleted);
    EXPECT_EQ(sos::evaluateStockLevel(0, 0), sos::StockLevel::Depleted);
}

TEST(MonitoringTest, EvaluateStockLevelIsSufficientWhenStockCoversDemand) {
    EXPECT_EQ(sos::evaluateStockLevel(10, 5), sos::StockLevel::Sufficient);
    EXPECT_EQ(sos::evaluateStockLevel(5, 5), sos::StockLevel::Sufficient);
}

TEST(MonitoringTest, EvaluateStockLevelIsInsufficientWhenStockIsBelowDemand) {
    EXPECT_EQ(sos::evaluateStockLevel(3, 5), sos::StockLevel::Insufficient);
}

TEST(MonitoringTest, TotalDemandForSampleSumsOnlyReservedOrdersForThatSample) {
    std::vector<sos::Order> orders;
    orders.emplace_back("O-001", "S-001", "Acme Labs", 5, sos::OrderStatus::Reserved);
    orders.emplace_back("O-002", "S-001", "Acme Labs", 3, sos::OrderStatus::Reserved);
    orders.emplace_back("O-003", "S-002", "Acme Labs", 100, sos::OrderStatus::Reserved);  // different sample
    orders.emplace_back("O-004", "S-001", "Acme Labs", 7, sos::OrderStatus::Confirmed);   // already fulfilled

    int demand = sos::totalDemandForSample("S-001", orders);

    EXPECT_EQ(demand, 8);
}
