#include "gui.h"

struct funcs
{
    static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; }
}; // Hide Native<>ImGuiKey duplicates when both exists in the array

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int gui::init_application(char* glsl_version) {
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

    // Create window with graphics context
    window = glfwCreateWindow(1280, 720, "t89-OS", NULL, NULL);
    if (window == NULL)
        exit(EXIT_FAILURE);
    glfwMakeContextCurrent(window);
    return 0;
}

gui::gui(char* elf_file, int debug) {
    std::vector<std::string> register_names = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    for (const std::string &reg_name : register_names) {
        registers.push_back(std::make_pair(reg_name, 0));
    }
    
    // Get GLSL version
    char glsl_version[13];
    
    if (init_application(glsl_version))
        exit(EXIT_FAILURE);
    
    // glfwSwapInterval(1); // Enable vsync
    IMGUI_CHECKVERSION();    // Setup Dear ImGui context
    ImGui::CreateContext();
    
    ImGui::StyleColorsClassic(); // Setup Dear ImGui style
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Background color
    glGenTextures(1, &textureID); // Initialize Texture ID for VRAM
    
    buttons = {TAB, W, A, S, D};
    my_tex_w = SCREEN_WIDTH;
    my_tex_h = SCREEN_HEIGHT;

    is_step_enabled = false;
    is_run_enabled = false;

    
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    egaFont = io.Fonts->AddFontFromFileTTF("myFont.ttf", 10.8);
    // io.Fonts->AddFontFromFileTTF("myFont.ttf", 1.0);

    // Parse ELF file to load ROM/RAM, disassembler
    elf_parser = new ELF_Parse(elf_file);
    elf_parser->elf_flash_sections(); // Generated ROM image    
    elf_parser->generate_disassembled_text(); // Generated disassembled code for GUIs

    // Initialize Emulator
    t89 = new Pipeline(elf_parser->rom_start, ROM_SIZE, elf_parser->ram_start, RAM_SIZE, debug);

    // GUI needs pointers to emulator parts to probe values
    vram = (uint8_t*)(t89->bus->video_device->mem + 16 + VIDEO_TEXT_BUFFER_SIZE);
    vga_text_buffer = (char*)(t89->bus->video_device->mem + 16);
    rom = t89->bus->rom_device->mem;
    ram = t89->bus->ram_device->mem;
    csr_mem = t89->bus->csr_device->mem;
    pc_ptr = &t89->pc->PC;
    rf = t89->rf;

    // Set entry PC
    *pc_ptr = elf_parser->get_entry_pc();
    t89->nextpc->nextPC = *pc_ptr;
    printf("entry pc: %08x\n", t89->pc->PC);

    
    // Flash ROM
    uint8_t* elf_rom = elf_parser->get_rom_image();
    memcpy(rom, elf_rom, elf_parser->flash_image.size());

#ifdef DISASSEMBLER_IMPL_HEX
    load_disassembled_code();
#endif

    // C De-Compiler feature in future
#ifdef CDECOMPILE
    load_decompiled_code();
#endif

    if (debug) {
        run_debug_application();
    } else {
        run_main_application();
    }
}

void gui::load_disassembled_code() {
    disassembled_code = elf_parser->get_disassembled_code();
    // for (const auto &entry : disassembled_code) {
    //     printf("%s\n", entry.line.c_str());
    // }
}

void gui::load_decompiled_code() {
    
}

void gui::run_debug_application() {
    // Main loop
    std::cout << "Starting debug application" << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        if (is_run_enabled) {
            int num_instructions = 0;
            while ((num_instructions < INSTRUCTIONS_PER_FRAME) && (std::find(breakpoints.begin(), breakpoints.end(), *pc_ptr) == breakpoints.end())) {
                t89->next_instruction();
                num_instructions++;
            }
        }
        if (is_step_enabled) {
            t89->next_instruction();
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // bool show_demo_window = true;
        // ImGui::ShowDemoWindow(&show_demo_window);

        render_memory_viewer();
        render_register_bank();
        render_csr_bank();
        render_io_panel();
        render_lcd_display();
        render_disassembled_code_section();
        render_control_panel();
        // render_fonts();
        render_frame();
    }
}

void gui::run_main_application() {
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        // int num_instructions = 0;
        if (is_run_enabled) {
            int num_instructions = 0;
            while ((num_instructions < INSTRUCTIONS_PER_FRAME)) {
                t89->next_instruction();
                num_instructions++;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_io_panel();
        render_lcd_display();
        render_control_panel();
        
        render_frame();
    }
}

void gui::render_register_bank() {
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
            sprintf(buf, "0x%08X", rf->read(row));
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
            ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key);
            // Set Bit Flag in Keyboard CSR
            for (size_t i = 0; i < buttons.size(); i++) {
                if (key == buttons.at(i)) {
                    csr_mem[4] |= (1 << i); // i specifies what bit "key" maps to
                }
            }
            break;
        } else {
            csr_mem[4] = 0;
        }
    }
    ImGui::End();
}

