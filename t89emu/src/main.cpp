#include <iostream>

#include "Gui.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
    int debug = (atoi(argv[2]) == 1) ? 1 : 0;
    Gui interface(argv[1], debug);

    return 0;
}