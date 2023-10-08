#pragma once

namespace unpacker
{
	struct unpackerContext
	{

		/*ptr to the data that has the addr info */
		uint8_t**					m_encryptedTokenBlock;
		uint8_t**					m_encryptedGameDataBlock;

		int32_t*					m_decryptionKey;

		/*the 2 values used to rotate the key*/
		int32_t*					m_decryptionKeyRot;
		int32_t						m_decryptionKeyRot2;

		/*self explanatory*/
		Pe::PeNative*				m_peFile;

		/*THINGS USED TO CHECK IF DECOMPILATION WAS GOOD*/
		uint32_t					m_initialDecTokenResult;
	};
	struct encryptedDataInfo
	{
		BYTE*						m_encryptedBlockAddrBytes;
		memBlock*					m_decryptedBlockAddr;
		uint64_t*					m_currentToken;
	};
	extern void unpack(unpackerContext* context);
}