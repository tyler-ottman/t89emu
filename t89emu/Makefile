
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#
IMGUI_SRC = ./src/vendor
IMGUI_INC = ./include/vendor
TOP_LEVEL_SRC = ./src
TOP_LEVEL_INC = ./include

OBJ_DIR = ./obj

CXX = g++
CXXFLAGS = -std=c++11 -I$(IMGUI_INC) -I$(IMGUI_INC)/backends # Vendor Include
CXXFLAGS += -I$(TOP_LEVEL_INC) # Non-Vendor Include
CXXFLAGS += -g -Wall -Wformat -O3

TARGET_OBJS = $(OBJ_DIR)/main.o
TARGET_OBJS += $(OBJ_DIR)/AluControlUnit.o $(OBJ_DIR)/NextPc.o $(OBJ_DIR)/RegisterFile.o $(OBJ_DIR)/Alu.o $(OBJ_DIR)/ImmediateGenerator.o $(OBJ_DIR)/ProgramCounter.o $(OBJ_DIR)/Csr.o $(OBJ_DIR)/MemControlUnit.o $(OBJ_DIR)/MemoryDevice.o $(OBJ_DIR)/ClintMemoryDevice.o $(OBJ_DIR)/RamMemoryDevice.o $(OBJ_DIR)/RomMemoryDevice.o $(OBJ_DIR)/VideoMemoryDevice.o $(OBJ_DIR)/Bus.o $(OBJ_DIR)/Trap.o
TARGET_OBJS += $(OBJ_DIR)/imgui.o $(OBJ_DIR)/imgui_demo.o $(OBJ_DIR)/imgui_draw.o $(OBJ_DIR)/imgui_tables.o $(OBJ_DIR)/imgui_widgets.o
TARGET_OBJS += $(OBJ_DIR)/imgui_impl_glfw.o $(OBJ_DIR)/imgui_impl_opengl3.o
TARGET_OBJS += $(OBJ_DIR)/Cpu.o $(OBJ_DIR)/Gui.o $(OBJ_DIR)/ElfParser.o

TARGET_NAME = t89emu

UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL
LIBS =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
all: directories $(TARGET_NAME)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET_NAME): $(TARGET_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: $(IMGUI_SRC)/%.cpp # ImGui frontend object files
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(IMGUI_SRC)/backends/%.cpp # ImGui backend object files
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TOP_LEVEL_SRC)/%.cpp # main object file
	$(CXX) $(CXXFLAGS) -c -o $@ $<

directories: $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(TARGET_NAME) $(OBJ_DIR)
