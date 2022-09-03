#include <stdint.h>

#ifndef REGISTERFILE_H
#define REGISTERFILE_H

class RegisterFile
{
private:
    uint32_t* registers;

public:
    RegisterFile();
    ~RegisterFile();
    uint32_t read(int);
    void write(uint32_t, int);
};

#endif // REGISTERFILE_H