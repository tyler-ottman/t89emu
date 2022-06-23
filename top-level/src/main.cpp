#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <vector>
#include <utility>
#include <string>
#include <stdint.h>
#include <cmath>
#include <cfenv>
#include <iostream>
#include <map>
#include <fstream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "pipeline.h"
#include "register_information.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
        struct funcs
        {
            static bool IsLegacyNativeDupe(ImGuiKey) { return false; }
        };
        // const ImGuiKey key_first = ImGuiKey_NamedKey_BEGIN;
#else
        struct funcs
        {
            static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; }
        }; // Hide Native<>ImGuiKey duplicates when both exists in the array
#endif

// Defined Buttons
#define TAB 512
#define W 568
#define A 546
#define S 564
#define D 549
#define INSTRUCTIONS_PER_FRAME 100000

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static int init_application(char* glsl_version) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return EXIT_FAILURE;
    
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    strcpy(glsl_version, "#version 100");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    strcpy(glsl_version, "#version 150");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    strcpy(glsl_version, "#version 130");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    return 0;
}

int main(int argc, char **argv)
{
    // Get GLSL version
    char glsl_version[13];
    if (init_application(glsl_version))
        return EXIT_FAILURE;

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "t89-OS", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsClassic(); // Setup Dear ImGui style
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImVec4 clear_color = ImVec4(0.00f, 0.55f, 0.60f, 1.00f); // Background color

    if (argc != 3 && argc != 4) {
        std::cerr << "Invalid Arguments\n";
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
	int debug = 0;
	if (argc == 4)
		debug = (atoi(argv[3]) == 1) ? 1 : 0;

    // Initialize Emulator
    std::cout << "Hello\n";
    Pipeline t89(argv[1], argv[2], debug);
    
    // Emulator Setup
    std::vector<int> buttons = {TAB, W, A, S, D};
    static bool is_step_enabled = true;
    static bool is_run_enabled = false;

    // Initialize Texture ID for LCD Display
    GLuint textureID;
    glGenTextures(1, &textureID);

    // LCD Display showing vram
    float my_tex_w = 512;
    float my_tex_h = 288;
    const uint32_t* vram = t89.dram.video_memory;

    // Refresh rate (dependent on instructions executed)
    static int num_instructions = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        t89.next_instruction();
        if (num_instructions != INSTRUCTIONS_PER_FRAME) {
            num_instructions++;
            continue;
        }
        num_instructions = 0;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Control Module
        ImGui::Begin("Control Panel");
        if(is_step_enabled == true) {
            // Step only enabled for 1 cycle
            // t89.next_instruction();
            is_step_enabled = false;
            // std::cout << "Vram: " << vram[0] << "\n";
        }
        ImGui::Checkbox("Step", &is_step_enabled);
        ImGui::Checkbox("Run", &is_run_enabled);
        ImGui::End();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        // if (true)
        //     ImGui::ShowDemoWindow(&show_demo_window);

        // Register Module
        enum ContentsType
        {
            CT_Text,
            CT_FillButton
        };
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        static bool display_headers = true;
        static int contents_type = CT_Text;
        ImGui::Begin("Registers");
        if (ImGui::BeginTable("Registers", 2, flags))
        {
            // Display headers so we can inspect their interaction with borders.
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
            if (display_headers)
            {
                ImGui::TableSetupColumn("Register Name");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
            }

            for (size_t row = 0; row < t89::registers.size(); row++)
            {
                // Register Entry
                ImGui::TableNextRow();

                // Register Name
                ImGui::TableSetColumnIndex(0);
                char buf[32];
                sprintf(buf, "%s", t89::registers.at(row).first.c_str());
                if (contents_type == CT_Text)
                    ImGui::TextUnformatted(buf);
                else if (contents_type == CT_FillButton)
                    ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));

                // Register Value
                ImGui::TableSetColumnIndex(1);
                // registers.at(row).second = rand() % 4294967295;
                sprintf(buf, "0x%08X", t89.rf.read(row));
                if (contents_type == CT_Text)
                    ImGui::TextUnformatted(buf);
                else if (contents_type == CT_FillButton)
                    ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
            }
            ImGui::EndTable();
        }
        ImGui::End();

        // Keyboard State
        ImGui::Begin("External I/O");
        ImGui::Text("Button Pressed:");
        // Check for buttons pressed
        for (ImGuiKey const &key : buttons)
        {
            if (funcs::IsLegacyNativeDupe(key))
                continue;
            if (ImGui::IsKeyDown(key))
            {
                // ImGui::SameLine();
                ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key);
                break;
            }
        }
        ImGui::End();
        
        // VRAM Module
        ImGui::Begin("LCD Display");
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, my_tex_w, my_tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, vram);
        ImGui::Image((void*)(intptr_t)textureID, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
        ImGui::End();

        // Current Instruction (Disassembled Output Module)
        ImGui::Begin("Current Instruction");
        ImGui::Text("Here it is");
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}