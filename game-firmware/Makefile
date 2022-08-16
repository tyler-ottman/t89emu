CXX = riscv32-unknown-elf-gcc
OBJCOPY = riscv32-unknown-elf-objcopy
OBJDUMP = riscv32-unknown-elf-objdump

OBJ_DIR = ./obj
SRC_DIR = ./src
BIN_DIR = ./bin
INC_DIR = ./include

LDFILE = ./t89-linker.ld
INCLUDE = -I $(INC_DIR)
CFLAGS = -ffreestanding
LDFLAGS = -T $(LDFILE) -nostartfiles
OBJCOPY_FLAGS = -O binary

TARGET_NAME = game
TARGET_OBJS = $(OBJ_DIR)/crt0.o $(OBJ_DIR)/bootup.o $(OBJ_DIR)/interrupt.o $(OBJ_DIR)/game.o

all: directories $(BIN_DIR)/$(TARGET_NAME)

$(BIN_DIR)/$(TARGET_NAME): $(TARGET_OBJS)
	$(CXX) $(LDFLAGS) $(TARGET_OBJS) -o $(BIN_DIR)/$(TARGET_NAME).elf $(CFLAGS)
	$(OBJCOPY) $(OBJCOPY_FLAGS) -j .text $(BIN_DIR)/$(TARGET_NAME).elf $(BIN_DIR)/$(TARGET_NAME).code
	$(OBJCOPY) $(OBJCOPY_FLAGS) -j .data $(BIN_DIR)/$(TARGET_NAME).elf $(BIN_DIR)/$(TARGET_NAME).data
	cat ./bin/game.code ./bin/game.data > ./bin/game.bin

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
