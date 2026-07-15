#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "service/AppContext.h"
#include "service/ConsoleUI.h"

namespace {

// Resolve the data file next to the executable itself, so the same data is
// used regardless of the working directory the program happens to be launched
// from (double-click, Ctrl+F5, a different build output folder, etc.).
std::filesystem::path dataFilePath() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        return std::filesystem::path(buffer).parent_path() / "sample_order_system_data.json";
    }
#endif
    return "sample_order_system_data.json";
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    sos::AppContext appContext(dataFilePath());
    sos::ConsoleUI ui(appContext);
    ui.run(std::cin, std::cout);
    return 0;
}
