OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib

CFLAGS = -Wall -std=c++17 -g
# LDFLAGS += -L../modules/lib -lmodules.a -lX11 -lGL -lpthread -lpng -lstdc++fs
LDFLAGS = ../modules/lib/modules.a -lX11 -lGL -lpthread -lpng -lstdc++fs

CXX = g++
INCLUDE += -I $(INC_DIR) -I../modules/include

TARGET_NAME = $(LIB_DIR)/cpulib.a
TARGET_OBJS = $(OBJ_DIR)/CPU.o

all: directories $(TARGET_NAME)

emulate: $(SRC_DIR)/emulate.cpp $(LIB_DIR)/cpulib.a
	$(CXX) $(SRC_DIR)/emulate.cpp $(CFLAGS) $(INCLUDE) $(LIB_DIR)/cpulib.a $(LDFLAGS) -o emulate

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(TARGET_NAME): $(TARGET_OBJS)
	ar rv $(TARGET_NAME) $(TARGET_OBJS)
	ranlib $(TARGET_NAME)

directories: $(OBJ_DIR) $(LIB_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(TARGET_NAME)
	rm -rf $(LIB_DIR)
	rm -rf ./emulate
