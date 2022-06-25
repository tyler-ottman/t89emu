#include "gui.h"

struct funcs
{
    static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; }
}; // Hide Native<>ImGuiKey duplicates when both exists in the array

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

gui::gui(uint32_t* video_mem_pointer) {
    std::vector<std::string> register_names = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    for (const std::string &reg_name : register_names) {
        registers.push_back(std::make_pair(reg_name, 0));
    }
    
    // Initialize Texture ID for LCD Display
    // Get GLSL version
    char glsl_version[13];
    if (init_application(glsl_version))
        exit(EXIT_FAILURE);
    
    // Create window with graphics context
    window = glfwCreateWindow(1280, 720, "t89-OS", NULL, NULL);
    if (window == NULL)
        exit(EXIT_FAILURE);
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
    clear_color = ImVec4(0.00f, 0.55f, 0.60f, 1.00f); // Background color
    glGenTextures(1, &textureID);

    vram = video_mem_pointer;
    buttons = {TAB, W, A, S, D};
    my_tex_w = SCREEN_WIDTH;
    my_tex_h = SCREEN_HEIGHT;

    is_step_enabled = true;
    is_run_enabled = false;
}

GLFWwindow* gui::get_window() {
    return window;
}

void gui::render_register_bank(RegisterFile& rf) {
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

        for (size_t row = 0; row < registers.size(); row++)
        {
            // Register Entry
            ImGui::TableNextRow();

            // Register Name
            ImGui::TableSetColumnIndex(0);
            char buf[32];
            sprintf(buf, "%s", registers.at(row).first.c_str());
            if (contents_type == CT_Text)
                ImGui::TextUnformatted(buf);
            else if (contents_type == CT_FillButton)
                ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));

            // Register Value
            ImGui::TableSetColumnIndex(1);
            // registers.at(row).second = rand() % 4294967295;
            sprintf(buf, "0x%08X", rf.read(row));
            if (contents_type == CT_Text)
                ImGui::TextUnformatted(buf);
            else if (contents_type == CT_FillButton)
                ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void gui::render_io_panel() {
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
}

void gui::render_lcd_display() {
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
    ImGui::Image((void *)(intptr_t)textureID, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
    ImGui::End();

    // Current Instruction (Disassembled Output Module)
    ImGui::Begin("Current Instruction");
    ImGui::Text("Here it is");
    ImGui::End();
}

void gui::render_frame() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void gui::render_control_panel() {
    ImGui::Begin("Control Panel");
    if (is_step_enabled == true)
    {
        // Step only enabled for 1 cycle
        // t89.next_instruction();
        is_step_enabled = false;
        // std::cout << "Vram: " << vram[0] << "\n";
    }
    ImGui::Checkbox("Step", &is_step_enabled);
    ImGui::Checkbox("Run", &is_run_enabled);
    ImGui::End();
}

// Based on https://github.com/ThomasRinsma/dromaius
void gui::render_memory_viewer() {
    uint16_t lineBuffer[16];
    uint8_t charBuffer[16];
    static char hexBuf[9] = {0x00};
    int jumpAddr = -1;

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::Begin("Memory viewer", nullptr);

    if (ImGui::BeginTable("jumps", 2, ImGuiTableFlags_BordersInnerV))
    {
        // Manually Typed Address
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(100);
        ImGui::InputTextWithHint("###jump to", "address", hexBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::SameLine();
        if (ImGui::Button("<- jump to addr"))
            jumpAddr = (int)strtol(hexBuf, NULL, 16);

        // Memory Section Button
        ImGui::TableNextColumn();
        ImGui::Text("Jump to: ");
        ImGui::SameLine();
        if (ImGui::Button("SP"))
            jumpAddr = 0; // Jump to SP
        ImGui::SameLine();
        if (ImGui::Button("PC"))
            jumpAddr = 0x00000000; // Jump to PC
        ImGui::SameLine();
        if (ImGui::Button("VRAM"))
            jumpAddr = 0x8000; // Jump to VRAM
        ImGui::EndTable();
    }

    ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("headers", 4, flags))
    {
        ImGui::TableSetupColumn("Region", 0, 60);
        ImGui::TableSetupColumn("Addr", 0, 60);
        ImGui::TableSetupColumn("", 0, 330);
        ImGui::TableSetupColumn("", 0, 150);
        ImGui::TableHeadersRow();
        ImGui::EndTable();
    }

    ImVec2 child_size = ImVec2(0, 0);
    ImGui::BeginChild("##ScrollingRegion", child_size); //, false);

    // Jump to memory section if clicked
    if (jumpAddr != -1)
        ImGui::SetScrollY((jumpAddr >> 4) * TEXT_BASE_HEIGHT);

    if (ImGui::BeginTable("mem", 4, flags))
    {
        ImGui::TableSetupColumn("1", 0, 60);
        ImGui::TableSetupColumn("2", 0, 60);
        ImGui::TableSetupColumn("3", 0, 330);
        ImGui::TableSetupColumn("4", 0, 150);

        uint32_t mem_section_len = 1048576; // 128 KB of instruction memory
        ImGuiListClipper clipper;           //(endAddr - startAddr, ImGui::GetTextLineHeight());
        clipper.Begin(mem_section_len >> 4);
        while (clipper.Step())
        {
            for (int addr = clipper.DisplayStart; addr < clipper.DisplayEnd; ++addr)
            {
                ImGui::TableNextRow();

                // Memory Section Name
                ImGui::TableNextColumn();
                ImGui::Text("CODE"); // CHANGE LATER

                // 32-bit Memory Address
                ImGui::TableNextColumn();
                ImGui::Text("%07X0", addr); // CHANGE LATER

                for (int i = 0; i < 16; ++i)
                {
                    // lineBuffer[i] = emu->memory.readByte((addr << 4) + i); COMMENTED HERE
                    // lineBuffer[i] = rand() % 0xff;
                    lineBuffer[i] = i + 0x20;
                    charBuffer[i] = (lineBuffer[i] >= 0x20 && lineBuffer[i] <= 0x7E) ? lineBuffer[i] : '.';
                }

                // Memory Section to Hex Bytes
                ImGui::TableNextColumn();
                ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                            lineBuffer[0], lineBuffer[1], lineBuffer[2], lineBuffer[3],
                            lineBuffer[4], lineBuffer[5], lineBuffer[6], lineBuffer[7],
                            lineBuffer[8], lineBuffer[9], lineBuffer[10], lineBuffer[11],
                            lineBuffer[12], lineBuffer[13], lineBuffer[14], lineBuffer[15]);

                // Memory Section to Readable ASCII characters
                ImGui::TableNextColumn();
                ImGui::Text("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                            charBuffer[0], charBuffer[1], charBuffer[2], charBuffer[3],
                            charBuffer[4], charBuffer[5], charBuffer[6], charBuffer[7],
                            charBuffer[8], charBuffer[9], charBuffer[10], charBuffer[11],
                            charBuffer[12], charBuffer[13], charBuffer[14], charBuffer[15]);
            }
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

void gui::render_disassembled_code_section() {}