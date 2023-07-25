#pragma once

#define EXCEPTION(condition, message, ...) if (condition) \
{ \
	auto formattedMessage = std::format(message, __VA_ARGS__); \
	throw std::runtime_error \
	(\
		std::format("[{}] -> {}", __FUNCDNAME__, formattedMessage).data() \
	); \
}


#define HIWORDPTR(x) *(reinterpret_cast<unsigned short int *>(&x) + 1)
#define LOWORDPTR(x) *(reinterpret_cast<unsigned short int *>(&x) + 0)

class ptr
{
public:

	inline ptr(void* data) : m_ptr(data)
	{
		EXCEPTION(!data, "Ptr is not valid!");
	}

	inline operator bool()
	{
		return m_ptr != 0;
	}
	inline bool isNull()
	{
		return !m_ptr;
	}

	template <typename T>
	inline std::enable_if_t<std::is_pointer<T>::value, T> as() const
	{
		return reinterpret_cast<T>(m_ptr);
	}

	template<typename T>
	inline T deref()
	{
		return *as<T>();
	}

	inline uint64_t address()
	{
		return uint64_t(m_ptr);
	}
	inline uint64_t address() const
	{
		return uint64_t(m_ptr);
	}

	inline ptr add(uintptr_t offset)
	{
		return ptr((void*)(uintptr_t(m_ptr) + offset));
	}
	inline ptr sub(uintptr_t offset)
	{
		return ptr((void*)(uintptr_t(m_ptr) - offset));
	}

	inline ptr relative(uintptr_t instructionSize) /*the number of opcodes of the instruction*/
	{
		/*we need to add 4 because the relative offset is from the current instruction, so we need to
		skip the relative address bytes*/
		auto relOffset = *add(instructionSize).as<int32_t*>();
		return ptr((void*)(intptr_t(m_ptr) + relOffset)).add(4);
	}

private:
	void* m_ptr{ 0 };
};

inline uint64_t getImportAddress(Pe::PeNative& peFile, const char* moduleName, const char* importName)
{
	auto descriptor = peFile.imports().descriptor().ptr;
	while (descriptor->Characteristics)
	{
		uint32_t numImports = 0;
		auto importedModuleName = peFile.byRva<const char>(descriptor->Name);
		auto importedFunc = peFile.byRva<IMAGE_THUNK_DATA64>(descriptor->FirstThunk);

		if (_strcmpi(importedModuleName, "kernel32.dll") != 0)
			continue;

		while (importedFunc)
		{
			auto name = peFile.byRva<IMAGE_IMPORT_BY_NAME>(importedFunc->u1.AddressOfData);

			if (_strcmpi(name->Name, "VirtualProtect") == 0)
				return uint64_t(peFile.byRva<uint64_t>(descriptor->FirstThunk + numImports * sizeof(void*)));
			

			numImports++;
			importedFunc++;
		}
		descriptor++;
	}

	return 0;
}

inline uint64_t jsonHexStr2Uint(nlohmann::json& json)
{
	auto str = json.get<std::string>();
	return strtoull(str.c_str(), nullptr, 16);
}
inline int64_t jsonHexStr2Int(nlohmann::json& json)
{
	auto str = json["tokenBlock"].get<std::string>();
	return int64_t(strtoull(str.c_str(), nullptr, 16));
}

inline uint32_t generateNewPeChecksum(smartBuffer& data, uint64_t checkSumOffset)
{
	auto pointer = (uint32_t*)data.getBufferData();
	int64_t checksum = 0;
	int64_t top = int64_t(pow(2, 32));

	for (uint32_t i = 0; i < data.getBuffersize() / 4; i++)
	{
		if (i == uint32_t(checkSumOffset / 4))
			continue;

		checksum = (checksum & 0xffffffff) + pointer[i] + (checksum >> 32);

		if (checksum > top)
			checksum = (checksum & 0xffffffff) + (checksum >> 32);
	}

	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = (checksum)+(checksum >> 16);
	checksum = checksum & 0xffff;

	checksum += uint32_t(data.getBuffersize());

	return uint32_t(checksum);
}
