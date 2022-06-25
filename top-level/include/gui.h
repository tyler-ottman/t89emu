#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <stdint.h>
#include <iostream>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
// #include "register_information.h"
#include "Components.h"

// Defined Buttons
#define TAB 512
#define W 568
#define A 546
#define S 564
#define D 549

#define INSTRUCTIONS_PER_FRAME 1000000

class gui {
private:
    float my_tex_w;
    float my_tex_h;
    GLuint textureID;
    GLFWwindow *window;
    std::vector<int> buttons;
    std::vector<std::pair<std::string, uint32_t>> registers;
    uint32_t* vram;
    ImVec4 clear_color;
    bool is_step_enabled;
    bool is_run_enabled;
public:
    
    gui(uint32_t*);
    GLFWwindow* get_window();
    void render_register_bank(RegisterFile&);
    void render_lcd_display();
    void render_disassembled_code_section();
    void render_io_panel();
    void render_control_panel();
    void render_memory_viewer();
    void render_frame();
};