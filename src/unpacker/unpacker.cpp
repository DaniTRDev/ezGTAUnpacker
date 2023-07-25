#include "pch.hpp"

namespace unpacker
{
    void decryptBlockRound1(
        Pe::PeNative* peFile,
        BYTE** encryptedBlockAddrBytes,
        memBlock* decryptedBlockAddress,
        uint32_t* decryptionTokenAddress,
        uint32_t decryptionSize,
        uint32_t* decryptedBytesCount)
    {
        BYTE* decryptedBytesAddr;
        uint32_t remainingBytesCount = 0;

        auto decryptionTokenStart = decryptionTokenAddress;
        auto decryptionTokenEnd = (uint32_t*)((char*)decryptionTokenAddress + decryptionSize);
        auto decryptionSize_1 = decryptionSize;

        do
        {
            remainingBytesCount = decryptedBlockAddress->m_memSize - *decryptedBytesCount;
            decryptedBytesAddr = (BYTE*)peFile->byRva<BYTE>(*decryptedBytesCount + decryptedBlockAddress->m_memStart);

            if (decryptionSize_1 >= remainingBytesCount)
            {
                if (decryptionSize_1 == remainingBytesCount)
                {
                    copy_1((BYTE*)decryptionTokenStart, (BYTE*)decryptedBytesAddr, remainingBytesCount);
                    decryptionTokenStart = (uint32_t*)((char*)decryptionTokenStart + remainingBytesCount);
                    decryptMemBlockData(encryptedBlockAddrBytes, decryptedBlockAddress);

                    if (decryptedBlockAddress->m_memStart == -1)
                        return;

                    *decryptedBytesCount = 0;
                }
                else
                {
                    copy_1((BYTE*)decryptionTokenStart, (BYTE*)decryptedBytesAddr, remainingBytesCount);
                    decryptionTokenStart = (uint32_t*)((char*)decryptionTokenStart + remainingBytesCount);
                    *decryptedBytesCount = 0;

                    decryptMemBlockData(encryptedBlockAddrBytes, decryptedBlockAddress);

                    decryptionSize_1 -= remainingBytesCount;

                    if (decryptedBlockAddress->m_memStart == -1)
                    {
                        memset_1((BYTE*)decryptionTokenStart, 0, decryptionSize_1);
                        return;
                    }
                }
            }
            else
            {
                copy_1((BYTE*)decryptionTokenStart, (BYTE*)decryptedBytesAddr, decryptionSize_1);
                decryptionTokenStart = (uint32_t*)((char*)decryptionTokenStart + decryptionSize_1);
                *decryptedBytesCount += decryptionSize_1;
            }

        } while (decryptionTokenStart != decryptionTokenEnd);
    }

    void decryptBlockRound2(
        Pe::PeNative* peFile,
        BYTE** encryptedGameDataInfoAddr,
        uint32_t* decryptionTokenAddress,
        uint32_t decryptionSize,                 // always 8
        memBlock* decryptedBlockAddress,                      // persists
        uint32_t* decryptedBytesCount)          // this one is zeroed
    {
        BYTE* encryptedBytesAddr = 0;
        uint32_t remainingBytesCount = 0;
        uint32_t remainingBytesCount_2 = 0;
        uint32_t currentDecryptedByteCount = 0;

        do
        {
            remainingBytesCount = decryptedBlockAddress->m_memSize - *decryptedBytesCount;
            remainingBytesCount_2 = decryptionSize - currentDecryptedByteCount;
            encryptedBytesAddr = (BYTE*)peFile->byRva<BYTE>(*decryptedBytesCount + decryptedBlockAddress->m_memStart);

            if (remainingBytesCount <= remainingBytesCount_2)  // moves on to the next
            {
                copy_1(encryptedBytesAddr, (BYTE*)decryptionTokenAddress + currentDecryptedByteCount, remainingBytesCount);
                currentDecryptedByteCount += remainingBytesCount;
                decryptMemBlockData(encryptedGameDataInfoAddr, decryptedBlockAddress);
                if (decryptedBlockAddress->m_memStart == -1)
                    return;
                *decryptedBytesCount = 0;
            }
            else
            {
                copy_1(encryptedBytesAddr, (BYTE*)decryptionTokenAddress + currentDecryptedByteCount, remainingBytesCount_2);
                currentDecryptedByteCount = decryptionSize;
                *decryptedBytesCount += remainingBytesCount_2;
            }
        } while (decryptionTokenAddress + currentDecryptedByteCount != decryptionTokenAddress + decryptionSize);
    }

    void rotateDecryptionKey(unpackerContext* context)
    {
        uint32_t rot1 = context->m_decryptionKeyRot[0]; // [rsp+38h] [rbp+18h]
        uint32_t rot2 = context->m_decryptionKeyRot[1]; // [rsp+3Ch] [rbp+1Ch]
        uint32_t rot3 = 0; // [rsp+40h] [rbp+20h]
        uint32_t i = 0; // [rsp+44h] [rbp+24h]

        for (i = 0; i < 0x40; ++i)
        {
            rot1 += (rot3 + context->m_decryptionKey[rot3 & 3]) ^ (rot2 + ((rot2 >> 5) ^ (16 * rot2)));
            rot3 -= context->m_decryptionKeyRot2;
            rot2 += (rot3 + context->m_decryptionKey[(rot3 >> 11) & 3]) ^ (rot1 + ((rot1 >> 5) ^ (16 * rot1)));
        }
        context->m_decryptionKeyRot[0] = rot1;
        context->m_decryptionKeyRot[1] = rot2;
    }

    void doFirstDecryptionRound(unpackerContext* context)
    {
        memBlock block{};
        decryptMemBlockData(context->m_encryptedTokenBlock, &block);

        uint32_t currentToken = 0;
        uint32_t remainingBytes = 0;

        uint32_t finalToken = 0;

        for (finalToken = 0; block.m_memStart != -1; finalToken += currentToken)
        {
            decryptBlockRound1
            (
                context->m_peFile,
                context->m_encryptedTokenBlock,
                &block,
                &currentToken, 4u, &remainingBytes
            );
        }

        EXCEPTION(finalToken != context->m_initialDecTokenResult, "Initial decryption stage failed!");
    }


    void unpack(unpackerContext* context)
    {
        doFirstDecryptionRound(context);

        memBlock block{};
        memBlock block2{};
        decryptMemBlockData(context->m_encryptedGameDataBlock, &block);

        int64_t currentToken = 0;
        uint32_t remainingBytes = { 0 };
        uint32_t remainingBytes2[2] = {};

        encryptedDataInfo info{};

        while (block.m_memStart != -1)
        {
            block2 = block;/*this sucks but we need a copy forcefully...*/
            remainingBytes2[0] = remainingBytes;
            info.m_encryptedBlockAddrBytes = *context->m_encryptedGameDataBlock;
            info.m_decryptedBlockAddr = &block;

            decryptBlockRound1
            (
                context->m_peFile,
                context->m_encryptedGameDataBlock,
                &block,
                (uint32_t*)&currentToken,
                8,
                &remainingBytes
            );
            rotateDecryptionKey(context);
            currentToken ^= *(uint64_t*)context->m_decryptionKeyRot;

            info.m_currentToken = (uint64_t*)&currentToken;
            decryptBlockRound2
            (
                context->m_peFile,
                (BYTE**)&info,
                (uint32_t*)&currentToken,
                8,
                &block2,
                (uint32_t*)&remainingBytes2
            );
        }
    }
}