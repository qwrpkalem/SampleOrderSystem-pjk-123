#include <gtest/gtest.h>

#include "service/ProductionQueue.h"

TEST(ProductionQueueTest, ListReturnsPendingJobsWithoutRemovingThem) {
    sos::ProductionQueue queue;
    auto completionTime = std::chrono::system_clock::now();
    queue.enqueue(sos::ProductionJob{"O-001", "S-001", 12, completionTime});
    queue.enqueue(sos::ProductionJob{"O-002", "S-002", 5, completionTime});

    auto jobs = queue.list();

    ASSERT_EQ(jobs.size(), 2u);
    EXPECT_EQ(jobs[0].orderId, "O-001");
    EXPECT_EQ(jobs[1].orderId, "O-002");
    EXPECT_FALSE(queue.empty());
}

TEST(ProductionQueueTest, DequeueReturnsJobsInFifoOrder) {
    sos::ProductionQueue queue;
    auto completionTime = std::chrono::system_clock::now();
    queue.enqueue(sos::ProductionJob{"O-001", "S-001", 12, completionTime});
    queue.enqueue(sos::ProductionJob{"O-002", "S-002", 5, completionTime});

    sos::ProductionJob first = queue.dequeue();
    sos::ProductionJob second = queue.dequeue();

    EXPECT_EQ(first.orderId, "O-001");
    EXPECT_EQ(second.orderId, "O-002");
    EXPECT_TRUE(queue.empty());
}
