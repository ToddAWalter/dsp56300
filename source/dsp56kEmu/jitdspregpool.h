#pragma once

#include <list>
#include <map>
#include <set>

#include "jitregtypes.h"

namespace dsp56k
{
	class JitBlock;

	class JitDspRegPool
	{
	public:
		enum DspReg
		{
			DspR0,	DspR1,	DspR2,	DspR3,	DspR4,	DspR5,	DspR6,	DspR7,
			DspN0,	DspN1,	DspN2,	DspN3,	DspN4,	DspN5,	DspN6,	DspN7,
			DspM0,	DspM1,	DspM2,	DspM3,	DspM4,	DspM5,	DspM6,	DspM7,

			DspA,	DspB,
			DspX,	DspY,

			DspExtMem,
			DspSR,
			DspLC,
			DspLA,
			DspCount
		};

		JitDspRegPool(JitBlock& _block);
		~JitDspRegPool();

		JitReg get(DspReg _reg, bool _read, bool _write);

		void read(const JitReg& _dst, DspReg _src);
		void write(DspReg _dst, const JitReg& _src);

		void lock(DspReg _reg);
		void unlock(DspReg _reg);

		void storeAll();

		bool hasWrittenRegs() const { return !m_writtenDspRegs.empty(); }

		static constexpr JitReg m_gps[] =	{ asmjit::x86::r8, asmjit::x86::r9, asmjit::x86::r10, asmjit::x86::r11};

		static constexpr JitReg128 m_xmms[] =	{ asmjit::x86::xmm0, asmjit::x86::xmm1, asmjit::x86::xmm2, asmjit::x86::xmm3
												, asmjit::x86::xmm4, asmjit::x86::xmm5, asmjit::x86::xmm6, asmjit::x86::xmm7
												, asmjit::x86::xmm8, asmjit::x86::xmm9, asmjit::x86::xmm10, asmjit::x86::xmm11};

		static constexpr uint32_t m_gpCount = sizeof(m_gps) / sizeof(m_gps[0]);
		static constexpr uint32_t m_xmmCount = sizeof(m_xmms) / sizeof(m_xmms[0]);

	private:
		void makeSpace();
		void clear();

		void load(JitReg& _dst, DspReg _src);
		void store(DspReg _dst, JitReg& _src);
		void store(DspReg _dst, JitReg128& _src);

		JitBlock& m_block;

		std::list<JitReg> m_availableGps;
		std::list<JitReg128> m_availableXmms;

		std::set<DspReg> m_lockedGps;

		std::list<DspReg> m_usedGps;
		std::map<DspReg, JitReg> m_usedGpsMap;

		std::list<DspReg> m_usedXmms;
		std::map<DspReg, JitReg128> m_usedXmmMap;

		std::set<DspReg> m_writtenDspRegs;
	};
}
