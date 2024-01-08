#include "Gui.h"

struct funcs {
    static bool IsLegacyNativeDupe(ImGuiKey key) {
        return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
    }
};  // Hide Native<>ImGuiKey duplicates when both exists in the array

Gui::Gui(ElfParser *elfParser, DwarfParser *dwarfParser, int debug)
    : elfParser(elfParser),
      dwarfParser(dwarfParser),
      isStepEnabled(false),
      isRunEnabled(false) {
    // Call vendor specific initializer code
    if (initImGuiInstance()) {
        exit(EXIT_FAILURE);
    }

    // GUI uses pointers to MCU modules for rendering
    Mcu *mcu = Mcu::getInstance();
    csrMemProbe = mcu->getClintDevice();
    ramProbe = mcu->getRamDevice();
    romProbe = mcu->getRomDevice();
    vramProbe = mcu->getVideoDevice();
    rfProbe = mcu->getRegisterFileModule();
    pcProbe = mcu->getProgramCounterModule();
    csrProbe = mcu->getCsrModule();
    immgenProbe = mcu->getImmediateGeneratorModule();
    
    textureW = vramProbe->getGWidth();
    textureH = vramProbe->getGHeight();

    // Initialize Register Viewer
    for (const std::string &regName : rfProbe->getNames()) {
        registers.push_back(std::make_pair(regName, 0));
    }

    // Initialize VRAM Viewer
    glGenTextures(1, &textureID);
    vramFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("./egaFont.ttf", 10.8);

    // Initialize I/O Panel Viewer
    buttons = {ImGuiKey_Tab, ImGuiKey_W, ImGuiKey_A, ImGuiKey_S, ImGuiKey_D};

    // Initialize Source Code Viewer
    for (size_t i = 0; i < dwarfParser->getNumCompileUnits(); i++) {
        CompileUnit *cu = dwarfParser->getCompileUnit(i);
        SourceFileInfo *sourceInfo = new SourceFileInfo;
        
        // Full path to source file
        std::stringstream stream;
        stream << cu->getUnitDir() << "/" << cu->getUnitName();
        sourceInfo->path = stream.str();

        // Source file name
        size_t found = sourceInfo->path.find_last_of("/");
        sourceInfo->name = sourceInfo->path.substr(++found);

        // Source file bytes
        std::ifstream fs;
        fs.open(sourceInfo->path, std::ifstream::in);
        if (!fs.is_open()) {
            printf("Could not open file: %s\n", sourceInfo->name.c_str());
            exit(EXIT_FAILURE);
        }

        std::string line;
        while (std::getline(fs, line)) {
            sourceInfo->lines.push_back(line);
        }

        fs.close();
        sourceFiles.push_back(sourceInfo);
    }
}

Gui::~Gui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Gui::runApplication() {
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        if (isRunEnabled) {
            uint n = INSTRUCTIONS_PER_FRAME;
            while (n-- > 0 && (std::find(breakpoints.begin(), breakpoints.end(),
                                     pcProbe->getPc()) == breakpoints.end())) {
                Mcu::getInstance()->nextInstruction();
            }
        }

        if (isStepEnabled) {
            Mcu::getInstance()->nextInstruction();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // bool show_demo_window = true;
        // ImGui::ShowDemoWindow(&show_demo_window);
        renderMemoryViewer();
        renderRegisterBank();
        renderIoPanel();
        renderLcdDisplay();
        renderDisassembledCodeSection();
        renderControlPanel();
        renderDebugSource();

        // Draw frame to screen
        renderFrame();
    }
}

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int Gui::initImGuiInstance() {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) { return EXIT_FAILURE; }
    
    // Decide GL+GLSL versions (GL 3.0 + GLSL 130)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only

    // Create window with graphics context
    window = glfwCreateWindow(1920, 1080, "t89emu", NULL, NULL);
    if (!window) { return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsClassic(); // Setup Dear ImGui style

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

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
    uint32_t pc = pcProbe->getPc();
    std::vector<DisassembledEntry> &disassembly = elfParser->getDisassembledCode();

    float scroll_pos = 0.0;
    if (isStepEnabled) {
        for (size_t idx = 0; idx < disassembly.size(); idx++) {
            struct DisassembledEntry &entry = disassembly[idx];
            if (entry.isInstruction && (entry.address == pc)) {
                scroll_pos = idx * TEXT_BASE_HEIGHT;
                break;
            }
        }
    }

    // Begin Table
    static ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;
    if (!ImGui::BeginTable("table1", 1, tableFlags)) {
        return;
    }

    for (auto &entry : disassembly) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImU32 bgColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.65f));

        if (entry.isInstruction) {
            if (std::find(breakpoints.begin(), breakpoints.end(),
                          entry.address) != breakpoints.end()) {
                // Breakpoint (red)             
                bgColor = ImGui::GetColorU32(ImVec4(1.0f, 0.5f, 0.5f, 0.25f));
            } else if (entry.address == pcProbe->getPc()) {
                // Current line (yellow)
                bgColor = ImGui::GetColorU32(
                    ImVec4(0.75f, 0.61f, 0.19f, 0.25f));
            }

            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0x00, 0xab, 0x41, 0xff));
            ImGui::Text("%08x:", entry.address);
        } else {  // Symbol Name (Yellow)
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0xff, 0xdf, 0x00, 0xff));
            ImGui::Text("%s", entry.line.c_str());
        }
        ImGui::PopStyleColor();

        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, bgColor);

        // Print Instruction
        if (entry.isInstruction) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  IM_COL32(0x00, 0x77, 0xb6, 255));
            ImGui::Text("%s", entry.line.c_str());
            ImGui::PopStyleColor();
        }

        // If stepping through code, auto scroll to current instruction
        if (isStepEnabled) {
            // ImGui::SetScrollY(scroll_pos);
            ImGui::SetScrollFromPosY(scroll_pos - ImGui::GetScrollY());
        }
    }

    // End Table
    ImGui::EndTable();

    ImGui::EndChild();
    ImGui::End();
}

