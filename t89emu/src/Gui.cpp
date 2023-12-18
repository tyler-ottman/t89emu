#include "Gui.h"

struct funcs {
    static bool IsLegacyNativeDupe(ImGuiKey key) {
        return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
    }
};  // Hide Native<>ImGuiKey duplicates when both exists in the array

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Gui::Gui(char *elfFile, int debug) {
    std::vector<std::string> registerNames = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    for (const std::string &regName : registerNames) {
        registers.push_back(std::make_pair(regName, 0));
    }
    
    // Get GLSL version
    char glslVersion[13];
    
    if (initApplication(glslVersion))
        exit(EXIT_FAILURE);
    
    // glfwSwapInterval(1); // Enable vsync
    IMGUI_CHECKVERSION();    // Setup Dear ImGui context
    ImGui::CreateContext();
    
    ImGui::StyleColorsClassic(); // Setup Dear ImGui style
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);
    clearColor = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Background color
    glGenTextures(1, &textureID); // Initialize Texture ID for VRAM
    
    buttons = {TAB, W, A, S, D};
    myTexW = SCREEN_WIDTH;
    myTexH = SCREEN_HEIGHT;

    isStepEnabled = false;
    isRunEnabled = false;
    
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    egaFont = io.Fonts->AddFontFromFileTTF("./egaFont.ttf", 10.8);

    // Parse ELF file to load ROM/RAM, disassembler
    elfParser = new ElfParser(elfFile);

    // Initialize Emulator
    t89 = new Cpu(elfParser->getRomStart(), ROM_SIZE, elfParser->getRamStart(),
                  RAM_SIZE, debug);

    // Set entry PC
    t89->getProgramCounterModule()->setPc(elfParser->getEntryPc());
    t89->getNextPcModule()->setNextPc(elfParser->getEntryPc());
    
    // Flash ELF Loadable sections to ROM Device
    elfParser->flashRom(t89->getBusModule()->getRomMemoryDevice());

    if (debug) {
        runDebugApplication();
    } else {
        runMainApplication();
    }
}

Gui::~Gui() {

}

void Gui::runDebugApplication() {
    // Main loop
    std::cout << "Starting debug application" << std::endl;
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        if (isRunEnabled) {
            int numInstructions = 0;
            while ((numInstructions < INSTRUCTIONS_PER_FRAME) &&
                   (std::find(breakpoints.begin(), breakpoints.end(),
                    *(t89->getProgramCounterModule()->getPcPtr())) ==
                    breakpoints.end())) {
                t89->nextInstruction();
                numInstructions++;
            }
        }

        if (isStepEnabled) {
            t89->nextInstruction();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // bool show_demo_window = true;
        // ImGui::ShowDemoWindow(&show_demo_window);
        renderMemoryViewer();
        renderRegisterBank();
        renderCsrBank();
        renderIoPanel();
        renderLcdDisplay();
        renderDisassembledCodeSection();
        renderControlPanel();
        renderFrame();
    }
}

void Gui::runMainApplication() {
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        // int num_instructions = 0;
        if (isRunEnabled) {
            int numInstructions = 0;
            while ((numInstructions < INSTRUCTIONS_PER_FRAME)) {
                t89->nextInstruction();
                numInstructions++;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderIoPanel();
        renderLcdDisplay();
        renderControlPanel();
        renderFrame();
    }
}

int Gui::initApplication(char *glslVersion) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return EXIT_FAILURE;
    
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    strcpy(glslVersion, "#version 100");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    strcpy(glslVersion, "#version 150");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
    // GL 3.0 + GLSL 130
    strcpy(glslVersion, "#version 130");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    // Create window with graphics context
    window = glfwCreateWindow(1280, 720, "t89-OS", NULL, NULL);
    if (window == NULL) exit(EXIT_FAILURE);
    glfwMakeContextCurrent(window);
    return 0;
}

