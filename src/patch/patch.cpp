#include "pch.hpp"

class myErrorHandler : public asmjit::ErrorHandler
{
public:
	void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
		std::cout << (std::format("asmjit error! {} (Id: {})", message, err)) << std::endl;
	}
};

static auto errHandler = std::make_unique<myErrorHandler>();
static auto env = asmjit::Environment(asmjit::Arch::kX64);


patch::patch(uint64_t address) : m_compiled(false)
{
	m_codeHolder = std::make_unique<asmjit::CodeHolder>();
	m_codeHolder->init(env, address);
	m_codeHolder->setErrorHandler(errHandler.get());

	m_assembler = std::make_unique<asmjit::x86::Assembler>(m_codeHolder.get());
}
patch::~patch()
{
	m_assembler.reset();
	m_codeHolder.reset();
}

void patch::compile()
{
	if (isCompiled())
		return;

	EXCEPTION(!buildPatch, "There's no code to compile");
	EXCEPTION(!buildPatch(m_assembler.get()), "Could not build patch");

	m_compiled = true;
}
bool patch::isCompiled()
{
	return m_compiled;
}

asmjit::CodeHolder* patch::getCompiledCode()
{
	return m_codeHolder.get();
}