void Gui::renderFrame() {
    int displayW, displayH;
    ImVec4 clearColor = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Background color

    ImGui::Render();
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
                 clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void Gui::renderIoPanel() {
    uint8_t *csrMemBuffer = csrMemProbe->getBuffer();
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
    uint32_t videoMode = *(vramProbe->getBuffer());

    char lineStr[vramProbe->getTWidth() + 1];  // Print line by line

    // VRAM Module
    ImGui::Begin("VRAM Module");
    if (videoMode == VGA_TEXT_MODE) {
        ImGui::PushFont(vramFont);

        char *vgaTextBuffer = (char *)(vramProbe->getBuffer() + 16);
        for (size_t i = 0; i < vramProbe->getTHeight(); i++) {
            memcpy(lineStr, vgaTextBuffer + vramProbe->getTWidth() * i,
                   vramProbe->getTWidth());
            for (size_t j = 0; j < vramProbe->getTWidth(); j++) {
                if (lineStr[j] == '\0') {
                    // Allow GUI to print entire line by replacing null byte
                    // with space
                    lineStr[j] = ' ';
                }
            }
            lineStr[vramProbe->getTWidth()] = '\0';
            ImGui::Text("%s", lineStr);
        }
        ImGui::PopFont();
    } else if (videoMode == GRAPHICS_MODE) {
        ImVec2 uvMin = ImVec2(0.0f, 0.0f);                  // Top-left
        ImVec2 uvMax = ImVec2(1.0f, 1.0f);                  // Lower-right
        ImVec4 tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);    // No tint
        ImVec4 borderCol = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);  // 50% opaque white

        uint8_t *vramBuffer =
            (uint8_t *)(vramProbe->getBuffer() + 16 +
                        vramProbe->getTWidth() * vramProbe->getTHeight());

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureW, textureH, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, vramBuffer);
        ImGui::Image((void *)(intptr_t)textureID, ImVec2(textureW, textureH),
                     uvMin, uvMax, tintCol, borderCol);
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

        if (ImGui::Button("SP"))  // Jump to SP
            jumpAddr = romProbe->getEndAddress() - romProbe->getBaseAddress();
        ImGui::SameLine();
        if (ImGui::Button("VRAM"))  // Jump to VRAM
            jumpAddr = romProbe->getEndAddress() - romProbe->getBaseAddress() +
                       ramProbe->getEndAddress() - ramProbe->getBaseAddress();

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

        addMemorySection(romProbe->getDeviceSize(), romProbe->getBaseAddress(),
                         romProbe->getBuffer(), "CODE");
        addMemorySection(ramProbe->getDeviceSize(), ramProbe->getBaseAddress(),
                         ramProbe->getBuffer(), "DATA");
        addMemorySection(vramProbe->getDeviceSize(),
                         vramProbe->getBaseAddress(), vramProbe->getBuffer(),
                         "VRAM");

        ImGui::EndTable();
    }
    
    ImGui::EndChild();
    ImGui::End();
}

