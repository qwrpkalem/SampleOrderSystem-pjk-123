#include <gtest/gtest.h>

#include <sstream>

#include "service/AppContext.h"
#include "service/ConsoleUI.h"

namespace {

std::filesystem::path uniqueTempFilePath(const char* testName) {
    auto path = std::filesystem::temp_directory_path() / (std::string("sos_console_test_") + testName + ".json");
    std::filesystem::remove(path);
    return path;
}

}  // namespace

TEST(ConsoleUITest, EnteringZeroAtMainMenuExitsImmediately) {
    auto path = uniqueTempFilePath("exit");
    sos::AppContext appContext(path);

    std::istringstream input("0\n");
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    EXPECT_NE(output.str().find("시료 관리"), std::string::npos);
    EXPECT_NE(output.str().find("종료"), std::string::npos);
}

TEST(ConsoleUITest, FullLifecycleFlowThroughMenusRegistersOrdersApprovesAndReleases) {
    auto path = uniqueTempFilePath("full_lifecycle");
    sos::AppContext appContext(path);

    std::istringstream input(
        "1\n"        // main -> sample menu
        "1\n"        // register sample
        "S-001\n"
        "Wafer-A\n"
        "12.5\n"
        "0.9\n"
        "10\n"
        "0\n"        // back to main
        "2\n"        // main -> order menu
        "1\n"        // place order
        "O-001\n"
        "S-001\n"
        "Acme Labs\n"
        "5\n"
        "2\n"        // list reserved / approve
        "O-001\n"    // approve O-001
        "0\n"        // back to main
        "4\n"        // main -> release menu
        "O-001\n"    // release O-001
        "0\n"        // back to main
        "0\n");       // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Release);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 5);

    EXPECT_NE(output.str().find("시료가 등록되었습니다."), std::string::npos);
    EXPECT_NE(output.str().find("주문이 접수되었습니다."), std::string::npos);
    EXPECT_NE(output.str().find("승인 처리되었습니다."), std::string::npos);
    EXPECT_NE(output.str().find("출고 처리되었습니다."), std::string::npos);
}

TEST(ConsoleUITest, SampleSearchMenuFindsRegisteredSampleByPartialName) {
    auto path = uniqueTempFilePath("search");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    appContext.sampleCatalog().registerSample(sos::Sample("S-002", "Ingot-B", 5.0, 0.8, 4));

    std::istringstream input(
        "1\n"        // main -> sample menu
        "3\n"        // search
        "Wafer\n"    // query
        "0\n"        // back to main
        "0\n");      // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    EXPECT_NE(output.str().find("S-001"), std::string::npos);
    EXPECT_EQ(output.str().find("S-002"), std::string::npos);
}

TEST(ConsoleUITest, OrderRejectMenuTransitionsOrderToRejected) {
    auto path = uniqueTempFilePath("reject");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    appContext.orderBook().placeOrder("O-001", "S-001", "Acme Labs", 5);

    std::istringstream input(
        "2\n"        // main -> order menu
        "3\n"        // reject
        "O-001\n"
        "0\n"        // back to main
        "0\n");      // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Rejected);
    EXPECT_NE(output.str().find("거절 처리되었습니다."), std::string::npos);
}

TEST(ConsoleUITest, ApproveWithInsufficientStockShowsProducingOrderInProductionLineMenu) {
    auto path = uniqueTempFilePath("approve_insufficient");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    appContext.orderBook().placeOrder("O-001", "S-001", "Acme Labs", 5);

    std::istringstream input(
        "2\n"        // main -> order menu
        "2\n"        // list reserved / approve
        "O-001\n"    // approve (stock insufficient -> Producing)
        "0\n"        // back to main
        "5\n"        // main -> production line menu
        "0\n"        // back to main
        "0\n");      // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);
    EXPECT_NE(output.str().find("O-001 | S-001 | 생산량"), std::string::npos);
}
