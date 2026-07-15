#include <gtest/gtest.h>

#include <sstream>

#include "service/AppContext.h"
#include "service/ConsoleUI.h"
#include "service/DateTimeFormat.h"

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

TEST(ConsoleUITest, MainMenuLabelsOrderMenuAsSampleOrder) {
    auto path = uniqueTempFilePath("order_label");
    sos::AppContext appContext(path);

    std::istringstream input("0\n");
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    EXPECT_NE(output.str().find("시료 주문"), std::string::npos);
}

TEST(ConsoleUITest, StartupShowsSSemiBannerOnceBeforeMainMenu) {
    auto path = uniqueTempFilePath("banner");
    sos::AppContext appContext(path);

    std::istringstream input("0\n");
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    std::string text = output.str();
    auto bannerPos = text.find("S-SEMI");
    auto mainMenuPos = text.find("시료 관리");
    ASSERT_NE(bannerPos, std::string::npos);
    ASSERT_NE(mainMenuPos, std::string::npos);
    EXPECT_LT(bannerPos, mainMenuPos);

    // Banner should appear exactly once even though the main menu can loop.
    EXPECT_EQ(text.find("S-SEMI", bannerPos + 1), std::string::npos);
}

TEST(ConsoleUITest, MainMenuShowsCurrentDateTimeAndCountsUnderSystemStatus) {
    auto path = uniqueTempFilePath("system_status");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));

    auto fixedNow = std::chrono::system_clock::now();
    std::istringstream input("0\n");
    std::ostringstream output;

    sos::ConsoleUI ui(appContext, [fixedNow]() { return fixedNow; });
    ui.run(input, output);

    std::string text = output.str();
    EXPECT_NE(text.find("시스템 현황"), std::string::npos);
    EXPECT_NE(text.find(sos::formatDateTime(fixedNow)), std::string::npos);
    EXPECT_NE(text.find("총 등록 시료"), std::string::npos);
    EXPECT_NE(text.find("총 재고"), std::string::npos);
    EXPECT_NE(text.find("전체 주문 건수"), std::string::npos);
    EXPECT_NE(text.find("생산라인 대기"), std::string::npos);
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
        "S-001\n"
        "Acme Labs\n"
        "5\n"
        "Y\n"        // confirm reservation
        "0\n"        // back to main
        "3\n"        // main -> approval menu
        "1\n"        // approve
        "O-0001\n"   // the auto-generated order id
        "Y\n"        // confirm approval
        "0\n"        // back to main
        "5\n"        // main -> release menu
        "O-0001\n"   // release
        "0\n"        // back to main
        "0\n");       // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].id(), "O-0001");
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Release);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].stock(), 5);

    EXPECT_NE(output.str().find("시료가 등록되었습니다."), std::string::npos);
    EXPECT_NE(output.str().find("시료 주문이 접수되었습니다. (주문번호: O-0001)"), std::string::npos);
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

TEST(ConsoleUITest, PlaceOrderShowsSampleListAndConfirmationBeforeAssigningIdAutomatically) {
    auto path = uniqueTempFilePath("order_sample_selection");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    appContext.sampleCatalog().registerSample(sos::Sample("S-002", "Ingot-B", 5.0, 0.8, 4));

    std::istringstream input(
        "2\n"          // main -> order menu
        "1\n"          // place order
        "S-002\n"
        "Acme Labs\n"
        "3\n"
        "Y\n"          // confirm reservation
        "0\n"          // back to main
        "0\n");        // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    std::string text = output.str();
    // The sample list should be shown for reference before asking for the sample ID.
    EXPECT_NE(text.find("S-001"), std::string::npos);
    EXPECT_NE(text.find("S-002"), std::string::npos);
    // The confirmation summary should be shown before the reservation is placed.
    EXPECT_NE(text.find("[확인]"), std::string::npos);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].sampleId(), "S-002");
    EXPECT_EQ(orders[0].quantity(), 3);
    // The order ID must be assigned by the system, not entered by the user.
    EXPECT_EQ(orders[0].id(), "O-0001");
}

