#include <stdio.h>
// #if defined(IMGUI_IMPL_OPENGL_ES2)
// #include <GLES2/gl2.h>
// #endif
// #include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <vector>
#include <utility>
#include <string>
#include <stdint.h>
#include <cmath>
#include <cfenv>
#include <iostream>
#include <map>
#include <fstream>

#include "pipeline.h"
#include "gui.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
	int debug = 0;
	if (argc == 4)
		debug = (atoi(argv[3]) == 1) ? 1 : 0;

    // Initialize Emulator
    Pipeline t89(argv[1], argv[2], debug);
    uint32_t* vram = t89.dram.video_memory;
    gui interface(vram);
    GLFWwindow* window = interface.get_window();

    bool show_demo_window = true;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        // for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
        //     t89.next_instruction();
        // }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow(&show_demo_window);

        interface.render_memory_viewer();
        interface.render_control_panel();
        interface.render_register_bank(t89.rf);
        interface.render_io_panel();
        interface.render_lcd_display();
        // interface.render_disassembled_code_section();
        interface.render_frame();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}