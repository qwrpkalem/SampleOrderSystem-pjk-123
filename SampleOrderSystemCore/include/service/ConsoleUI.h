#pragma once

#include <chrono>
#include <functional>
#include <istream>
#include <ostream>

#include "service/AppContext.h"

namespace sos {

class ConsoleUI {
public:
    using NowProvider = std::function<std::chrono::system_clock::time_point()>;

    explicit ConsoleUI(AppContext& appContext, NowProvider nowProvider = &std::chrono::system_clock::now);

    void run(std::istream& in, std::ostream& out);

private:
    AppContext& appContext_;
    NowProvider nowProvider_;

    void printBanner(std::ostream& out);
    void printMainMenu(std::ostream& out);
    void handleSampleMenu(std::istream& in, std::ostream& out);
    void handleOrderMenu(std::istream& in, std::ostream& out);
    void handleMonitoringMenu(std::istream& in, std::ostream& out);
    void handleReleaseMenu(std::istream& in, std::ostream& out);
    void handleProductionLineMenu(std::istream& in, std::ostream& out);
};

}  // namespace sos
