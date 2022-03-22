#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <exception>
#include <math.h>
#include "Components.h"

/* ALU Can perform 8/16/32/64-bit operations
ALU Operations (ALU_op)
     0: add
     1: sub
     2: or
     3: and
     4: xor
     5: srl
     6: sra
     7: sll
     8: slt
     9: sltu
*/
template <typename T>T A;    // First ALU operand
template <typename T>T B;    // Second ALU operand
template <typename T>T* A_bits; // T-bits of A parsed
template <typename T>T* B_bits; // T-bits of B parsed
int size_of_operand; // T-bits

template <typename T>
void ALU<T>::setOperands(T A, T B)
{
    for (int i = 0; i < this->size_of_operand; i++)
    {
        A_bits[i] = ((A >> (this->size_of_operand - i - 1)) & 0x1);
        B_bits[i] = ((B >> (this->size_of_operand - i - 1)) & 0x1);
    }
}

template <typename T>
T ALU<T>::getNum(T *arr)
{
    T sum = 0;
    for (int i = 0; i < this->size_of_operand; i++)
    {
        sum += arr[i] * (T)pow(2, this->size_of_operand - 1 - i);
    }
    return sum;
}

template <typename T>
T* ALU<T>::fullAdder(T a, T b, T carry)
{
    T xor1 = a ^ b;
    T xor2 = xor1 ^ carry;
    T and1 = xor1 & carry;
    T and2 = a & b;
    T or1 = and1 | and2;
    T sum = xor2;
    T carry_out = or1;
    T *ret = (T *)malloc(2 * sizeof(T));
    ret[0] = sum;
    ret[1] = carry_out;
    return (ret);
}

template <typename T>
T ALU<T>::invertNum(T num)
{
    for (int i = 0; i < this->size_of_operand; i++)
        num = (num ^ (T)(1 << i));
    return ++num;
}

template <typename T>
void ALU<T>::bitShiftRight(T isArithmetic)
{
    T MSB = A_bits[0];
    for (int i = this->size_of_operand - 1; i > 0; i--)
    {
        A_bits[i] = A_bits[i - 1];
    }
    if (isArithmetic)
    {
        // Arithmetic right shift
        A_bits[0] = (MSB == 1) ? 1 : 0;
        return;
    }
    A_bits[0] = 0;
}

template <typename T>
void ALU<T>::bitShiftLeft()
{
    for (int i = 0; i < this->size_of_operand; i++)
    {
        A_bits[i] = A_bits[i + 1];
    }
    A_bits[this->size_of_operand - 1] = 0;
}

template <typename T>
T ALU<T>::add()
{
    T *sum = (T *)malloc(this->size_of_operand * sizeof(T));
    T carry_in = 0;
    T *ret;
    for (int i = this->size_of_operand - 1; i >= 0; i--)
    {
        ret = fullAdder(A_bits[i], B_bits[i], carry_in);
        sum[i] = ret[0];
        carry_in = ret[1];
    }

    T alu_out = getNum(sum);
    return alu_out;
}

template <typename T>
T ALU<T>::sub()
{
    this->B = invertNum(this->B);
    setOperands(this->A, this->B);
    T alu_out = add();
    this->B = invertNum(this->B);
    setOperands(this->A, this->B);
    return alu_out;
}

template <typename T>
T ALU<T>::_or()
{
    T *sum = (T *)malloc(this->size_of_operand * sizeof(T));
    for (int i = 0; i < this->size_of_operand; i++)
    {
        sum[i] = this->A_bits[i] | this->B_bits[i];
    }
    T alu_out = getNum(sum);
    return alu_out;
}

template <typename T>
T ALU<T>::_and()
{
    T *sum = (T *)malloc(this->size_of_operand * sizeof(T));
    for (int i = 0; i < this->size_of_operand; i++)
    {
        sum[i] = this->A_bits[i] & this->B_bits[i];
    }
    T alu_out = getNum(sum);
    return alu_out;
}

template <typename T>
T ALU<T>::_xor()
{
    T *sum = (T *)malloc(this->size_of_operand * sizeof(T));
    for (int i = 0; i < this->size_of_operand; i++)
    {
        sum[i] = this->A_bits[i] ^ this->B_bits[i];
    }
    T alu_out = getNum(sum);
    return alu_out;
}

