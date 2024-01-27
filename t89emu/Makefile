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

CXX = g++
CXXFLAGS = -std=c++11 -I$(IMGUI_INC) -I$(IMGUI_INC)/backends # Vendor Include
CXXFLAGS += -I$(TOP_LEVEL_INC) # Non-Vendor Include
CXXFLAGS += -g -Wall -Wformat -O3

CPPFILES := $(shell find . -type f -name '*.cpp')
override OBJ := $(CPPFILES:.cpp=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)

TARGET_NAME = t89emu

UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL
LIBS =

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
all: $(TARGET_NAME)

$(TARGET_NAME): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

-include $(HEADER_DEPS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(HEADER_DEPS)
