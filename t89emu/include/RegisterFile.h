#include <stdint.h>
#include <string>
#include <vector>

#ifndef REGISTERFILE_H
#define REGISTERFILE_H

class RegisterFile {
public:
    RegisterFile(void);
    ~RegisterFile();

    uint32_t read(int reg);
    void write(uint32_t data, int reg);

    std::vector<std::string>& getNames(void);

private:
    uint32_t *registers;
    std::vector<std::string> names;
};

#endif // REGISTERFILE_H