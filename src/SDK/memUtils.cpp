#include "pch.hpp"

uint32_t copy_1(BYTE* destination, BYTE* source, uint32_t size)
{/*simplified code and refactored*/
    uint32_t result = 0;
    uint32_t i = 0;

    for (; i < size; i++, result = i)
        destination[i] = source[i];

    return result;
}
uint32_t memset_1(BYTE* destination, char value, uint32_t size)
{
    uint32_t result = 0; // rax
    uint32_t i = 0; // [rsp+28h] [rbp+8h]

    for (; i < size; i++, result = i)
        destination[i] = (BYTE)value;

    return result;
}