#include <gtest/gtest.h>

#include "service/ProductionQueue.h"

namespace {

std::chrono::system_clock::time_point fixedNow() {
    static auto now = std::chrono::system_clock::now();
    return now;
}

}  // namespace

TEST(ProductionQueueTest, ListReturnsPendingJobsWithoutRemovingThem) {
    sos::ProductionQueue queue(fixedNow);
    queue.enqueue("O-001", "S-001", 12, 10.0);
    queue.enqueue("O-002", "S-002", 5, 10.0);

    auto jobs = queue.list();

    ASSERT_EQ(jobs.size(), 2u);
    EXPECT_EQ(jobs[0].orderId, "O-001");
    EXPECT_EQ(jobs[1].orderId, "O-002");
    EXPECT_FALSE(queue.empty());
}

TEST(ProductionQueueTest, DequeueReturnsJobsInFifoOrder) {
    sos::ProductionQueue queue(fixedNow);
    queue.enqueue("O-001", "S-001", 12, 10.0);
    queue.enqueue("O-002", "S-002", 5, 10.0);

    sos::ProductionJob first = queue.dequeue();
    sos::ProductionJob second = queue.dequeue();

    EXPECT_EQ(first.orderId, "O-001");
    EXPECT_EQ(second.orderId, "O-002");
    EXPECT_TRUE(queue.empty());
}

TEST(ProductionQueueTest, SecondJobStartsOnlyAfterFirstJobCompletes) {
    sos::ProductionQueue queue(fixedNow);
    queue.enqueue("O-001", "S-001", 12, 10.0);
    queue.enqueue("O-002", "S-002", 5, 20.0);

    auto jobs = queue.list();

    ASSERT_EQ(jobs.size(), 2u);
    auto expectedFirstCompletion = fixedNow() + std::chrono::minutes(10);
    auto expectedSecondCompletion = expectedFirstCompletion + std::chrono::minutes(20);
    EXPECT_EQ(jobs[0].completionTime, expectedFirstCompletion);
    EXPECT_EQ(jobs[1].completionTime, expectedSecondCompletion);
}
