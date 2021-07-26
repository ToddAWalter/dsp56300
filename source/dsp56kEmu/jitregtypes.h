#pragma once

#include "jittypes.h"

namespace dsp56k
{
#ifdef _MSC_VER
	static constexpr JitReg64 g_funcArgGPs[] = {asmjit::x86::rcx, asmjit::x86::rdx, asmjit::x86::r8, asmjit::x86::r9};

	static constexpr JitReg64 g_nonVolatileGPs[] = { asmjit::x86::rbx, asmjit::x86::rbp, asmjit::x86::rdi, asmjit::x86::rsi// not needed, asmjit::x86::rsp
	                                               , asmjit::x86::r12, asmjit::x86::r13, asmjit::x86::r14, asmjit::x86::r15};

	static constexpr JitReg128 g_nonVolatileXMMs[] = { asmjit::x86::xmm6, asmjit::x86::xmm7, asmjit::x86::xmm8, asmjit::x86::xmm9, asmjit::x86::xmm10, asmjit::x86::xmm11, asmjit::x86::xmm12, asmjit::x86::xmm13, asmjit::x86::xmm14, asmjit::x86::xmm15 };

	static constexpr JitRegGP g_dspPoolGps[] =		{ asmjit::x86::rdx, asmjit::x86::r8, asmjit::x86::r9, asmjit::x86::r10, asmjit::x86::r11, asmjit::x86::rsi, asmjit::x86::rdi};

#else
	static constexpr JitReg64 g_funcArgGPs[] = { asmjit::x86::rdi, asmjit::x86::rsi, asmjit::x86::rdx, asmjit::x86::rcx };

	// Note: rcx is not used in any pools because it is needed as shift register

	static constexpr JitReg64 g_nonVolatileGPs[] = { asmjit::x86::rbx, asmjit::x86::rbp, asmjit::x86::rsi// not needed, asmjit::x86::rsp
	                                               , asmjit::x86::r12, asmjit::x86::r13, asmjit::x86::r14, asmjit::x86::r15};
	
	static constexpr JitReg128 g_nonVolatileXMMs[] = { asmjit::x86::xmm6, asmjit::x86::xmm7, asmjit::x86::xmm8, asmjit::x86::xmm9, asmjit::x86::xmm10, asmjit::x86::xmm11, asmjit::x86::xmm12, asmjit::x86::xmm13, asmjit::x86::xmm14, asmjit::x86::xmm15 };

	static constexpr JitRegGP g_dspPoolGps[] =		{ asmjit::x86::rdx, asmjit::x86::r8, asmjit::x86::r9, asmjit::x86::r10, asmjit::x86::r11, asmjit::x86::rsi};
	
#endif

	static constexpr auto regReturnVal = asmjit::x86::rax;

	static constexpr std::initializer_list<JitReg> g_regGPTemps = { asmjit::x86::r12, asmjit::x86::r13, asmjit::x86::r14, asmjit::x86::r15, asmjit::x86::rbp };

	static constexpr auto regLastModAlu = asmjit::x86::xmm0;

	static constexpr auto regXMMTempA = asmjit::x86::xmm1;

	static constexpr JitReg128 g_dspPoolXmms[] =	{ asmjit::x86::xmm2, asmjit::x86::xmm3,  asmjit::x86::xmm4,  asmjit::x86::xmm5,  asmjit::x86::xmm6,  asmjit::x86::xmm7,  asmjit::x86::xmm8,
													  asmjit::x86::xmm9, asmjit::x86::xmm10, asmjit::x86::xmm11, asmjit::x86::xmm12, asmjit::x86::xmm13, asmjit::x86::xmm14, asmjit::x86::xmm15};

}
