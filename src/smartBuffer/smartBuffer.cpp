#include "pch.hpp"

/*THIS FUNCTION ONLY ACCEPTS IDA PATTERN STYLE*/
std::vector<patternByte> getBytesFromPattern(std::string_view pattern)
{
	std::vector<patternByte> result;

	for (size_t i = 0; i < pattern.size(); i++)
	{
		if (pattern[i] == ' ')
			continue;

		if (pattern[i] == '?')
		{
			if (i != (pattern.size() - 1) && pattern[i + 1] == '?')
				i++; /*skip double wildcard*/

			result.push_back({ 0, true });
			continue;
		}

		/*this thing corrects signatures like this: 0F 9
		it will add a 0 BEFORE the nine, meaning it'll be a 09*/
		char firstByte = i != (pattern.size() - 1) ? pattern[i] : char(0);
		char secondByte = i != (pattern.size() - 1) ? pattern[i + 1] : pattern[i];
		char string[2] = { firstByte, secondByte };

		result.push_back({ uint8_t(strtoul(string, nullptr, 16)), false });
		i++;
	}

	return std::move(result);
}

smartBuffer::smartBuffer(char* buffer, size_t size, bool copy) : 
	m_wasCopied(false), m_bufferSize(size), m_bufferPointer(0), m_linkedFile()
{
	EXCEPTION(!buffer, "Input buffer can't be NULL");
	EXCEPTION(!size, "Input buffer size can't be NULL");

	if (!copy)
	{
		m_memory = std::unique_ptr<uint8_t[]>((uint8_t*)buffer); /*only take ownership*/
		return;
	}

	m_memory = std::make_unique<uint8_t[]>(size);
	m_wasCopied = true;

	readFrom(buffer, size);
}
smartBuffer::smartBuffer(std::filesystem::path filePath) :
	m_wasCopied(false), m_bufferSize(0), m_bufferPointer(0), m_linkedFile()
{
	setFileMode(filePath);
}
smartBuffer::smartBuffer(size_t size) :
	m_wasCopied(false), m_bufferSize(0), m_bufferPointer(0), m_linkedFile()
{
	reallocate(size);
}
smartBuffer::~smartBuffer()
{
	clear();
}

void smartBuffer::write(void* inBuffer, size_t inBufferSize)
{
	auto startAddr = m_bufferPointer;
	startBufferOperation(bufferOperation::write, inBufferSize);

	std::copy_n((uint8_t*)inBuffer, inBufferSize, &m_memory[startAddr]);
}
void smartBuffer::writeFile(void* inBuffer, size_t inBufferSize)
{
	EXCEPTION(!m_linkedFile.is_open(), "Buffer is not set to work with files");
	
	m_linkedFile.seekg(m_bufferPointer, std::fstream::beg);
	startBufferOperation(bufferOperation::fileWrite, inBufferSize);

	EXCEPTION(!m_linkedFile.write((char*)inBuffer, inBufferSize), "Could not write to file");
}

void smartBuffer::read(void* outBuffer, size_t outBufferSize)
{
	auto startAddr = m_bufferPointer;
	startBufferOperation(bufferOperation::read, outBufferSize);

	std::copy_n(&m_memory[startAddr], outBufferSize, (uint8_t*)outBuffer);
}
void smartBuffer::readFile(void* outBuffer, size_t outBufferSize)
{
	EXCEPTION(!m_linkedFile.is_open(), "Buffer is not set to work with files");
	
	m_linkedFile.seekg(m_bufferPointer, std::fstream::beg);
	startBufferOperation(bufferOperation::fileRead, outBufferSize);

	EXCEPTION(!m_linkedFile.read((char*)outBuffer, outBufferSize), "Could not read from file");
	m_bufferPointer = 0;
}

void smartBuffer::memset(uint8_t val)
{
	for (size_t i = 0; i < m_bufferSize; i++)
		m_memory[i] = 0;
}
std::vector<uint64_t> smartBuffer::scanPattern(std::string_view pattern, uint64_t startOff, uint64_t endOff)
{
	EXCEPTION(startOff > m_bufferSize, "Starting offset can't be greater than buffer size");
	EXCEPTION(endOff > m_bufferSize, "Ending offset can't be greater than buffer size");
	EXCEPTION(startOff > endOff, "Starting offset can't be greater than ending offset");

	std::vector<uint64_t> result;
	auto patternBytes = getBytesFromPattern(pattern);

	auto endOffset = (endOff == 0) ? m_bufferSize : endOff;	
	for (auto i = startOff; i < endOffset - patternBytes.size(); i++)
	{
		auto bytes = &m_memory[i];

		for (uint64_t j = 0; j < patternBytes.size(); j++)
		{
			if (patternBytes[j].m_isWildcard)
				continue;

			if (patternBytes[j].m_value != bytes[j])
				break;

			if (j == (patternBytes.size() - 1))
				result.push_back(uint64_t(m_memory.get()) + i);
		}
	}
	return result;
}