TEST(ConsoleUITest, OrderRejectMenuTransitionsOrderToRejected) {
    auto path = uniqueTempFilePath("reject");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 10));
    sos::Order placed = appContext.orderBook().placeOrder("S-001", "Acme Labs", 5);

    std::istringstream input(
        "3\n"        // main -> approval menu
        "2\n"        // reject
        + placed.id() + "\n" +
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

TEST(ConsoleUITest, ProductionCompletesWithinTheSameRunningSessionOnceTimePasses) {
    // Regression test: previously, completed production jobs were only caught up when the
    // program restarted. This verifies it also happens while the app keeps running, once the
    // user returns to the main menu after enough (wall-clock) time has passed.
    auto path = uniqueTempFilePath("session_time_passes");
    auto currentTime = std::chrono::system_clock::now();
    auto clock = [&currentTime]() { return currentTime; };
    sos::AppContext appContext(path, clock);

    sos::ConsoleUI ui(appContext, clock);

    std::istringstream setupInput(
        "1\n1\nS-001\nWafer-A\n12.5\n0.9\n3\n0\n"  // register sample with stock 3
        "2\n1\nS-001\nAcme Labs\n5\nY\n0\n"          // place order for qty 5, confirm
        "3\n1\nO-0001\nY\n0\n"                       // approve (insufficient stock -> Producing)
        "0\n");                                     // exit
    std::ostringstream setupOutput;
    ui.run(setupInput, setupOutput);

    auto ordersBefore = appContext.orderBook().list();
    ASSERT_EQ(ordersBefore.size(), 1u);
    ASSERT_EQ(ordersBefore[0].status(), sos::OrderStatus::Producing);

    // Time passes while the console app keeps running (no restart, no AppContext recreated).
    currentTime += std::chrono::minutes(38);  // shortage=2, qty=ceil(2/0.9)=3, duration=12.5*3=37.5min

    std::istringstream resumeInput("0\n");  // just return to main menu and exit
    std::ostringstream resumeOutput;
    ui.run(resumeInput, resumeOutput);

    auto ordersAfter = appContext.orderBook().list();
    ASSERT_EQ(ordersAfter.size(), 1u);
    EXPECT_EQ(ordersAfter[0].status(), sos::OrderStatus::Confirmed);

    auto samples = appContext.sampleCatalog().list();
    ASSERT_EQ(samples.size(), 1u);
    // Only the yield-rounding surplus (productionQuantity 3 - shortage 2 = 1) becomes real stock.
    EXPECT_EQ(samples[0].stock(), 1);
}

TEST(ConsoleUITest, ApproveWithInsufficientStockShowsProducingOrderInProductionLineMenu) {
    auto path = uniqueTempFilePath("approve_insufficient");
    sos::AppContext appContext(path);
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 12.5, 0.9, 3));
    sos::Order placed = appContext.orderBook().placeOrder("S-001", "Acme Labs", 5);

    std::istringstream input(
        "3\n"        // main -> approval menu
        "1\n"        // approve
        + placed.id() + "\n" +  // approve (stock insufficient -> Producing)
        "Y\n"        // confirm approval
        "0\n"        // back to main
        "6\n"        // main -> production line menu
        "0\n"        // back to main
        "0\n");      // exit
    std::ostringstream output;

    sos::ConsoleUI ui(appContext);
    ui.run(input, output);

    auto orders = appContext.orderBook().list();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status(), sos::OrderStatus::Producing);
    EXPECT_NE(output.str().find(placed.id() + " | S-001"), std::string::npos);
}

TEST(ConsoleUITest, ProductionLineMenuShowsOrderNumberSampleQuantityShortageProductionQuantityAndRemainingTime) {
    auto path = uniqueTempFilePath("production_line_detail");
    auto currentTime = std::chrono::system_clock::now();
    auto clock = [&currentTime]() { return currentTime; };
    sos::AppContext appContext(path, clock);

    // yield=1.0 keeps productionQuantity == shortage exactly, and averageProductionTime=10.0
    // keeps the total duration a whole number of minutes, so the test isn't sensitive to
    // rounding of fractional minutes.
    appContext.sampleCatalog().registerSample(sos::Sample("S-001", "Wafer-A", 10.0, 1.0, 3));
    sos::Order placed = appContext.orderBook().placeOrder("S-001", "Acme Labs", 8);
    appContext.orderBook().approve(placed.id());
    // shortage = 8 - 3 = 5, productionQuantity = ceil(5/1.0) = 5, duration = 10.0 * 5 = 50 minutes

    currentTime += std::chrono::minutes(20);  // 30 minutes remaining

    sos::ConsoleUI ui(appContext, clock);
    std::istringstream input("6\n0\n0\n");  // main -> production line menu -> back -> exit
    std::ostringstream output;
    ui.run(input, output);

    std::string text = output.str();
    EXPECT_NE(text.find("순서"), std::string::npos);
    EXPECT_NE(text.find("주문번호"), std::string::npos);
    EXPECT_NE(text.find("주문량"), std::string::npos);
    EXPECT_NE(text.find("부족분"), std::string::npos);
    EXPECT_NE(text.find("실생산량"), std::string::npos);
    EXPECT_NE(text.find("남은시간"), std::string::npos);

    std::ostringstream expectedRow;
    expectedRow << "1 | " << placed.id() << " | S-001 | 8 | 5 | 5 | 30분";
    EXPECT_NE(text.find(expectedRow.str()), std::string::npos) << "actual output:\n" << text;
}
