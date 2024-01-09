#include <iostream>

#include "DwarfParser.h"
#include "ElfParser.h"
#include "Gui.h"
#include "Mcu.h"
#include "McuDebug.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }

    McuDebug *debug = McuDebug::getInstance(argv[1]);

    Gui *emulator = new Gui(debug);
    emulator->runApplication();
}