void smartBuffer::clear()
{
	if (m_wasCopied)
		m_memory.reset();

	m_wasCopied = false;
	m_bufferSize = 0;
	m_bufferPointer = 0;
	m_linkedFile.close();
}
void smartBuffer::reallocate(size_t newSize)
{
	/*this will force a copy*/
	m_wasCopied = true;
	startBufferOperation(bufferOperation::reallocate, newSize);

	auto newMem = std::make_unique<uint8_t[]>(newSize);

	/*we might have init the buffer but it is empty, so it'd be dumb to copy 0s*/
	if (m_memory) 
		std::copy_n(m_memory.get(), m_bufferSize, newMem.get()); /*copy old data to new mem*/

	m_memory = std::move(newMem);
	m_bufferSize = newSize;
}

void smartBuffer::setBufferPointer(uint64_t newPointer)
{
	EXCEPTION(!newPointer, "Buffer pointers can't be NULL");
	EXCEPTION((m_bufferPointer + newPointer) > m_bufferSize, "Buffer pointer surpass buffer bounds!");

	m_bufferPointer = newPointer;
}

void smartBuffer::setFileMode(std::filesystem::path filePath)
{
	EXCEPTION(m_linkedFile.is_open(), "File is already opened, close it first");

	m_linkedFile.open
	(
		filePath.string(), 
		std::fstream::in | std::fstream::out | 
		std::fstream::binary | std::fstream::ate
	);
	
	if (!m_linkedFile.is_open()) /*file didn't exist, let's create it*/
	{
		m_linkedFile.open
		(
			filePath.string(),
			std::fstream::in | std::fstream::out |
			std::fstream::binary | std::fstream::trunc | std::fstream::ate
		);
	}

	m_bufferSize = m_linkedFile.tellg();/*if != 0, file was not empty*/
	m_linkedFile.seekg(0, m_linkedFile.beg);

	if (m_bufferSize == 0)
		return; /*file was created but it was empty*/

	m_memory = std::make_unique<uint8_t[]>(m_bufferSize);
	readFile(m_memory.get(), m_bufferSize);
}

size_t smartBuffer::getBuffersize()
{
	return m_bufferSize;
}
size_t smartBuffer::getBufferPointer()
{
	return m_bufferPointer;
}
uint8_t* smartBuffer::getBufferData()
{
	return m_memory.get();
}

uint8_t* smartBuffer::begin()
{
	EXCEPTION(!m_memory, "Buffer is not initialized!");
	return &m_memory[0];
}
uint8_t* smartBuffer::end()
{
	EXCEPTION(!m_memory, "Buffer is not initialized!");
	return &m_memory[m_bufferSize - 1];
}

void smartBuffer::startBufferOperation(bufferOperation operation, size_t operationSize)
{
	EXCEPTION(!operationSize, "Buffer operation size can't be NULL");


	switch (operation)
	{
	case bufferOperation::read:
	case bufferOperation::write:
	{
		EXCEPTION((m_bufferPointer + operationSize) > m_bufferSize, "Buffer operation surpass buffer bounds!");

		m_bufferPointer += operationSize;
		break;
	}
	case bufferOperation::fileRead:
	{
		EXCEPTION(!m_linkedFile.is_open(), "Buffer is not set to work with files");
		EXCEPTION(operationSize > m_bufferSize, "File size is smaller than read size");

		m_bufferPointer += operationSize;
		break;
	}
	case bufferOperation::fileWrite:
	{
		EXCEPTION(!m_linkedFile.is_open(), "Buffer is not set to work with files");
		m_bufferPointer += operationSize;
		break;
	}
	case bufferOperation::reallocate:
	{
		/*in reallocations we want to preserve the bufferPointer*/
		EXCEPTION(operationSize < m_bufferSize, "Buffer reallocated size is smaller than buffer size");
		break;
	}
	}

}
void smartBuffer::readFrom(char* inBuffer, size_t inBufferSize)
{
	EXCEPTION(inBufferSize > m_bufferSize, "In buffer is bigger than buffer");
	EXCEPTION(!inBuffer, "In buffer is NULL");

	std::copy_n((uint8_t*)inBuffer, inBufferSize, m_memory.get());
}