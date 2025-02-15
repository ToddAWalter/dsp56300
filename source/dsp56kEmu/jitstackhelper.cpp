#include "jitstackhelper.h"

#include "jitblock.h"
#include "jitemitter.h"
#include "jitblockruntimedata.h"

#include "dspassert.h"

#include "asmjit/core/builder.h"

#include <algorithm>	// std::sort

#include "dsp.h"

namespace dsp56k
{
	constexpr size_t g_stackAlignmentBytes = 16;
#ifdef HAVE_ARM64
	constexpr size_t g_functionCallSize = 0;
	constexpr auto g_stackReg = asmjit::a64::regs::sp;
#else
	constexpr size_t g_functionCallSize = 8;
	constexpr auto g_stackReg = asmjit::x86::rsp;
#endif
#ifdef _MSC_VER
	constexpr size_t g_shadowSpaceSize = 32;
#else
	constexpr size_t g_shadowSpaceSize = 0;
#endif

	JitStackHelper::JitStackHelper(JitBlock& _block) : m_block(_block)
	{
		m_pushedRegs.reserve(32);
		m_usedRegs.reserve(32);
	}

	JitStackHelper::~JitStackHelper()
	{
		popAll();
	}

	void JitStackHelper::push(const JitReg64& _reg)
	{
		PushedReg reg;
		m_block.asm_().push(_reg);
		reg.reg = _reg;

		m_pushedBytes += pushSize(_reg);
		reg.stackOffset = m_pushedBytes;

		m_pushedRegs.push_back(reg);
	}

	void JitStackHelper::push(const JitReg128& _reg)
	{
		PushedReg reg;
		stackRegSub(pushSize(_reg));
		m_block.asm_().movq(ptr(g_stackReg), _reg);
		reg.reg = _reg;

		m_pushedBytes += pushSize(_reg);
		reg.stackOffset = m_pushedBytes;

		m_pushedRegs.push_back(reg);
	}

	void JitStackHelper::pop(const JitReg64& _reg)
	{
		assert(!m_pushedRegs.empty());
		assert(m_pushedBytes >= pushSize(_reg));

		m_pushedRegs.pop_back();
		m_pushedBytes -= pushSize(_reg);

		m_block.asm_().pop(_reg);
	}

	void JitStackHelper::pop(const JitReg128& _reg)
	{
		assert(!m_pushedRegs.empty());
		assert(m_pushedBytes >= pushSize(_reg));

		m_pushedRegs.pop_back();
		m_pushedBytes -= pushSize(_reg);

		m_block.asm_().movq(_reg, ptr(g_stackReg));
		stackRegAdd(pushSize(_reg));
	}

	void JitStackHelper::pop(const JitReg& _reg)
	{
		if(_reg.isGp())
			pop(_reg.as<JitReg64>());
		else if(_reg.isVec())
			pop(_reg.as<JitReg128>());
		else
			assert(false && "unknown register type");
	}

	void JitStackHelper::pop()
	{
		const auto reg = m_pushedRegs.back();
		pop(reg.reg);
	}

	void JitStackHelper::popAll()
	{
		// we push stack-relative if there is at least one used vector register as we can save a bunch of instructions this way
		bool haveVectors = false;
		for (const auto& r : m_pushedRegs)
		{
			if(r.reg.isVec())
			{
				haveVectors = true;
				break;
			}
		}

		if(haveVectors)
		{
			// sort in order of memory address
			std::sort(m_pushedRegs.begin(), m_pushedRegs.end());

			for(size_t i=0; i<m_pushedRegs.size(); ++i)
			{
				const auto& r = m_pushedRegs[i];
				const int offset = static_cast<int>(m_pushedBytes - m_pushedRegs[i].stackOffset);

				const auto memPtr = ptr(g_stackReg, offset);

				if(r.reg.isVec())
				{
					m_block.asm_().movq(r.reg.as<JitReg128>(), memPtr);
				}
				else
				{
#ifdef HAVE_ARM64
					m_block.asm_().ldr(r64(r.reg.as<JitRegGP>()), memPtr);
#else
					m_block.asm_().mov(r64(r.reg.as<JitRegGP>()), memPtr);
#endif
				}
			}

			stackRegAdd(m_pushedBytes);
			m_pushedBytes = 0;

			m_pushedRegs.clear();
		}
		else
		{
			while (!m_pushedRegs.empty())
				pop();
		}
	}

