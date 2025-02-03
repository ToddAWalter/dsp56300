#pragma once

#include "jitcacheentry.h"
#include "jitdspregs.h"
#include "jitdspregpool.h"
#include "jitmem.h"
#include "jitregtracker.h"
#include "jitruntimedata.h"
#include "jitstackhelper.h"
#include "jittypes.h"
#include "jitconfig.h"

#include <vector>
#include <set>

namespace asmjit
{
	inline namespace ASMJIT_ABI_NAMESPACE
	{
		class CodeHolder;
	}
}

namespace dsp56k
{
	struct JitBlockInfo;
	struct JitConfig;

	class DSP;
	class JitBlockChain;
	class JitBlockRuntimeData;
	class JitDspMode;

	class JitBlock final
	{
	public:

		JitBlock(JitEmitter& _a, DSP& _dsp, JitRuntimeData& _runtimeData, JitConfig&& _config);
		~JitBlock();

		static void getInfo(JitBlockInfo& _info, const DSP& _dsp, TWord _pc, const JitConfig& _config, const std::vector<JitCacheEntry>& _cache, const std::set<TWord>& _volatileP, const std::map<TWord, TWord>& _loopStarts, const std::set<TWord>& _loopEnds);

		bool emit(JitBlockRuntimeData& _rt, JitBlockChain* _chain, TWord _pc, const std::vector<JitCacheEntry>& _cache, const std::set<TWord>& _volatileP, const std::map<TWord, TWord>& _loopStarts, const std::set<TWord>& _loopEnds, bool _profilingSupport);

		JitEmitter& asm_() { return m_asm; }
		DSP& dsp() { return m_dsp; }
		JitStackHelper& stack() { return m_stack; }
		JitRegpool& gpPool() { return m_gpPool; }
		JitRegpool& xmmPool() { return m_xmmPool; }
		JitDspRegs& regs() { return m_dspRegs; }
		JitDspRegPool& dspRegPool() { return m_dspRegPool; }
		Jitmem& mem() { return m_mem; }
		const JitBlockRuntimeData* currentJitBlockRuntimeData() const { return m_currentJitBlockRuntimeData; }

		operator JitEmitter& ()		{ return m_asm;	}

		// JIT code writes these
		uint32_t& pMemWriteAddress() { return m_runtimeData.m_pMemWriteAddress; }
		uint32_t& pMemWriteValue() { return m_runtimeData.m_pMemWriteValue; }

		void setNextPC(const DspValue& _pc);

		void increaseInstructionCount(const asmjit::Operand& _count);
		void increaseCycleCount(const asmjit::Operand& _count);
		void increaseUint64(const asmjit::Operand& _count, const uint64_t& _target);

		const JitConfig& getConfig() const { return m_config; }

		AddressingMode getAddressingMode(uint32_t _aguIndex) const;
		const JitDspMode* getMode() const;
		void setMode(JitDspMode* _mode);

		void lockScratch()
		{
			assert(!m_scratchLocked && "scratch reg is already locked");
			m_scratchLocked = true;
		}

		void unlockScratch()
		{
			assert(m_scratchLocked && "scratch reg is not locked");
			m_scratchLocked = false;
		}

		void lockShift()
		{
			assert(!m_shiftLocked && "shift reg is already locked");
			m_shiftLocked = true;
		}

		void unlockShift()
		{
			assert(m_shiftLocked && "shift reg is not locked");
			m_shiftLocked = false;
		}

		void reset(JitConfig&& _config);

	private:
		class JitBlockGenerating
		{
		public:
			JitBlockGenerating(JitBlockRuntimeData& _block);
			~JitBlockGenerating();
		private:
			JitBlockRuntimeData& m_block;
		};

		JitReg64 getJumpTarget(const JitReg64& _dst, const JitBlockRuntimeData* _child) const;

		void jumpToChild(const JitBlockRuntimeData* _child, JitCondCode _cc = JitCondCode::kMaxValue) const;
		void jumpToOneOf(JitCondCode _ccTrue, const JitBlockRuntimeData* _childTrue, const JitBlockRuntimeData* _childFalse) const;

		JitRuntimeData& m_runtimeData;

		JitEmitter& m_asm;
		DSP& m_dsp;
		JitStackHelper m_stack;
		JitRegpool m_xmmPool;
		JitRegpool m_gpPool;
		JitDspRegs m_dspRegs;
		JitDspRegPool m_dspRegPool;
		Jitmem m_mem;

		JitConfig m_config;
		JitBlockChain* m_chain = nullptr;

		bool m_scratchLocked = false;
		bool m_shiftLocked = false;

		JitDspMode* m_mode = nullptr;
		JitBlockRuntimeData* m_currentJitBlockRuntimeData = nullptr;
	};
}
