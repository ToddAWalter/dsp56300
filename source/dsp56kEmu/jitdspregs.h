#pragma once

#include "jittypes.h"
#include "types.h"
#include "asmjit/x86/x86features.h"

namespace asmjit
{
	namespace x86
	{
		class Assembler;
	}
}

namespace dsp56k
{
	class JitBlock;
	class DSP;

	class JitDspRegs
	{
	public:
		JitDspRegs(JitBlock& _block);

		~JitDspRegs();

		template<typename T>
		void getR(const T& _dst, int _agu)
		{
			if(!isLoaded(LoadedRegR0 + _agu))
				loadAGU(_agu);

			m_asm.movd(_dst, asmjit::x86::xmm(xmmR0+_agu));
		}

		template<typename T>
		void getN(const T& _dst, int _agu)
		{
			if(!isLoaded(LoadedRegR0 + _agu))
				loadAGU(_agu);

			const auto xm(asmjit::x86::xmm(xmmR0+_agu));

			if(asmjit::CpuInfo::host().hasFeature(asmjit::x86::Features::kSSE4_1))
			{
				m_asm.pextrd(_dst, xm, asmjit::Imm(1));
			}
			else
			{
				m_asm.pshufd(xm, xm, asmjit::Imm(0xe1));		// swap lower two words to get N in word 0
				m_asm.movd(_dst, xm);
				m_asm.pshufd(xm, xm, asmjit::Imm(0xe1));		// swap back
			}
		}

		template<typename T>
		void getM(const T& _dst, int _agu)
		{
			if(!isLoaded(LoadedRegR0 + _agu))
				loadAGU(_agu);

			const auto xm(asmjit::x86::xmm(xmmR0+_agu));

			if(asmjit::CpuInfo::host().hasFeature(asmjit::x86::Features::kSSE4_1))
			{
				m_asm.pextrd(_dst, xm, asmjit::Imm(2));
			}
			else
			{
				m_asm.pshufd(xm, xm, asmjit::Imm(0xc6));		// swap words 0 and 2 to ret M in word 0
				m_asm.movd(_dst, xm);
				m_asm.pshufd(xm, xm, asmjit::Imm(0xc6));		// swap back
			}
		}

		void setR(int _agu, const JitReg64& _src);


		JitReg getPC();
		JitReg getSR();
		JitReg getLC();
		JitReg getExtMemAddr();
		void getALU(asmjit::x86::Gp _dst, int _alu);
		void setALU(int _alu, asmjit::x86::Gp _src);

		void getXY(asmjit::x86::Gp _dst, int _xy);
		void getXY0(const JitReg& _dst, uint32_t _aluIndex);
		void getXY1(const JitReg& _dst, uint32_t _aluIndex);
		void getALU0(const JitReg& _dst, uint32_t _aluIndex);
		void getALU1(const JitReg& _dst, uint32_t _aluIndex);
		void getALU2(const JitReg& _dst, uint32_t _aluIndex);

		void getEP(const JitReg32& _dst);
		void getVBA(const JitReg32& _dst);
		void getSC(const JitReg32& _dst);
		void getSZ(const JitReg32& _dst);
		void getSR(const JitReg32& _dst);
		void getOMR(const JitReg32& _dst);
		void getSP(const JitReg32& _dst);
		void getSSH(const JitReg32& _dst);
		void getSSL(const JitReg32& _dst);
		void getLA(const JitReg32& _dst) const;
		void getLC(const JitReg32& _dst) const;

		void getSS(const JitReg64& _dst);

		void decSP() const;
		void incSP() const;

	private:
		enum LoadedRegs
		{
			LoadedRegR0,	LoadedRegR1,	LoadedRegR2,	LoadedRegR3,	LoadedRegR4,	LoadedRegR5,	LoadedRegR6,	LoadedRegR7,
			LoadedRegA,		LoadedRegB,
			LoadedRegX,		LoadedRegY,
			LoadedRegLC,	LoadedRegExtMem,
			LoadedRegSR,	LoadedRegPC,
		};

		void loadDSPRegs();
		void storeDSPRegs();

		void loadAGU(int _agu);
		void loadALU(int _alu);
		void loadXY(int _xy);

		void storeAGU(int _agu);
		void storeALU(int _alu);
		void storeXY(int _xy);

		void load24(const asmjit::x86::Gp& _dst, TReg24& _src) const;
		void store24(TReg24& _dst, const asmjit::x86::Gp& _src) const;

		bool isLoaded(const uint32_t _reg) const		{ return m_loadedRegs & (1<<_reg); }
		void setLoaded(const uint32_t _reg)				{ m_loadedRegs |= (1<<_reg); }
		void setUnloaded(const uint32_t _reg)			{ m_loadedRegs &= ~(1<<_reg); }

		JitBlock& m_block;
		asmjit::x86::Assembler& m_asm;
		DSP& m_dsp;

		uint32_t m_loadedRegs = 0;
	};
}
