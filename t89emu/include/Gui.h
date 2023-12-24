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
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "ElfParser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Mcu.h"

// Defined Buttons
#define TAB 512
#define W 568
#define A 546
#define S 564
#define D 549

#define INSTRUCTIONS_PER_FRAME 1000000

class Gui {
public:
    Gui(ElfParser *elfParser, int debug);
    ~Gui();
    void runApplication(void);

private:
    int initApplication(char *glslVersion);
    void addMemorySection(uint32_t memSize, uint32_t memStart, uint8_t *memPtr,
                          std::string memSecitonName);

    // Render Modules
    void renderControlPanel(void);
    void renderCsrBank(void);
    void renderDisassembledCodeSection(void);
    void renderFrame(void);
    void renderIoPanel(void);
    void renderLcdDisplay(void);
    void renderMemoryViewer(void);
    void renderRegisterBank(void);

    // Rendered modules
    ClintMemoryDevice *csrMemProbe;
    RamMemoryDevice *ramProbe;
    RomMemoryDevice *romProbe;
    VideoMemoryDevice *vramProbe;
    RegisterFile *rfProbe;
    ProgramCounter *pcProbe;
    Csr *csrProbe;

    float textureW;
    float textureH;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    
    // For Rendering Disassembled Code
    ElfParser *elfParser;

    bool isStepEnabled;
    bool isRunEnabled;
    std::vector<uint32_t> breakpoints;
    
    ImVec4 clearColor;
    ImFont *egaFont;
};

#endif // GUI_H