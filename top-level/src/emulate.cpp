#include "Pipeline.h"
#include <fstream>
#include <iterator>
#include <vector>
#include <map>

#define ROM_START 0x00000000
#define RAM_START 0x80000000

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4) {
        std::cerr << "Invalid Arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // Debug Mode
	int debug = 0;
	if (argc == 4)
		debug = (atoi(argv[3]) == 1) ? 1 : 0;

    // Memory
    std::multimap<uint32_t, uint32_t> dram_flash;

    // Flash text section to Memory
    std::ifstream text_input(argv[1], std::ios::binary);
    std::vector<char> text_section(
         (std::istreambuf_iterator<char>(text_input)),
         (std::istreambuf_iterator<char>()));
    text_input.close();
    int num_instructions = text_section.size() / 4;    
    for (int i = 0; i < num_instructions; i++) {
        // Preliminary 32-bit instruction
        uint32_t instruction = ((text_section[4*i+3] << 24) & 0xff000000) |
                               ((text_section[4*i+2] << 16) & 0x00ff0000) |
                               ((text_section[4*i+1] << 8)  & 0x0000ff00) |
                               ((text_section[4*i+0])       & 0x000000ff);
        dram_flash.insert(std::make_pair((uint32_t)(ROM_START + 4*i), instruction));
    }

    // Flash data segment
    std::ifstream data_input(argv[2], std::ios::binary);
    std::vector<char> data_section(
         (std::istreambuf_iterator<char>(data_input)),
         (std::istreambuf_iterator<char>()));
    data_input.close();
    int num_data = data_section.size() / 4;    
    for (int i = 0; i < num_data; i++) {
        // Preliminary 32-bit instruction
        uint32_t data = ((data_section[4*i+3] << 24) & 0xff000000) |
                        ((data_section[4*i+2] << 16) & 0x00ff0000) |
                        ((data_section[4*i+1] << 8)  & 0x0000ff00) |
                        ((data_section[4*i+0])       & 0x000000ff);
        dram_flash.insert(std::make_pair((uint32_t)(RAM_START + 4*i), data));
    }

    // for (auto data : dram_flash) {
    //     std::cout << std::hex << data.first << ", " << data.second << std::endl;
    // }
    // exit(1);

    // std::cout << std::hex << "Starting\n";
	Pipeline t89(dram_flash, debug);
	if (t89.Construct(WIDTH, HEIGHT, 2, 2))
		t89.Start();

	exit(EXIT_SUCCESS);
}
