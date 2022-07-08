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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Components.h"
#include "pipeline.h"

// Defined Buttons
#define TAB 512
#define W 568
#define A 546
#define S 564
#define D 549

#define INSTRUCTIONS_PER_FRAME 1000000
#define DISASSEMBLER_IMPL_STRING_PARSE 1
#define DISASSMELBER_IMPL_HEX 1

class gui {
private:
    float my_tex_w;
    float my_tex_h;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    Pipeline* t89;
    uint32_t* vram;
    uint32_t* rom;
    uint32_t* ram;
    RegisterFile* rf;
    uint32_t* pc_ptr;
    ImVec4 clear_color;
    void add_memory_section(uint32_t, uint32_t, uint32_t*, std::string);
    void load_disassembled_code(char* path_name);
    std::unordered_map<uint32_t, std::string> disassembled_module;
    int num_disassembled_instructions = 0;
    bool is_step_enabled;
    bool is_run_enabled;
    std::vector<uint32_t> breakpoints;
public:
    // gui(uint32_t*, RegisterFile*, uint32_t*, uint32_t*, uint32_t*);
    gui(char*, char*, int);
    void run_debug_application();
    void render_register_bank();
    void render_lcd_display();
    void render_disassembled_code_section();
    void render_io_panel();
    void render_control_panel();
    void render_memory_viewer();
    void render_csr_bank();
    void render_frame();
};