// Render General Purpose and Control State Registers
void Gui::renderRegisterBank() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    std::vector<int> csrAddress = {CSR_MSTATUS, CSR_MISA,     CSR_MIE,
                                   CSR_MTVEC,   CSR_MSCRATCH, CSR_MEPC,
                                   CSR_MCAUSE,  CSR_MTVAL,    CSR_MIP};

    std::vector<std::string> csrName = {"mstatus", "misa",     "mie",
                                        "mtvec",   "mscratch", "mepc",
                                        "mcause",  "mtval",    "mip"};

    std::vector<std::string> csrMemName = {"mcycle_l", "mcycle_h", "mtimecmp_l",
                                           "mtimecmp_h", "keyboard"};

    ImGui::Begin("Registers");
    if (ImGui::BeginTable("Registers", 2, flags)) {
        ImGui::TableSetupColumn("Register Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        // General Purpose Registers
        for (size_t row = 0; row < registers.size(); row++) {
            // Register Entry
            ImGui::TableNextRow();

            // Register Name
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", registers[row].first.c_str());

            // Register Value
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%08X", rfProbe->read(row));
        }

        // Embedded CSRs
        for (size_t row = 0; row < csrAddress.size(); row++) {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", csrName.at(row).c_str());

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%08X", csrProbe->readCsr(csrAddress.at(row)));
        }

        // Memory Mapped CSRs
        uint8_t *csrMemBuffer = csrMemProbe->getBuffer();
        for (size_t row = 0; row < csrMemName.size(); row++) {
            // CSR Entry
            ImGui::TableNextRow();

            // CSR Name
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", csrMemName.at(row).c_str());

            // CSR Value
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%08X", *((uint32_t *)&csrMemBuffer[4 * row]));
        }

        ImGui::EndTable();
    }
    ImGui::End();
}

void Gui::renderDebugSource() {
    ImGuiTableFlags tableFlags;

    ImGui::Begin("Source Code Debugger"); // Begin Window    

    // Source Code Window
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("sourceCodeWindow",
                      ImVec2(windowSize.x * 0.75f, windowSize.y),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX, 0);

    // Source Code Window Tab Bar
    ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_FittingPolicyScroll;
    ImGui::BeginTabBar("sourceCodeTabBar", tabBarFlags);

    for (const SourceFileInfo *sourceFile : sourceFiles) {
        if(!ImGui::BeginTabItem(sourceFile->name.c_str())) { continue; }

        windowSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("sourceCodeLines", ImVec2(windowSize.x, windowSize.y),
                          ImGuiChildFlags_None, 0);

        // Source Code Lines
        tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX;
        if (!ImGui::BeginTable("sourceCodeLines", 1, tableFlags)) { continue; }
        for (size_t row = 0; row < sourceFile->lines.size(); row++) {
            ImGui::TableNextRow();

            ImU32 bgColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.65f));
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, bgColor);

            ImGui::TableSetColumnIndex(0);
            
            // Line Number
            ImGui::PushStyleColor(ImGuiCol_Text, 0xff41ab00);
            ImGui::Text("%4ld: ", row);
            ImGui::PopStyleColor();

            // Line Source
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, 0xffb67700);
            ImGui::Text("%s\n", sourceFile->lines[row].c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndTable(); // sourceCodeLines
        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar(); // sourceCodeTabBar
    ImGui::EndChild();  // sourceCodeWindow

    // Source Code Variables
    ImGui::SameLine();
    windowSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("sourceCodeVariables", ImVec2(windowSize.x, windowSize.y),
                      ImGuiChildFlags_Border, 0);

    // Local/Global Variables (placeholder)
    displayVarTable("Global Variables");
    displayVarTable("Local Variables");

    ImGui::EndChild();
    ImGui::End(); // Source Code Debugger    
}

void Gui::displayVarTable(const std::string &name) {
    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;

    if (!ImGui::BeginTable(name.c_str(), 1, tableFlags)) { return; }
    ImGui::TableSetupColumn(name.c_str());
    ImGui::TableHeadersRow();

    for (uint row = 0; row < 10; row++) {
        ImGui::TableNextRow();

        // Variable Information
        ImGui::PushStyleColor(ImGuiCol_Text, 0xff844b78);
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("var%d: ", row);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, 0xffd4d4d4);
        ImGui::SameLine();
        ImGui::Text("null");
        ImGui::PopStyleColor();
    }
    ImGui::TableNextRow();
    ImGui::EndTable();
}

// export PATH=/opt/riscv32/bin:$PATH