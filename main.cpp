#include "ui/ConsoleUI.h"
#include "ui/GraphicUI.h"

#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::strcmp(argv[1], "--console") == 0 || std::strcmp(argv[1], "-c") == 0)) {
        std::cout << ">>> Khoi chay giao dien Console Terminal <<<\n";
        runConsoleApp();
    } else {
        std::cout << ">>> Khoi chay giao dien Do hoa Desktop GUI (Raylib) <<<\n";
        std::cout << "(Mẹo: De chay ban Console, go: ./build/app --console)\n";
        runGraphicApp();
    }
    return 0;
}
