#include "CPU.h"
#include <fstream>
#include <iterator>
#include <vector>
#include <map>

int main(int argc, char *argv[])
{
    // Read binary and flash to DRAM
    std::ifstream input("../firmware/bin/tester.bin", std::ios::binary);
    std::vector<char> bytes(
         (std::istreambuf_iterator<char>(input)),
         (std::istreambuf_iterator<char>()));
    input.close();
    
    // Start simulation
	int debug = 0;
	if (argc == 2)
		debug = (atoi(argv[1]) == 1) ? 1 : 0;

    // Convert to big endian and create flash image
    std::multimap<uint32_t, uint32_t> dram_flash;
    int num_instructions = bytes.size() / 4;    
    for (int i = 0; i < num_instructions; i++) {
        // Preliminary 32-bit instruction
        uint32_t instruction = ((bytes[4*i+3] << 24) & 0xff000000) |
                               ((bytes[4*i+2] << 16) & 0x00ff0000) |
                               ((bytes[4*i+1] << 8)  & 0x0000ff00) |
                               ((bytes[4*i+0])       & 0x000000ff);
        dram_flash.insert(std::make_pair((uint32_t)(4*i), instruction));
    }

	CPU t89(dram_flash, debug);
	if (t89.Construct(200, 200, 2, 2))
		t89.Start();

	exit(EXIT_SUCCESS);
}