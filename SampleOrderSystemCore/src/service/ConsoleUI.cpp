#include "service/ConsoleUI.h"

#include <stdexcept>
#include <string>

#include "service/DateTimeFormat.h"
#include "service/Monitoring.h"
#include "service/Summary.h"

namespace sos {

namespace {

const char* stockLevelLabel(StockLevel level) {
    switch (level) {
        case StockLevel::Sufficient:
            return "여유";
        case StockLevel::Insufficient:
            return "부족";
        case StockLevel::Depleted:
            return "고갈";
    }
    return "알수없음";
}

const char* orderStatusLabel(OrderStatus status) {
    switch (status) {
        case OrderStatus::Reserved:
            return "RESERVED";
        case OrderStatus::Rejected:
            return "REJECTED";
        case OrderStatus::Producing:
            return "PRODUCING";
        case OrderStatus::Confirmed:
            return "CONFIRMED";
        case OrderStatus::Release:
            return "RELEASE";
    }
    return "UNKNOWN";
}

}  // namespace

ConsoleUI::ConsoleUI(AppContext& appContext, NowProvider nowProvider)
    : appContext_(appContext), nowProvider_(std::move(nowProvider)) {}

void ConsoleUI::printBanner(std::ostream& out) {
    out << "============================================\n";
    out << "                  S-SEMI\n";
    out << "        반도체 시료 생산주문관리 시스템\n";
    out << "============================================\n";
}

void ConsoleUI::printMainMenu(std::ostream& out) {
    Summary summary = buildSummary(appContext_.sampleCatalog(), appContext_.orderBook(), appContext_.productionQueue());

    out << "========================================\n";
    out << "시스템 현황  " << formatDateTime(nowProvider_()) << "\n";
    out << "총 등록 시료: " << summary.sampleTypeCount << "  총 재고: " << summary.totalStock
        << "  전체 주문 건수: " << summary.totalOrderCount
        << "  생산라인 대기: " << summary.pendingProductionJobCount << "\n";
    out << "========================================\n";
    out << "1. 시료 관리\n";
    out << "2. 주문\n";
    out << "3. 모니터링\n";
    out << "4. 출고 처리\n";
    out << "5. 생산 라인\n";
    out << "0. 종료\n";
    out << "> ";
}

void ConsoleUI::run(std::istream& in, std::ostream& out) {
    printBanner(out);

    while (true) {
        appContext_.processCompletedProductionJobs();
        printMainMenu(out);

        std::string choice;
        if (!std::getline(in, choice)) {
            break;
        }

        if (choice == "0") {
            break;
        } else if (choice == "1") {
            handleSampleMenu(in, out);
        } else if (choice == "2") {
            handleOrderMenu(in, out);
        } else if (choice == "3") {
            handleMonitoringMenu(in, out);
        } else if (choice == "4") {
            handleReleaseMenu(in, out);
        } else if (choice == "5") {
            handleProductionLineMenu(in, out);
        } else {
            out << "알 수 없는 선택입니다.\n";
        }
    }

    appContext_.save();
}

void ConsoleUI::handleSampleMenu(std::istream& in, std::ostream& out) {
    while (true) {
        out << "-- 시료 관리 --\n";
        out << "1. 시료 등록\n";
        out << "2. 시료 조회\n";
        out << "3. 시료 검색\n";
        out << "0. 뒤로 가기\n> ";

        std::string choice;
        if (!std::getline(in, choice) || choice == "0") {
            return;
        }

        if (choice == "1") {
            std::string id, name, averageProductionTimeStr, yieldStr, stockStr;
            out << "시료 ID: ";
            std::getline(in, id);
            out << "이름: ";
            std::getline(in, name);
            out << "평균 생산시간(분): ";
            std::getline(in, averageProductionTimeStr);
            out << "수율: ";
            std::getline(in, yieldStr);
            out << "재고 수량: ";
            std::getline(in, stockStr);

            try {
                appContext_.sampleCatalog().registerSample(
                    Sample(id, name, std::stod(averageProductionTimeStr), std::stod(yieldStr), std::stoi(stockStr)));
                out << "시료가 등록되었습니다.\n";
            } catch (const std::exception& e) {
                out << "등록 실패: " << e.what() << "\n";
            }
        } else if (choice == "2") {
            for (const auto& sample : appContext_.sampleCatalog().list()) {
                out << sample.id() << " | " << sample.name() << " | 재고 " << sample.stock() << "\n";
            }
        } else if (choice == "3") {
            std::string query;
            out << "검색어: ";
            std::getline(in, query);
            for (const auto& sample : appContext_.sampleCatalog().search(query)) {
                out << sample.id() << " | " << sample.name() << " | 재고 " << sample.stock() << "\n";
            }
        } else {
            out << "알 수 없는 선택입니다.\n";
        }
    }
}

void ConsoleUI::handleOrderMenu(std::istream& in, std::ostream& out) {
    while (true) {
        out << "-- 주문 --\n";
        out << "1. 시료 주문 접수\n";
        out << "2. 접수된 주문 목록 / 승인\n";
        out << "3. 주문 거절\n";
        out << "0. 뒤로 가기\n> ";

        std::string choice;
        if (!std::getline(in, choice) || choice == "0") {
            return;
        }

        if (choice == "1") {
            std::string id, sampleId, customerName, quantityStr;
            out << "주문 ID: ";
            std::getline(in, id);
            out << "시료 ID: ";
            std::getline(in, sampleId);
            out << "고객명: ";
            std::getline(in, customerName);
            out << "주문 수량: ";
            std::getline(in, quantityStr);

            try {
                appContext_.orderBook().placeOrder(id, sampleId, customerName, std::stoi(quantityStr));
                out << "주문이 접수되었습니다.\n";
            } catch (const std::exception& e) {
                out << "접수 실패: " << e.what() << "\n";
            }
        } else if (choice == "2") {
            for (const auto& order : appContext_.orderBook().list()) {
                if (order.status() == OrderStatus::Reserved) {
                    out << order.id() << " | " << order.sampleId() << " | " << order.customerName() << " | "
                        << order.quantity() << "\n";
                }
            }
            out << "승인할 주문 ID (건너뛰려면 빈 줄): ";
            std::string orderId;
            std::getline(in, orderId);
            if (!orderId.empty()) {
                try {
                    appContext_.orderBook().approve(orderId);
                    out << "승인 처리되었습니다.\n";
                } catch (const std::exception& e) {
                    out << "승인 실패: " << e.what() << "\n";
                }
            }
        } else if (choice == "3") {
            std::string orderId;
            out << "거절할 주문 ID: ";
            std::getline(in, orderId);
            try {
                appContext_.orderBook().reject(orderId);
                out << "거절 처리되었습니다.\n";
            } catch (const std::exception& e) {
                out << "거절 실패: " << e.what() << "\n";
            }
        } else {
            out << "알 수 없는 선택입니다.\n";
        }
    }
}

void ConsoleUI::handleMonitoringMenu(std::istream& in, std::ostream& out) {
    while (true) {
        out << "-- 모니터링 --\n";
        auto orders = appContext_.orderBook().list();
        OrderStatusSummary orderSummary = summarizeOrdersByStatus(orders);
        out << "RESERVED: " << orderSummary.reservedCount << "  PRODUCING: " << orderSummary.producingCount
            << "  CONFIRMED: " << orderSummary.confirmedCount << "  RELEASE: " << orderSummary.releaseCount << "\n";

        for (const auto& sample : appContext_.sampleCatalog().list()) {
            int demand = totalDemandForSample(sample.id(), orders);
            StockLevel level = evaluateStockLevel(sample.stock(), demand);
            out << sample.id() << " | 재고 " << sample.stock() << " | 상태 " << stockLevelLabel(level) << "\n";
        }

        out << "0. 뒤로 가기\n> ";
        std::string choice;
        if (!std::getline(in, choice) || choice == "0") {
            return;
        }
    }
}

void ConsoleUI::handleReleaseMenu(std::istream& in, std::ostream& out) {
    while (true) {
        out << "-- 출고 처리 --\n";
        for (const auto& order : appContext_.orderBook().list()) {
            if (order.status() == OrderStatus::Confirmed) {
                out << order.id() << " | " << order.sampleId() << " | " << order.customerName() << "\n";
            }
        }
        out << "출고할 주문 ID (건너뛰려면 0): ";

        std::string orderId;
        if (!std::getline(in, orderId) || orderId == "0") {
            return;
        }

        try {
            appContext_.orderBook().release(orderId);
            out << "출고 처리되었습니다.\n";
        } catch (const std::exception& e) {
            out << "출고 실패: " << e.what() << "\n";
        }
    }
}

void ConsoleUI::handleProductionLineMenu(std::istream& in, std::ostream& out) {
    while (true) {
        out << "-- 생산 라인 --\n";
        for (const auto& job : appContext_.productionQueue().list()) {
            out << job.orderId << " | " << job.sampleId << " | 생산량 " << job.productionQuantity << "\n";
        }
        out << "0. 뒤로 가기\n> ";
        std::string choice;
        if (!std::getline(in, choice) || choice == "0") {
            return;
        }
    }
}

}  // namespace sos
