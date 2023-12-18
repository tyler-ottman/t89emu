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
private:
    float myTexW;
    float myTexH;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    Cpu *t89;
    ElfParser *elfParser;
    uint8_t *vram;
    uint8_t *rom;
    uint8_t *ram;
    uint8_t *csrMem;
    char *vgaTextBuffer;
    RegisterFile *rf;
    std::unordered_map<uint32_t, std::string> disassembledModule;
    std::vector<struct DisassembledEntry> disassembledCode;
    bool isStepEnabled;
    bool isRunEnabled;
    std::vector<uint32_t> breakpoints;
    uint32_t *pcPtr;
    ImVec4 clearColor;
    ImFont *egaFont;
    int initApplication(char *);
    void addMemorySection(uint32_t, uint32_t, uint8_t *, std::string);
    void loadDisassembledCode();
    void loadDecompiledCode();
public:
    Gui(char *, int);
    void runDebugApplication();
    void runMainApplication();
    void renderRegisterBank();
    void renderLcdDisplay();
    void renderDisassembledCodeSection();
    void renderIoPanel();
    void renderControlPanel();
    void renderMemoryViewer();
    void renderCsrBank();
    void renderFrame();
    void renderFonts();
};

#endif // GUI_H