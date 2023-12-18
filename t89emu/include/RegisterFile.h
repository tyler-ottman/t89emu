#include <stdint.h>

#ifndef REGISTERFILE_H
#define REGISTERFILE_H

class RegisterFile {
public:
    RegisterFile(void);
    ~RegisterFile();

    uint32_t read(int reg);
    void write(uint32_t data, int reg);

private:
    uint32_t *registers;
};

#endif // REGISTERFILE_H