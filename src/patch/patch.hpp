#pragma once

class patch
{
public:

	patch(uint64_t address);
	~patch();

	using buildFuncT = bool(asmjit::x86::Assembler* a);
	std::function<buildFuncT> buildPatch;

	void compile();
	bool isCompiled();

	asmjit::CodeHolder* getCompiledCode();

private:

	bool m_compiled;

	std::unique_ptr<asmjit::CodeHolder> m_codeHolder;
	std::unique_ptr<asmjit::x86::Assembler> m_assembler;
};