void Gui::addMemorySection(uint32_t memSize, uint32_t memStart, uint8_t *memPtr,
                           std::string memSectionName) {
    uint8_t hexBytes[16];
    uint8_t asciiBytes[16];
    uint32_t rowsInMemSection = ceil(memSize / 16.0);  // 16 bytes per row
    ImGuiListClipper clipper;
    clipper.Begin(rowsInMemSection);
    while (clipper.Step()) {
        for (int addr = clipper.DisplayStart; addr < clipper.DisplayEnd;
             addr++) {
            ImGui::TableNextRow();

            // Memory Section Name
            ImGui::TableNextColumn();
            ImGui::Text("%s", memSectionName.c_str());  // change later?

            // 32-bit Memory Address
            ImGui::TableNextColumn();
            ImGui::Text("%08x", 16 * addr + memStart);
            for (size_t i = 0; i < 16; i++) {
                hexBytes[i] =
                    ((16 * addr + i) >= memSize) ? 0xff : memPtr[16 * addr + i];
                asciiBytes[i] = (hexBytes[i] >= 0x20 && hexBytes[i] <= 0x7E)
                                    ? hexBytes[i]
                                    : '.';
            }

            // Memory Section to Hex Bytes
            ImGui::TableNextColumn();
            ImGui::Text(
                "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X",
                hexBytes[0], hexBytes[1], hexBytes[2], hexBytes[3], hexBytes[4],
                hexBytes[5], hexBytes[6], hexBytes[7], hexBytes[8], hexBytes[9],
                hexBytes[10], hexBytes[11], hexBytes[12], hexBytes[13],
                hexBytes[14], hexBytes[15]);

            // Memory Section to Readable ASCII characters
            ImGui::TableNextColumn();
            ImGui::Text("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", asciiBytes[0],
                        asciiBytes[1], asciiBytes[2], asciiBytes[3],
                        asciiBytes[4], asciiBytes[5], asciiBytes[6],
                        asciiBytes[7], asciiBytes[8], asciiBytes[9],
                        asciiBytes[10], asciiBytes[11], asciiBytes[12],
                        asciiBytes[13], asciiBytes[14], asciiBytes[15]);
        }
    }
}

void Gui::renderControlPanel() {
    isStepEnabled = false;
    ImGui::Begin("Control Panel");
    if (isStepEnabled == true) {
        // Step only enabled for 1 cycle
        isStepEnabled = false;
        // std::cout << "Vram: " << vram[0] << "\n";
    }
    ImGui::Checkbox("Step", &isStepEnabled);
    
    ImGui::Checkbox("Run", &isRunEnabled);
    // std::cout << is_run_enabled << std::endl;
    ImGui::End();
}

