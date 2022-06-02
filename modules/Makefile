OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib

CFLAGS = -Wall -O3
CXX = g++
INCLUDE = -I $(INC_DIR)

TARGET_NAME = $(LIB_DIR)/modules.a
TARGET_OBJS = $(OBJ_DIR)/ALUControlUnit.o $(OBJ_DIR)/ControlUnit.o $(OBJ_DIR)/NextPC.o $(OBJ_DIR)/RegisterFile.o $(OBJ_DIR)/ALU.o $(OBJ_DIR)/ImmediateGenerator.o $(OBJ_DIR)/ProgramCounter.o $(OBJ_DIR)/CSR.o $(OBJ_DIR)/Memory.o $(OBJ_DIR)/MemControlUnit.o

all: directories $(TARGET_NAME)

tester: $(SRC_DIR)/Tester.cpp $(LIB_DIR)/modules.a
	$(CXX) $(SRC_DIR)/Tester.cpp $(CFLAGS) -lgtest -pthread $(INCLUDE) $(LIB_DIR)/modules.a -o tester

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(TARGET_NAME): $(TARGET_OBJS)
	ar rv $(TARGET_NAME) $(TARGET_OBJS)
	ranlib $(TARGET_NAME)

directories: $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(LIB_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(LIB_DIR)
	rm -rf tester
