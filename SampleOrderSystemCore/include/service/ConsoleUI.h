#pragma once

#include <istream>
#include <ostream>

#include "service/AppContext.h"

namespace sos {

class ConsoleUI {
public:
    explicit ConsoleUI(AppContext& appContext);

    void run(std::istream& in, std::ostream& out);

private:
    AppContext& appContext_;

    void printMainMenu(std::ostream& out);
    void handleSampleMenu(std::istream& in, std::ostream& out);
    void handleOrderMenu(std::istream& in, std::ostream& out);
    void handleMonitoringMenu(std::istream& in, std::ostream& out);
    void handleReleaseMenu(std::istream& in, std::ostream& out);
    void handleProductionLineMenu(std::istream& in, std::ostream& out);
};

}  // namespace sos
