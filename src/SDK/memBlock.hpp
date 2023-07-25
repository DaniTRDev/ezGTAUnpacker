#pragma once

struct memBlock
{
	int32_t m_memStart;
	int32_t m_memSize;
};

void decryptMemBlockData(uint8_t** encryptedBytes, memBlock* outMemBlock);