template <typename T>
T ALU<T>::srl()
{
    if (this->B >= (T)32)
    {
        return 0;
    }
    for (T i = 0; i < this->B; i++)
    {
        bitShiftRight(0);
    }
    T alu_out = getNum(A_bits);
    setOperands(this->A, this->B);
    return alu_out;
}

template <typename T>
T ALU<T>::sra()
{
    if (this->B >= (T)32)
    {
        T MSB = A_bits[0];
        T result = (MSB == 1) ? -1 : 0; // 0xffffffff or 0x00000000
        return result;
    }
    for (T i = 0; i < this->B; i++)
    {
        bitShiftRight(1);
    }
    T alu_out = getNum(A_bits);
    setOperands(this->A, this->B);
    return alu_out;
}

template <typename T>
T ALU<T>::sll()
{
    if (this->B >= (T)32)
    {
        return 0;
    }
    for (T i = 0; i < this->B; i++)
    {
        bitShiftLeft();
    }
    T alu_out = getNum(A_bits);
    setOperands(this->A, this->B);
    return alu_out;
}

template <typename T>
T ALU<T>::slt()
{
    if ((this->A == 0x80000000) && (this->B != this->A)) {return 1;}
    T alu_out = sub(); // A - B
    T MSB = alu_out >> 31;
    return MSB == 1;
}

template <typename T>
T ALU<T>::sltu()
{
    T alu_out;
    
    // If a and b are positive
    if (((this->A >> 31) == 0) && ((this->B >> 31) == 0)) {
        return (sub() >> 31) == 1;
    } else if (((this->A >> 31) == 1) && ((this->B >> 31) == 1)) {
        // a negative, b negative
        alu_out = sub();
        return (alu_out >> 31) == 1;
    } else if (((this->A >> 31) == 0) && ((this->B >> 31) == 1)) {
        // a positive, b "negative"
        return 1;
    } else {
        // a "negative", b positive
        return 0;
    }

    // return alu_out < 0;
    return 1;
}

template <typename T>
ALU<T>::ALU()
{
    T test;
    std::string input_type = typeid(test).name();
    //std::cout << "The input is of type " << typeid(test).name() << std::endl;

    // Check for valid operands size
    if (input_type == "j")
    {
        // 32-bit operands
        this->size_of_operand = 32;
    }
    else if (input_type == "m")
    {
        // 64-bit operands
        this->size_of_operand = 64;
    }
    else
    {
        // Invalid operands bit width
        std::cerr << "Invalid Bit Width Exception" << std::endl;
        exit(1);
    }

    A_bits = (T *)malloc((this->size_of_operand) * sizeof(T));
    B_bits = (T *)malloc((this->size_of_operand) * sizeof(T));
}

template <typename T>
void ALU<T>::printOperands()
{
    std::cout << "A: ";
    for (int i = 0; i < this->size_of_operand; i++)
    {
        std::cout << A_bits[i];
    }
    std::cout << " (" << A << ")\nB: ";
    for (int i = 0; i < this->size_of_operand; i++)
    {
        std::cout << B_bits[i];
    }
    std::cout << " (" << B << ")\n";
}

template <typename T>
T ALU<T>::exec(T A, T B, int ALU_op)
{
    // Set operands
    this->A = A;
    this->B = B;
    setOperands(A, B);

    T alu_out;
    switch (ALU_op)
    {
    case (0): // add
        alu_out = add();
        break;
    case (1): // sub
        alu_out = sub();
        break;
    case (2): // or
        alu_out = _or();
        break;
    case (3): // and
        alu_out = _and();
        break;
    case (4): // xor
        alu_out = _xor();
        break;
    case (5): // srl
        alu_out = srl();
        break;
    case (6): // sra
        alu_out = sra();
        break;
    case (7): // sll
        alu_out = sll();
        break;
    case (8): // slt
        alu_out = slt();
        break;
    case (9): // sltu
        alu_out = sltu();
        break;
    default:
        // Invalid opcode
        std::cerr << "Invalid ALU Opcode Exception" << std::endl;
        exit(1);
    }
    return alu_out;
}

template class ALU<uint32_t>;
template class ALU<uint64_t>;