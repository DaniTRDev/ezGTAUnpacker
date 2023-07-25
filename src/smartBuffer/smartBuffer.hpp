#pragma once

struct patternByte
{
	uint8_t m_value;
	bool m_isWildcard;
};

class smartBuffer
{
public:

	smartBuffer& operator=(const smartBuffer&) = delete;
	smartBuffer(const smartBuffer&) = delete;

	/*if copy is set buffer content will be copied to m_memory*/
	smartBuffer(char* buffer, size_t size, bool copy);
	smartBuffer(std::filesystem::path filePath);
	smartBuffer(size_t size); /*initialize buffer with 0s*/
	~smartBuffer();

	void write(void* inBuffer, size_t inBufferSize);
	void writeFile(void* inBuffer, size_t inBufferSize);

	void read(void* outBuffer, size_t outBufferSize);
	void readFile(void*  outBuffer, size_t outBufferSize);

	void memset(uint8_t val);
	std::vector<uint64_t> scanPattern(std::string_view pattern, uint64_t startOff = 0, uint64_t endOff = 0);

	void clear(); /*will clear memory*/
	void reallocate(size_t newSize);

	void setBufferPointer(uint64_t newPointer);
	void setFileMode(std::filesystem::path filePath);

	size_t getBuffersize();
	size_t getBufferPointer();
	uint8_t* getBufferData(); /*not safe!!*/

	uint8_t* begin();
	uint8_t* end();

private:

	enum class bufferOperation
	{
		read, 
		write,
		fileRead,
		fileWrite,
		reallocate,
	};

private:

	/*will do checks with fileSize & filePointer, as well as incrementing the filePointer*/
	void startBufferOperation(bufferOperation operation, size_t operationSize);

	/*used by the constructor, code is the same as read() but purpose is diff*/
	void readFrom(char* inBuffer, size_t inBufferSize);

private:

	std::unique_ptr<uint8_t[]> m_memory;
	bool m_wasCopied;

	size_t m_bufferSize;
	size_t m_bufferPointer;

	std::fstream m_linkedFile;
};