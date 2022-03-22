#include <stdlib.h>
#include <iostream>
#include "Components.h"

/*
x0	        zero
x1	        ra
x2	        sp
x3	        gp
x4	        tp
x5-7        t0-2
x8	        s0
x9	        s1
x10-11	    a0-1
x12-17	    a2-7
x18-27	    s2-11
x28-31	    t3-6
*/
template <typename T>T* registers;
int rs1;
int rs2;
int rd;
int Reg_Write;

template <typename T>
RegisterFile<T>::RegisterFile()
{
    // All registers initially 0
    this->registers = (T *)calloc(32, sizeof(T));
}

template <typename T>
void RegisterFile<T>::set_control_lines(int rs1, int rs2, int Reg_Write)
{
    this->rs1 = rs1;
    this->rs2 = rs2;
    this->Reg_Write = Reg_Write;

}

template <typename T>
T RegisterFile<T>::read_rs1()
{
    return this->registers[this->rs1];
}

template <typename T>
T RegisterFile<T>::read_rs2()
{
    return this->registers[this->rs2];
}

template <typename T>
T RegisterFile<T>::read_rd()
{
    return this->registers[this->rd];
}

template <typename T>
void RegisterFile<T>::write(T data, int rd)
{   
    if (rd == 0) {
        this->rd = 0;    
        return;
    }
    if (this->Reg_Write) {
        this->registers[rd] = data;
        this->rd = rd;
    }
}

template class RegisterFile<uint32_t>;
template class RegisterFile<uint64_t>;