	void JitStackHelper::pushAllUsed(asmjit::BaseNode* _baseNode)
	{
		m_block.asm_().setCursor(_baseNode);

		std::vector<JitReg> regsToPush;
		regsToPush.reserve(m_usedRegs.size());

		uint32_t bytesToPush = 0;
		bool hasVectors = false;

		JitReg64 lastReg;

		for (const auto& reg : m_usedRegs)
		{
			if(!isNonVolatile(reg))
				continue;

			regsToPush.push_back(reg);
			bytesToPush += pushSize(reg);

			if(reg.isVec())
				hasVectors = true;
			else
				lastReg = reg.as<JitReg64>();
		}

		// push the last one again to fix alignment. Only needed if we have function calls inbetween as the stack needs to be aligned for this purpose only
		const auto needsAlignment = m_callCount && (bytesToPush & (g_stackAlignmentBytes-1)) != 0;

		if(needsAlignment)
		{
			regsToPush.push_back(lastReg);
			bytesToPush += pushSize(lastReg);
		}

		if(hasVectors)
		{
			stackRegSub(bytesToPush);
			m_pushedBytes += bytesToPush;

			int32_t offset = 0;

			for (const auto & reg : regsToPush)
			{
				const auto size = pushSize(reg);
				const auto memPtr = ptr(g_stackReg, offset);
				m_pushedRegs.push_back({m_pushedBytes - offset, reg});
				offset += static_cast<int32_t>(size);
				if(reg.isVec())
				{
					m_block.asm_().movq(memPtr, reg.as<JitReg128>());
				}
				else
				{
					m_block.asm_().mov(memPtr, r64(reg.as<JitRegGP>()));
				}
			}
		}
		else
		{
			for (auto reg : regsToPush)
			{
				if(reg.isVec())
					push(reg.as<JitReg128>());
				else
					push(reg.as<JitReg64>());
			}
		}
		m_block.asm_().setCursor(m_block.asm_().lastNode());
	}
	void JitStackHelper::call(const void* _funcAsPtr)
	{
		call([&]()
		{
			m_block.asm_().call(_funcAsPtr);
		});
	}

	void JitStackHelper::call(const std::function<void()>& _execCall)
	{
		PushBeforeFunctionCall backup(m_block);

		const auto usedSize = m_pushedBytes + g_functionCallSize;
		const auto alignedStack = (usedSize + g_stackAlignmentBytes-1) & ~(g_stackAlignmentBytes-1);

		const auto offset = alignedStack - usedSize + g_shadowSpaceSize;

		stackRegSub(offset);

		_execCall();

		stackRegAdd(offset);

		++m_callCount;
	}

	bool JitStackHelper::isFuncArg(const JitRegGP& _gp, uint32_t _maxIndex/* = 255*/)
	{
		_maxIndex = std::min(static_cast<uint32_t>(std::size(g_funcArgGPs)), _maxIndex);

		for(uint32_t i=0; i<_maxIndex; ++i)
		{
			const auto& gp = g_funcArgGPs[i];

			if (gp.equals(r64(_gp)))
				return true;
		}
		return false;
	}

	bool JitStackHelper::isNonVolatile(const JitReg& _gp)
	{
		if(_gp.isGp())
			return isNonVolatile(_gp.as<JitRegGP>());
		if(_gp.isVec())
			return isNonVolatile(_gp.as<JitReg128>());
		return false;
	}

	bool JitStackHelper::isNonVolatile(const JitRegGP& _gp)
	{
		for (const auto& gp : g_nonVolatileGPs)
		{
			if(gp.equals(r64(_gp)))
				return true;
		}
		return false;
	}

