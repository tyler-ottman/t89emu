OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ./include

CFLAGS += -Wall -lgtest
CXX = g++
INCLUDE  += -I $(INC_DIR)

TARGET_NAME = tester
TARGET_OBJS = $(OBJ_DIR)/ALUControlUnit.o $(OBJ_DIR)/ControlUnit.o $(OBJ_DIR)/NextPC.o $(OBJ_DIR)/RegisterFile.o $(OBJ_DIR)/ALU.o $(OBJ_DIR)/ImmediateGenerator.o $(OBJ_DIR)/ProgramCounter.o $(OBJ_DIR)/CSR.o $(OBJ_DIR)/Memory.o $(OBJ_DIR)/Tester.o

all: directories $(TARGET_NAME)

$(TARGET_NAME): $(TARGET_OBJS)
	$(CXX) $(TARGET_OBJS) -o $(TARGET_NAME) $(CFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

directories: $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf tester