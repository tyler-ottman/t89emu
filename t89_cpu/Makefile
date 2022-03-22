OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ../t89_cpu_components/include
LIB_DIR = ../t89_cpu_components/src

CFLAGS += -Wall -lgtest -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -g
CXX = g++
INCLUDE += -I $(INC_DIR)

TARGET_NAME = t89_cpu
TARGET_OBJS = $(OBJ_DIR)/t89_cpu.o $(OBJ_DIR)/ALUControlUnit.o $(OBJ_DIR)/ControlUnit.o $(OBJ_DIR)/NextPC.o $(OBJ_DIR)/RegisterFile.o $(OBJ_DIR)/ALU.o $(OBJ_DIR)/ImmediateGenerator.o $(OBJ_DIR)/ProgramCounter.o $(OBJ_DIR)/Memory.o $(OBJ_DIR)/CSR.o

all: directories $(TARGET_NAME)

$(TARGET_NAME): $(TARGET_OBJS)
	$(CXX) $(TARGET_OBJS) -o $(TARGET_NAME) $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

directories: $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(TARGET_NAME)