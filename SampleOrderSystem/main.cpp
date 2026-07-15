#include <iostream>

#include "service/AppContext.h"
#include "service/ConsoleUI.h"

int main() {
    sos::AppContext appContext("sample_order_system_data.json");
    sos::ConsoleUI ui(appContext);
    ui.run(std::cin, std::cout);
    return 0;
}