// Register Module
void Gui::renderCsrBank() {
    enum ContentsType { 
        CT_Text, CT_FillButton
    };

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    static bool displayHeaders = true;

    std::vector<int> csrAddress = {MSTATUS, MISA,   MIE,   MTVEC, MSCRATCH,
                                   MEPC,    MCAUSE, MTVAL, MIP};
    std::vector<std::string> csrName = {"mstatus", "misa",     "mie",
                                        "mtvec",   "mscratch", "mepc",
                                        "mcause",  "mtval",    "mip"};

    std::vector<std::string> csrMemName = {"mcycle_l", "mcycle_h", "mtimecmp_l",
                                           "mtimecmp_h", "keyboard"};

    uint8_t *csrMemBuffer = t89->getBusModule()->getClintDevice()->getBuffer();

    ImGui::Begin("CSR");
    if (ImGui::BeginTable("CSRs", 2, flags)) {
        // Display headers so we can inspect their interaction with borders.
        // (Headers are not the main purpose of this section of the demo, so we
        // are not elaborating on them too much. See other sections for details)
        if (displayHeaders) {
            ImGui::TableSetupColumn("Register Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
        }

        char buf[32];
        for (size_t row = 0; row < csrAddress.size(); row++) {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            sprintf(buf, "%s", csrName.at(row).c_str());
            ImGui::TextUnformatted(buf);

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            sprintf(buf, "0x%08X",
                    t89->getCsrModule()->readCsr(csrAddress.at(row)));
            ImGui::TextUnformatted(buf);
        }

        // Display Memory mapped control state registers
        for (size_t row = 0; row < csrMemName.size(); row++) {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            sprintf(buf, "%s", csrMemName.at(row).c_str());
            ImGui::TextUnformatted(buf);

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            uint32_t *csrValue = (uint32_t *)&csrMemBuffer[4 * row];
            sprintf(buf, "0x%08X", *csrValue);
            ImGui::TextUnformatted(buf);
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void Gui::renderDisassembledCodeSection() {
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    ImGui::Begin("Disassembly");
    static char hexBuf[9] = {0x00};
    if (ImGui::BeginTable("jumps", 1, ImGuiTableFlags_BordersInnerV)) {
        // Manually Typed Address
        ImGui::TableNextColumn();
        ImGui::Text("Breakpoint ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::InputTextWithHint("###jump to", "address", hexBuf, 9,
                                 ImGuiInputTextFlags_CharsHexadecimal |
                                     ImGuiInputTextFlags_CharsUppercase);
        uint32_t breakpointAddress = (uint32_t)strtol(hexBuf, NULL, 16);
        ImGui::SameLine();
        if (ImGui::Button("set")) {
            if (std::find(breakpoints.begin(), breakpoints.end(),
                          breakpointAddress) == breakpoints.end()) {
                // Breakpoint at address not found, add it
                breakpoints.push_back(breakpointAddress);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("remove")) {
            for (auto i = breakpoints.begin(); i != breakpoints.end(); i++) {
                if (*i == breakpointAddress) {
                    breakpoints.erase(i);
                    break;
                }
            }
        }
        ImGui::EndTable();
    }
    ImVec2 child_size = ImVec2(0, 0);
    ImGui::BeginChild("##ScrollingRegion", child_size);  //, false);

    // When stepping through code, disassembler should auto scroll to current
    // executed instruction
    uint32_t *pcPtr = t89->getProgramCounterModule()->getPcPtr();
    std::vector<struct DisassembledEntry> &disassembledCode =
        elfParser->getDisassembledCode();

    float scroll_pos = 0.0;
    if (isStepEnabled) {
        for (size_t idx = 0; idx < disassembledCode.size(); idx++) {
            struct DisassembledEntry entry = disassembledCode.at(idx);
            if (entry.isInstruction && (entry.address == *pcPtr)) {
                scroll_pos = (idx - 7.0) * TEXT_BASE_HEIGHT;
                break;
            }
        }
    }

    for (const auto &entry : disassembledCode) {
        char addr_str[64];
        sprintf(addr_str, "%08x", entry.address);

        std::string disassembledLine = addr_str;

        if (entry.isInstruction) {
            if (std::find(breakpoints.begin(), breakpoints.end(),
                          entry.address) != breakpoints.end()) {
                // Red address to indicate breakpoint
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      IM_COL32(0xff, 0x00, 0x00, 0xff));
            } else {  // Green Address if no breakpoint
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      IM_COL32(0x00, 0xab, 0x41, 255));
            }
            ImGui::Text("%s:", disassembledLine.c_str());
        } else {  // Golden Yellow Name
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0xff, 0xdf, 0x00, 255));
            ImGui::Text("%s", entry.line.c_str());
        }
        ImGui::PopStyleColor();

        // Print Instruction
        if (entry.isInstruction) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0x00, 0x77, 0xb6, 255));
            ImGui::Text("%s", entry.line.c_str());
            ImGui::PopStyleColor();
        }

        // // Emulator at current instruction, draw arrow
        if (entry.isInstruction && (*pcPtr == entry.address)) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0xff, 0x00, 0x00, 0xff));
            ImGui::Text("<--");
            ImGui::PopStyleColor();
        }

        // If stepping through code, auto scroll to current instruction
        if (isStepEnabled) {
            ImGui::SetScrollY(scroll_pos);
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

void Gui::renderFrame() {
    ImGui::Render();
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
                 clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void Gui::renderIoPanel() {
    // Keyboard State
    uint8_t *csrMemBuffer = t89->getBusModule()->getClintDevice()->getBuffer();
    uint32_t *keyboardCsr = (uint32_t *)&csrMemBuffer[16];

    ImGui::Begin("External I/O");
    ImGui::Text("Button Pressed:");
    // Check for buttons pressed
    for (ImGuiKey const &key : buttons) {
        if (funcs::IsLegacyNativeDupe(key)) continue;
        if (ImGui::IsKeyDown(key)) {
            ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key);
            // Set Bit Flag in Keyboard CSR
            for (size_t i = 0; i < buttons.size(); i++) {
                if (key == buttons.at(i)) {
                    // i specifies what bit "key" maps to
                    *keyboardCsr = (1 << i);
                }
            }
            break;
        } else {
            *keyboardCsr = 0;
        }
    }
    ImGui::End();
}

void Gui::renderLcdDisplay() {
    uint32_t videoMode =
        *((uint8_t *)t89->getBusModule()->getVideoDevice()->getBuffer());
    char lineStr[TEXT_MODE_VERTICAL_LINES + 1];  // Print line by line

    // VRAM Module
    ImGui::Begin("VRAM Module");
    if (videoMode == VGA_TEXT_MODE) {
        ImGui::PushFont(egaFont);
        char *vgaTextBuffer =
            (char *)(t89->getBusModule()->getVideoDevice()->getBuffer() + 16);
        for (int i = 0; i < TEXT_MODE_HORIZONTAL_LINES; i++) {
            memcpy(lineStr, vgaTextBuffer + TEXT_MODE_VERTICAL_LINES * i,
                   TEXT_MODE_VERTICAL_LINES);
            for (int j = 0; j < TEXT_MODE_VERTICAL_LINES; j++) {
                if (lineStr[j] == '\0') {
                    // Allow GUI to print entire line by replacing null byte
                    // with space
                    lineStr[j] = ' ';
                }
            }
            lineStr[TEXT_MODE_VERTICAL_LINES] = '\0';
            ImGui::Text("%s", lineStr);
        }
        ImGui::PopFont();
    } else if (videoMode == GRAPHICS_MODE) {
        ImVec2 uvMin = ImVec2(0.0f, 0.0f);                  // Top-left
        ImVec2 uvMax = ImVec2(1.0f, 1.0f);                  // Lower-right
        ImVec4 tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);    // No tint
        ImVec4 borderCol = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);  // 50% opaque white

        uint8_t *vramBuffer =
            (uint8_t *)(t89->getBusModule()->getVideoDevice()->getBuffer() +
                        16 + VIDEO_TEXT_BUFFER_SIZE);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, myTexW, myTexH, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, vramBuffer);
        ImGui::Image((void *)(intptr_t)textureID, ImVec2(myTexW, myTexH), uvMin,
                     uvMax, tintCol, borderCol);
    }

    ImGui::End();
}

