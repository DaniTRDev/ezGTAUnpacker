#include "pch.hpp"

void unpackPE(Pe::PeNative& peFile) 
{
    /*open the file that contains the decryption context data*/
    try
    {
        auto file = smartBuffer(std::filesystem::current_path() / "data.json");
        EXCEPTION(!file.getBuffersize(), "Can't decrypt without the required data!");

        auto json = nlohmann::json::parse(std::string((char*)file.getBufferData(), file.getBuffersize()).c_str());
        auto encryptedTokenBlock = (uint8_t*)(peFile.byRva<uint64_t>(jsonHexStr2Uint(json["tokenBlock"])));
        auto encryptedGameDataBlock = (uint8_t*)(peFile.byRva<uint64_t>(jsonHexStr2Uint(json["encryptedDataBlock"])));

        //^the 2 things are equal, but they might change in a future update so 
        //I decided to have one variable for each. It also makes the code more 
        //readable
        unpacker::unpackerContext context;
        context.m_encryptedTokenBlock = &encryptedTokenBlock;
        context.m_encryptedGameDataBlock = &encryptedGameDataBlock;

        context.m_decryptionKeyRot = (int32_t*)(peFile.byRva<uint64_t>(jsonHexStr2Uint(json["decryptionKeyRot"])));
        context.m_decryptionKeyRot2 = int32_t(jsonHexStr2Uint(json["decryptionKeyRot2"]));
        context.m_decryptionKeyRot3 = int32_t(jsonHexStr2Uint(json["decryptionKeyRot3"]));
        context.m_decryptionKey = (int32_t*)(peFile.byRva<uint64_t>(jsonHexStr2Uint(json["decryptionKey"])));

        context.m_peFile = &peFile;

        context.m_initialDecTokenResult = uint32_t(jsonHexStr2Uint(json["tokenCheck"]));
        context.m_decryptionKeyResult = uint32_t(jsonHexStr2Uint(json["keyCheck"]));
        unpacker::unpack(&context);
    }
    catch (nlohmann::json::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
void patchEp(Pe::PeNative& peFile, uint64_t oepStart)
{
    auto mainAddress = ptr((void*)peFile.entryPoint()).add(13).relative(1);
    auto newEp = patch(peFile.entryPoint());
    
    newEp.buildPatch = [oepStart, mainAddress](asmjit::x86::Assembler* a) -> bool
    {
        using namespace asmjit::x86;

        a->sub(rsp, 0x28);
        a->call(oepStart);
        a->add(rsp, 0x28);
        a->jmp(mainAddress.address() + 1);

        return true;
    }; newEp.compile();

    auto newEpCode = newEp.getCompiledCode()->textSection();
    std::copy_n
    (
        newEpCode->buffer().data(),
        newEpCode->bufferSize(),
        (uint8_t*)peFile.entryPoint()
    );
}
void fixSectionsPermissions(Pe::PeNative& peFile)
{
    /*you'll need to deal with permissions at runtime or patch PE checks...*/
    for (uint64_t i = 0; i < peFile.sections().count(); i++)
    {
        auto section = const_cast<Pe::GenericTypes::SecHeader*>(&peFile.sections().sections()[i]);
        
        if (section->Characteristics & IMAGE_SCN_MEM_SHARED)
            continue;

        DWORD newPerms = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
        section->Characteristics = newPerms;
    }
}
void updatePeChecksum(smartBuffer& data, Pe::PeNative& peFile)
{
    auto nt = (IMAGE_NT_HEADERS64*)peFile.headers().nt();
    auto dos = (IMAGE_DOS_HEADER*)peFile.headers().dos();
    nt->OptionalHeader.CheckSum = generateNewPeChecksum(data, uint64_t(dos->e_lfanew + 0x58));
}

int main(int argv, char** argc)
{
    auto data = smartBuffer(std::filesystem::current_path() / "encrypted.exe");
    auto peFile = Pe::PeNative::fromFile(data.getBufferData());

    std::cout << "Unpacking...." << std::endl;
    unpackPE(peFile);

    std::cout << "Fixing oep..." << std::endl;
    patchEp(peFile, data.scanPattern("48 89 5C 24 20 55 48 8B EC 48 83 EC ?? 48 8B")[0]);

    std::cout << "Fixing sections permissions" << std::endl;
    fixSectionsPermissions(peFile);

    std::cout << "Updating PE checksum..." << std::endl;
    updatePeChecksum(data, peFile);
   
    auto decrypted = smartBuffer(std::filesystem::current_path() / "decrypted.exe");
    decrypted.writeFile(data.getBufferData(), data.getBuffersize());
 
    return S_OK;
}