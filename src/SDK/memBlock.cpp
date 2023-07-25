#include "pch.hpp"



/*thanks to https://github.com/sfinktah/sfida/blob/672b770eb46ab005c76d845847726687578bddd9/checksummer.py#L80*/
void decryptMemBlockData(uint8_t** encryptedBytes, memBlock* outMemBlock)
{
    EXCEPTION(!outMemBlock, "Resulting memory block can't be NULL");
    int32_t byte, accum = 0, shift = 0;

    do
    {
        accum += (**encryptedBytes & 0x7F) << shift;
        shift += 7;
        byte = *(*encryptedBytes)++;

    } while (byte >= 128);

    outMemBlock->m_memStart += accum + outMemBlock->m_memSize;

    if (outMemBlock->m_memStart != -1)
    {
        shift = 0;
        accum = 0;
        do
        {
            accum += (**encryptedBytes & 0x7F) << shift;
            shift += 7;
            byte = *(*encryptedBytes)++;

        } while (byte >= 128);

        outMemBlock->m_memSize = accum;
    }
}
