#include <gtest/gtest.h>

#include "service/SampleCatalog.h"

TEST(SampleCatalogTest, RegisterAddsSampleToList) {
    sos::SampleCatalog catalog;

    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].id(), "S-001");
}

TEST(SampleCatalogTest, RegisterDuplicateIdThrows) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));

    EXPECT_THROW(catalog.registerSample(sos::Sample("S-001", "Wafer-B", 5.0, 0.8, 3)),
                 std::invalid_argument);
}

TEST(SampleCatalogTest, SearchByNameReturnsPartialMatchesOnly) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    catalog.registerSample(sos::Sample("S-002", "Wafer-B", 12.5, 0.9, 10));
    catalog.registerSample(sos::Sample("S-003", "Ingot-C", 12.5, 0.9, 10));

    auto results = catalog.search("Wafer");

    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].id(), "S-001");
    EXPECT_EQ(results[1].id(), "S-002");
}

TEST(SampleCatalogTest, DecreaseStockReducesStock) {
    sos::SampleCatalog catalog;
    catalog.registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));

    catalog.decreaseStock("S-001", 4);

    auto samples = catalog.list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 6);
}
