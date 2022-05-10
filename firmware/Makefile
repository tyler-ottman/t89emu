RISCPREFIX = riscv32-unknown-elf-
CC = $(RISCPREFIX)gcc
OBJCOPY = $(RISCPREFIX)objcopy

# INC_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

# LDFILE = t89-cpu.ld
# INCLUDE += -I $(INC_DIR)
CFLAGS += -g -nostdlib

TARGET_NAME = testerJAL

TARGET_OBJS = $(OBJ_DIR)/$(TARGET_NAME).o

all: directories $(BIN_DIR)/$(TARGET_NAME)

$(BIN_DIR)/$(TARGET_NAME): $(TARGET_OBJS)
	$(CC) $(TARGET_OBJS) -o $(BIN_DIR)/$(TARGET_NAME) $(CFLAGS)
	$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET_NAME) $(BIN_DIR)/$(TARGET_NAME).bin

$(OBJ_DIR)/$(TARGET_NAME).o: $(SRC_DIR)/$(TARGET_NAME).s
	$(CC) $(CFLAGS) -c $(SRC_DIR)/$(TARGET_NAME).s -o $(TARGET_OBJS)

directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

# riscv32-unknown-elf-objdump -d tester -M no-aliases
