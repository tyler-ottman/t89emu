#include "terminal.h"

int kernel_main(void) {
    init_terminal();

    // Write 'Hello World!' to screen
    enable_text_mode();
    
    print_str_line("Terminal Init", 13);
    print_str("Hello World!", 12);

    return 0;
}
