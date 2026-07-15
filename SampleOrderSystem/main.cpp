#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "service/AppContext.h"
#include "service/ConsoleUI.h"

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    sos::AppContext appContext("sample_order_system_data.json");
    sos::ConsoleUI ui(appContext);
    ui.run(std::cin, std::cout);
    return 0;
}
