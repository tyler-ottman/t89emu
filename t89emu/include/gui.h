#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <stdint.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "CPU.h"
#include "risc_elf.h"

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

class gui {
private:
    float my_tex_w;
    float my_tex_h;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    Pipeline* t89;
    ELF_Parse* elf_parser;
    uint8_t* vram;
    uint8_t* rom;
    uint8_t* ram;
    uint8_t* csr_mem;
    char* vga_text_buffer;
    RegisterFile* rf;
    std::unordered_map<uint32_t, std::string> disassembled_module;
    std::vector<struct Disassembled_Entry> disassembled_code;
    bool is_step_enabled;
    bool is_run_enabled;
    std::vector<uint32_t> breakpoints;
    uint32_t* pc_ptr;
    ImVec4 clear_color;
    ImFont* egaFont;
    int init_application(char*);
    void add_memory_section(uint32_t, uint32_t, uint8_t*, std::string);
    void load_disassembled_code();
    void load_decompiled_code();
public:
    gui(char*, int);
    void run_debug_application();
    void run_main_application();
    void render_register_bank();
    void render_lcd_display();
    void render_disassembled_code_section();
    void render_io_panel();
    void render_control_panel();
    void render_memory_viewer();
    void render_csr_bank();
    void render_frame();
    void render_fonts();
};

#endif // GUI_H