#include <vector>
#include <utility>
#include <string>
// #include <stdint.h>
#include <iostream>

#include "pipeline.h"
#include "gui.h"

int main(int argc, char **argv)
{
    if (argc != 5) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
	int debug = (atoi(argv[4]) == 1) ? 1 : 0;

    // Initialize Emulator
    // Pipeline t89(argv[1], argv[2], debug);
    // uint32_t* vram = t89.dram.video_memory;
    // uint32_t* rom = t89.dram.instruction_memory;
    // uint32_t* ram = t89.dram.data_memory;
    // gui interface(vram, &t89.rf, rom, ram, &t89.pc.PC);
    gui interface(argv[1], argv[2], argv[3], debug);
    // interface.load_disassembled_code(argv[3]);
    // interface.run_debug_application();
    
    // exit(EXIT_SUCCESS);
    // GLFWwindow* window = interface.get_window();

    // // bool show_demo_window = true;

    // // Main loop
    // while (!glfwWindowShouldClose(window))
    // {
    //     // Poll and handle events (inputs, window resize, etc.)
    //     glfwPollEvents();
    //     // int num_instructions = 0;
    //     if (interface.is_run_enabled) {
    //         for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
    //             t89.next_instruction();
    //         }
    //     }
    //     if (interface.is_step_enabled) {
    //         t89.next_instruction();
    //     }

    //     // Start the Dear ImGui frame
    //     ImGui_ImplOpenGL3_NewFrame();
    //     ImGui_ImplGlfw_NewFrame();
    //     ImGui::NewFrame();

    //     // ImGui::ShowDemoWindow(&show_demo_window);

    //     interface.render_memory_viewer();
    //     interface.render_control_panel();
    //     interface.render_register_bank();
    //     interface.render_io_panel();
    //     interface.render_lcd_display();
    //     interface.render_disassembled_code_section();
    //     interface.render_frame();
    // }

    // // Cleanup
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    // glfwDestroyWindow(window);
    // glfwTerminate();

    return 0;
}