void gui::render_lcd_display() {
    uint32_t video_mode = *((uint8_t*)t89->bus->video_device->mem);
    char line_str[TEXT_MODE_VERTICAL_LINES+1]; // Print line by line on window

    // VRAM Module
    ImGui::Begin("VRAM Module");
    if (video_mode == VGA_TEXT_MODE) {
        ImGui::PushFont(egaFont);
        for (int i = 0; i < TEXT_MODE_HORIZONTAL_LINES; i++) {
            memcpy(line_str, vga_text_buffer + TEXT_MODE_VERTICAL_LINES * i, TEXT_MODE_VERTICAL_LINES);
            for (int j = 0; j < TEXT_MODE_VERTICAL_LINES; j++) {
                if (line_str[j] == '\0') {
                    // Allow GUI to print entire line by replacing null byte with space
                    line_str[j] = ' ';
                }
            }
            line_str[TEXT_MODE_VERTICAL_LINES] = '\0';
            ImGui::Text("%s", line_str);
        }
        ImGui::PopFont();
    } else if (video_mode == GRAPHICS_MODE) {
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, my_tex_w, my_tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, vram);
        ImGui::Image((void *)(intptr_t)textureID, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
    }
  
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
    is_step_enabled = false;
    ImGui::Begin("Control Panel");
    if (is_step_enabled == true)
    {
        // Step only enabled for 1 cycle
        is_step_enabled = false;
        // std::cout << "Vram: " << vram[0] << "\n";
    }
    ImGui::Checkbox("Step", &is_step_enabled);
    
    ImGui::Checkbox("Run", &is_run_enabled);
    // std::cout << is_run_enabled << std::endl;
    ImGui::End();
}

