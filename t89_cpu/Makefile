OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib

CFLAGS = -Wall -std=c++17 -g
# LDFLAGS += -L../t89_cpu_components/lib -lt89_cpu_componentslib.a -lX11 -lGL -lpthread -lpng -lstdc++fs
LDFLAGS = ../t89_cpu_components/lib/t89_cpu_componentslib.a -lX11 -lGL -lpthread -lpng -lstdc++fs

CXX = g++
INCLUDE += -I $(INC_DIR) -I../t89_cpu_components/include

TARGET_NAME = $(LIB_DIR)/cpulib.a
TARGET_OBJS = $(OBJ_DIR)/CPU.o

all: directories $(TARGET_NAME)

test_driver: $(SRC_DIR)/t89_driver.cpp $(LIB_DIR)/cpulib.a
	$(CXX) $(SRC_DIR)/t89_driver.cpp $(CFLAGS) $(INCLUDE) $(LIB_DIR)/cpulib.a $(LDFLAGS) -o test_driver

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
	rm -rf ./test_driver
	rm -rf ./emulate