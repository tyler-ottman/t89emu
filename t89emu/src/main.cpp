#include <iostream>

#include "Gui.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }

    ElfParser *parser = new ElfParser(argv[1]);
    Mcu *t89 = Mcu::getInstance(parser->getRomStart(), parser->getRamStart(),
                                parser->getEntryPc());
    parser->flashRom(t89->getRomDevice());

    Gui *emulator = new Gui(parser, 0);
    emulator->runApplication();
}