#include <iostream>

#include "DwarfParser.h"
#include "ElfParser.h"
#include "Gui.h"
#include "Mcu.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }

    ElfParser *parser = new ElfParser(argv[1]);
    Mcu *t89 = Mcu::getInstance(parser->getRomStart(), parser->getRamStart(),
                                parser->getEntryPc());
    parser->flashRom(t89->getRomDevice());

    DwarfParser *debugParser = new DwarfParser(argv[1]);

    Gui *emulator = new Gui(parser, 0);
    emulator->runApplication();
}