// Based on https://github.com/ThomasRinsma/dromaius
void Gui::renderMemoryViewer() {
    static char hexBuf[9] = {0x00};
    int jumpAddr = -1;

    // const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::Begin("Memory viewer", nullptr);
    if (ImGui::BeginTable("jumps", 2, ImGuiTableFlags_BordersInnerV)) {
        // Manually Typed Address
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(100);
        ImGui::InputTextWithHint("###jump to", "address", hexBuf, 9,
                                 ImGuiInputTextFlags_CharsHexadecimal |
                                     ImGuiInputTextFlags_CharsUppercase);
        ImGui::SameLine();
        if (ImGui::Button("go to addr"))
            jumpAddr = (int)strtol(hexBuf, NULL, 16);

        // Memory Section Button
        ImGui::TableNextColumn();
        ImGui::Text("Jump to: ");

        ImGui::SameLine();
        if (ImGui::Button("PC")) jumpAddr = 0;  // Jump to PC
        ImGui::SameLine();
        if (ImGui::Button("SP"))
            jumpAddr = (t89->getBusModule()->getRomEnd() -
                        t89->getBusModule()->getRomBase());  // Jump to SP
        ImGui::SameLine();
        if (ImGui::Button("VRAM"))
            jumpAddr = (t89->getBusModule()->getRomEnd() -
                        t89->getBusModule()->getRomBase()) +
                       (t89->getBusModule()->getRamEnd() -
                        t89->getBusModule()->getRamBase());  // Jump to VRAM

        ImGui::EndTable();
    }

    ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("headers", 4, flags)) {
        ImGui::TableSetupColumn("Region", 0, 60);
        ImGui::TableSetupColumn("Addr", 0, 60);
        ImGui::TableSetupColumn("", 0, 330);
        ImGui::TableSetupColumn("", 0, 150);
        ImGui::TableHeadersRow();
        ImGui::EndTable();
    }

    ImVec2 childSize = ImVec2(0, 0);
    ImGui::BeginChild("##ScrollingRegion", childSize);  //, false);

    // Jump to memory section if clicked
    if (jumpAddr != -1) ImGui::SetScrollY((jumpAddr >> 4) * TEXT_BASE_HEIGHT);

    if (ImGui::BeginTable("mem", 4, flags)) {
        ImGui::TableSetupColumn("1", 0, 60);
        ImGui::TableSetupColumn("2", 0, 60);
        ImGui::TableSetupColumn("3", 0, 330);
        ImGui::TableSetupColumn("4", 0, 150);
        addMemorySection(
            t89->getBusModule()->getRomMemoryDevice()->getDeviceSize(),
            t89->getBusModule()->getRomMemoryDevice()->getBaseAddress(),
            t89->getBusModule()->getRomMemoryDevice()->getBuffer(), "CODE");
        addMemorySection(
            t89->getBusModule()->getRamMemoryDevice()->getDeviceSize(),
            t89->getBusModule()->getRamMemoryDevice()->getBaseAddress(),
            t89->getBusModule()->getRamMemoryDevice()->getBuffer(), "DATA");
        addMemorySection(VIDEO_SIZE, VIDEO_BASE,
                         t89->getBusModule()->getVideoDevice()->getBuffer(),
                         "VRAM");

        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

void Gui::renderRegisterBank() {
    // Register Module
    enum ContentsType {
        CT_Text,
        CT_FillButton
    };

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    static bool display_headers = true;
    static int contents_type = CT_Text;
    ImGui::Begin("Registers");
    if (ImGui::BeginTable("Registers", 2, flags)) {
        // Display headers so we can inspect their interaction with borders.
        // (Headers are not the main purpose of this section of the demo, so we
        // are not elaborating on them too much. See other sections for details)
        if (display_headers) {
            ImGui::TableSetupColumn("Register Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
        }

        for (size_t row = 0; row < registers.size(); row++) {
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
            sprintf(buf, "0x%08X", t89->getRegisterFileModule()->read(row));
            if (contents_type == CT_Text)
                ImGui::TextUnformatted(buf);
            else if (contents_type == CT_FillButton)
                ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// export PATH=/opt/riscv32/bin:$PATH