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

#include "Cpu.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ElfParser.h"

// Defined Buttons
#define TAB 512
#define W 568
#define A 546
#define S 564
#define D 549

#define INSTRUCTIONS_PER_FRAME 1000000
#define DISASSEMBLER_IMPL_HEX
// #define CDECOMPILE

#ifndef GUI_H
#define GUI_H

class Gui {
public:
    Gui(char *elfFile, int debug);
    ~Gui();
    void runDebugApplication(void);
    void runMainApplication(void);

private:
    int initApplication(char *glslVersion);
    void addMemorySection(uint32_t memSize, uint32_t memStart, uint8_t *memPtr,
                          std::string memSecitonName);
    void loadDisassembledCode(void);
    void loadDecompiledCode(void);

    // Render Modules
    void renderControlPanel(void);
    void renderCsrBank(void);
    void renderDisassembledCodeSection(void);
    void renderFrame(void);
    void renderIoPanel(void);
    void renderLcdDisplay(void);
    void renderMemoryViewer(void);
    void renderRegisterBank(void);

    float myTexW;
    float myTexH;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    
    ElfParser *elfParser;
    Cpu *t89;
    
    // Device Probes
    uint8_t *vramProbe;
    uint8_t *romProbe;
    uint8_t *ramProbe;
    uint8_t *csrMemProbe;
    char *vgaTextProbe;
    RegisterFile *rfProbe;
    uint32_t *pcProbe;

    std::unordered_map<uint32_t, std::string> disassembledModule;
    std::vector<struct DisassembledEntry> disassembledCode;
    bool isStepEnabled;
    bool isRunEnabled;
    std::vector<uint32_t> breakpoints;
    
    ImVec4 clearColor;
    ImFont *egaFont;
};

#endif // GUI_H