	bool JitStackHelper::isNonVolatile(const JitReg128& _xm)
	{
		for (const auto& xm : g_nonVolatileXMMs)
		{
			if(xm.equals(_xm))
				return true;
		}
		return false;
	}

	void JitStackHelper::setUsed(const JitReg& _reg)
	{
		if(_reg.isGp())
			setUsed(_reg.as<JitRegGP>());
		else if(_reg.isVec())
			setUsed(_reg.as<JitReg128>());
		else
			assert(false && "unknown register type");
	}

	void JitStackHelper::setUsed(const JitRegGP& _reg)
	{
		if(isUsed(_reg))
			return;

		m_usedRegs.push_back(_reg);
	}

	void JitStackHelper::setUsed(const JitReg128& _reg)
	{
		if(isUsed(_reg))
			return;

		m_usedRegs.push_back(_reg);
	}

	void JitStackHelper::setUnused(const JitReg& _reg)
	{
		for(size_t i=0; i<m_usedRegs.size(); ++i)
		{
			if(m_usedRegs[i].equals(_reg))
			{
				m_usedRegs.erase(m_usedRegs.begin() + i);
				return;
			}
		}
	}

	bool JitStackHelper::isUsed(const JitReg& _reg) const
	{
		for (const auto& usedReg : m_usedRegs)
		{
			if(usedReg.equals(_reg))
				return true;
		}
		return false;
	}

	uint32_t JitStackHelper::pushSize(const JitReg& _reg)
	{
#ifdef HAVE_ARM64
		return 16;
#else
		return _reg.size();
#endif
	}

	void JitStackHelper::reset()
	{
		m_usedRegs.clear();
		m_pushedRegs.clear();
		m_pushedBytes = 0;
		m_callCount = 0;
	}

	void JitStackHelper::registerFuncArg(const uint32_t _argIndex)
	{
		assert(m_usedFuncArgs.find(_argIndex) == m_usedFuncArgs.end());
		m_usedFuncArgs.insert(_argIndex);
	}

	void JitStackHelper::unregisterFuncArg(const uint32_t _argIndex)
	{
		assert(m_usedFuncArgs.find(_argIndex) != m_usedFuncArgs.end());
		m_usedFuncArgs.erase(_argIndex);
	}

	bool JitStackHelper::isUsedFuncArg(const JitRegGP& _reg) const
	{
		if(!isFuncArg(_reg))
			return false;

		const auto r = r64(_reg);

		for (const auto idx : m_usedFuncArgs)
		{
			if(g_funcArgGPs[idx].equals(r))
				return true;
		}
		return false;
	}

	void JitStackHelper::stackRegAdd(uint64_t offset) const
	{
		if (!offset)
			return;

#ifdef HAVE_ARM64
		m_block.asm_().add(g_stackReg, g_stackReg, asmjit::Imm(offset));
#else
		m_block.asm_().add(g_stackReg, asmjit::Imm(offset));
#endif
	}

	void JitStackHelper::stackRegSub(uint64_t offset) const
	{
		if (!offset)
			return;

#ifdef HAVE_ARM64
		m_block.asm_().sub(g_stackReg, g_stackReg, asmjit::Imm(offset));
#else
		m_block.asm_().sub(g_stackReg, asmjit::Imm(offset));
#endif
	}

	PushAllUsed::PushAllUsed(JitBlock& _block, bool _begin) : m_block(_block)
	{
		if(_begin)
			begin();
	}

	PushAllUsed::~PushAllUsed()
	{
		end();
	}

	void PushAllUsed::begin()
	{
		m_cursorBeforePushes = m_block.asm_().cursor();
	}

	void PushAllUsed::end()
	{
		if(!m_cursorBeforePushes)
			return;
		m_block.stack().pushAllUsed(m_cursorBeforePushes);
		m_cursorBeforePushes = nullptr;
	}
}
