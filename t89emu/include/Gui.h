#ifndef GUI_H
#define GUI_H

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include <algorithm>
#include <cmath>
#include <fstream>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "DwarfParser.h"
#include "ElfParser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Mcu.h"

#define INSTRUCTIONS_PER_FRAME 1000000

class Gui {
public:
    Gui(ElfParser *elfParser, DwarfParser *dwarfParser, int debug);
    ~Gui();
    void runApplication(void);

private:
    int initImGuiInstance(void);
    void addMemorySection(uint32_t memSize, uint32_t memStart, uint8_t *memPtr,
                          std::string memSecitonName);

    // Render Modules
    void renderControlPanel(void);
    void renderDisassembledCodeSection(void);
    void renderFrame(void);
    void renderIoPanel(void);
    void renderLcdDisplay(void);
    void renderMemoryViewer(void);
    void renderRegisterBank(void);
    void renderDebugSource(void);

    void displayVarTable(const std::string &name);

    std::string getInstructionStr(struct DisassembledEntry & entry);

    // GLFW Window context
    GLFWwindow *window;

    // Rendered modules
    ClintMemoryDevice *csrMemProbe;
    RamMemoryDevice *ramProbe;
    RomMemoryDevice *romProbe;
    VideoMemoryDevice *vramProbe;
    RegisterFile *rfProbe;
    ProgramCounter *pcProbe;
    Csr *csrProbe;
    ImmediateGenerator *immgenProbe;

    // VRAM Module
    float textureW;
    float textureH;
    GLuint textureID;

    // I/O Panel
    std::vector<ImGuiKey> buttons;
    
    // General Purpose Registers
    std::vector<std::pair<std::string, uint32_t>> registers;
    
    // Disassembled Information
    ElfParser *elfParser;

    // Source-Level Information
    struct SourceFileInfo {
        std::string path;
        std::string name;
        std::vector<std::string> lines;
    };

    DwarfParser *dwarfParser;
    std::vector<SourceFileInfo *> sourceFiles;

    bool isStepEnabled;
    bool isRunEnabled;
    std::vector<uint32_t> breakpoints;
    
    ImFont *vramFont;
};

#endif // GUI_H