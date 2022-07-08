#include <iostream>
#include "gui.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
	int debug = (atoi(argv[3]) == 1) ? 1 : 0;
    gui interface(argv[1], argv[2], debug);

    return 0;
}