// Based on https://github.com/ThomasRinsma/dromaius
void gui::render_memory_viewer() {
    
    static char hexBuf[9] = {0x00};
    int jumpAddr = -1;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::Begin("Memory viewer", nullptr);

    if (ImGui::BeginTable("jumps", 2, ImGuiTableFlags_BordersInnerV))
    {
        // Manually Typed Address
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(100);
        ImGui::InputTextWithHint("###jump to", "address", hexBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::SameLine();
        if (ImGui::Button("go to addr"))
            jumpAddr = (int)strtol(hexBuf, NULL, 16);

        // Memory Section Button
        ImGui::TableNextColumn();
        ImGui::Text("Jump to: ");
        
        ImGui::SameLine();
        if (ImGui::Button("PC"))
            jumpAddr = 0; // Jump to PC
        ImGui::SameLine();
        if (ImGui::Button("SP"))
            jumpAddr = (t89->bus->rom_end - t89->bus->rom_base); // Jump to SP
        ImGui::SameLine();
        if (ImGui::Button("VRAM"))
            jumpAddr = (t89->bus->rom_end - t89->bus->rom_base) + (t89->bus->ram_end - t89->bus->ram_base); // Jump to VRAM
        
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
        add_memory_section(t89->bus->rom_device->deviceSize, t89->bus->rom_device->baseAddress, rom, "CODE");
        add_memory_section(t89->bus->ram_device->deviceSize, t89->bus->ram_device->baseAddress, ram, "DATA");
        add_memory_section(VIDEO_SIZE, VIDEO_BASE, t89->bus->video_device->mem, "VRAM");

        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

void gui::add_memory_section(uint32_t mem_size, uint32_t mem_start, uint8_t* mem_ptr, std::string mem_section_name) {
    uint8_t hex_bytes[16];
    uint8_t ascii_bytes[16];
    uint32_t rows_in_mem_section = ceil(mem_size / 16.0); // 16 bytes per row
    ImGuiListClipper clipper;
    clipper.Begin(rows_in_mem_section);
    while (clipper.Step()) {
        for (int addr = clipper.DisplayStart; addr < clipper.DisplayEnd; addr++) {
            ImGui::TableNextRow();

            // Memory Section Name
            ImGui::TableNextColumn();
            ImGui::Text("%s", mem_section_name.c_str()); // change later?

            // 32-bit Memory Address
            ImGui::TableNextColumn();
            ImGui::Text("%08x", 16 * addr + mem_start);
            for (size_t i = 0; i < 16; i++) {
                hex_bytes[i] = ((16 * addr + i) >= mem_size) ? 0xff : mem_ptr[16 * addr + i];
                ascii_bytes[i] = (hex_bytes[i] >= 0x20 && hex_bytes[i] <= 0x7E) ? hex_bytes[i] : '.';
            }
            
            // Memory Section to Hex Bytes
            ImGui::TableNextColumn();
            ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                        hex_bytes[0], hex_bytes[1], hex_bytes[2], hex_bytes[3],
                        hex_bytes[4], hex_bytes[5], hex_bytes[6], hex_bytes[7],
                        hex_bytes[8], hex_bytes[9], hex_bytes[10], hex_bytes[11],
                        hex_bytes[12], hex_bytes[13], hex_bytes[14], hex_bytes[15]);

            // Memory Section to Readable ASCII characters
            ImGui::TableNextColumn();
            ImGui::Text("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                        ascii_bytes[0], ascii_bytes[1], ascii_bytes[2], ascii_bytes[3],
                        ascii_bytes[4], ascii_bytes[5], ascii_bytes[6], ascii_bytes[7],
                        ascii_bytes[8], ascii_bytes[9], ascii_bytes[10], ascii_bytes[11],
                        ascii_bytes[12], ascii_bytes[13], ascii_bytes[14], ascii_bytes[15]);
        }
    }
}

void gui::render_disassembled_code_section() {
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    ImGui::Begin("Disassembly");
    static char hexBuf[9] = {0x00};
    if (ImGui::BeginTable("jumps", 1, ImGuiTableFlags_BordersInnerV))
    {
        // Manually Typed Address
        ImGui::TableNextColumn();
        ImGui::Text("Breakpoint ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::InputTextWithHint("###jump to", "address", hexBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        uint32_t breakpoint_address = (uint32_t)strtol(hexBuf, NULL, 16);
        ImGui::SameLine();
        if (ImGui::Button("set")) {
            if (std::find(breakpoints.begin(), breakpoints.end(), breakpoint_address) == breakpoints.end()) {
                // Breakpoint at address not found, add it
                breakpoints.push_back(breakpoint_address);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("remove")) {
            for (auto i = breakpoints.begin(); i != breakpoints.end(); i++) {
                if (*i == breakpoint_address) {
                    breakpoints.erase(i);
                    break;
                }
            }
        }
        ImGui::EndTable();
    }
    ImVec2 child_size = ImVec2(0, 0);
    ImGui::BeginChild("##ScrollingRegion", child_size); //, false);
    
    // When stepping through code, disassembler should auto scroll to current executed instruction
    float scroll_pos = 0.0;
    if (is_step_enabled) {
        for (size_t idx = 0; idx < disassembled_code.size(); idx++) {
            struct Disassembled_Entry entry = disassembled_code.at(idx);
            if (entry.is_instruction && (entry.address == *pc_ptr)) {
                scroll_pos = (idx - 7.0) * TEXT_BASE_HEIGHT;
                break;
            }
        }
    }
    
    for (const auto &entry : disassembled_code) {
        char addr_str[64];
        sprintf(addr_str, "%08x", entry.address);
        
        std::string disassembled_line = addr_str;

        if (entry.is_instruction) {
            if (std::find(breakpoints.begin(), breakpoints.end(), entry.address) != breakpoints.end()) {
                // Red address to indicate breakpoint
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0x00, 0x00, 0xff));
            } else { // Green Address if no breakpoint
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0x00,0xab,0x41,255));
            }
            ImGui::Text("%s:", disassembled_line.c_str());         
        } else { // Golden Yellow Name
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0xdf, 0x00, 255));
            ImGui::Text("%s", entry.line.c_str());
        }
        ImGui::PopStyleColor();

        // Print Instruction
        if (entry.is_instruction) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0x00,0x77,0xb6,255));
            ImGui::Text("%s", entry.line.c_str());
            ImGui::PopStyleColor();
        }

        // // Emulator at current instruction, draw arrow
        if (entry.is_instruction && (*pc_ptr == entry.address)) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff,0x00,0x00,0xff));
            ImGui::Text("<--");
            ImGui::PopStyleColor();
        }

        // If stepping through code, auto scroll to current instruction
        if (is_step_enabled) {
            ImGui::SetScrollY(scroll_pos);
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

void gui::render_csr_bank() {
    // Register Module
    enum ContentsType
    {
        CT_Text,
        CT_FillButton
    };
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    static bool display_headers = true;

    std::vector<int> csr_address = {MSTATUS, MISA, MIE, MTVEC, MSCRATCH, MEPC, MCAUSE, MTVAL, MIP};
    std::vector<std::string> csr_name = {"mstatus", "misa", "mie", "mtvec", "mscratch", "mepc", "mcause", "mtval", "mip"};

    std::vector<std::string> csr_mem_name = {"mcycle_l", "mcycle_h", "mtimecmp_l", "mtimecmp_h", "keyboard"};

    ImGui::Begin("CSR");
    if (ImGui::BeginTable("CSRs", 2, flags))
    {
        // Display headers so we can inspect their interaction with borders.
        // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
        if (display_headers)
        {
            ImGui::TableSetupColumn("Register Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
        }

        char buf[32];
        for (size_t row = 0; row < csr_address.size(); row++)
        {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            sprintf(buf, "%s", csr_name.at(row).c_str());
            ImGui::TextUnformatted(buf);

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            sprintf(buf, "0x%08X", t89->csr->read_csr(csr_address.at(row)));
            ImGui::TextUnformatted(buf);
        }

        // Display Memory mapped control state registers
        for (size_t row = 0; row < csr_mem_name.size(); row++) {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            sprintf(buf, "%s", csr_mem_name.at(row).c_str());
            ImGui::TextUnformatted(buf);

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            uint32_t* csr_value = (uint32_t*)(csr_mem + 4 * row);
            sprintf(buf, "0x%08X", *csr_value);
            ImGui::TextUnformatted(buf);
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// export PATH=/opt/riscv32/bin:$PATH