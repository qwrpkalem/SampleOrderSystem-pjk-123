#include <gtest/gtest.h>

#include "service/Summary.h"

TEST(SummaryTest, BuildSummaryReportsSampleCountTotalStockAndOrderCount) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    catalog.registerSample(sos::Sample("S-002", "Wafer-B", 5.0, 0.8, 4));

    sos::ProductionQueue productionQueue;
    sos::OrderBook orderBook(catalog, productionQueue);
    orderBook.placeOrder("S-001", "Acme Labs", 3);
    orderBook.placeOrder("S-002", "Acme Labs", 2);

    sos::Summary summary = sos::buildSummary(catalog, orderBook, productionQueue);

    EXPECT_EQ(summary.sampleTypeCount, 2);
    EXPECT_EQ(summary.totalStock, 14);
    EXPECT_EQ(summary.totalOrderCount, 2);
    EXPECT_EQ(summary.pendingProductionJobCount, 0);
}
