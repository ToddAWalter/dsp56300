#include "jitdspregs.h"

#include "jitmem.h"
#include "jitblock.h"

#include "dsp.h"

using namespace asmjit;
using namespace x86;

namespace dsp56k
{
	JitDspRegs::JitDspRegs(JitBlock& _block): m_block(_block), m_asm(_block.asm_()), m_dsp(_block.dsp())
	{
	}

	JitDspRegs::~JitDspRegs()
	{
		storeDSPRegs();
	}

	void JitDspRegs::loadAGU(int _agu)
	{
		// TODO: we can do better by reordering the DSP registers in memory so that we can load one 128 bit word at once
		const auto xm = xmm(xmmR0 + _agu);
		auto& mem = m_block.mem();
		mem.mov(xm, m_dsp.regs().m[_agu]);
		m_asm.pslldq(xm, Imm(4));

		const RegXMM xmmTemp(m_block.xmmPool());
		
		mem.mov(xmmTemp.get(), m_dsp.regs().n[_agu]);
		m_asm.movss(xm, xmmTemp.get());
		m_asm.pslldq(xm, Imm(4));

		mem.mov(xmmTemp.get(), m_dsp.regs().r[_agu]);
		m_asm.movss(xm, xmmTemp.get());

		setLoaded(LoadedRegR0 + _agu);
	}

	void JitDspRegs::loadALU(int _alu)
	{
		auto& alu = _alu ? m_dsp.regs().b : m_dsp.regs().a;
		
		m_block.mem().mov(xmm(xmmA + _alu), alu);

		setLoaded(LoadedRegA + _alu);
	}

	void JitDspRegs::loadXY(int _xy)
	{
		auto& xy = _xy ? m_dsp.regs().y : m_dsp.regs().x;

		m_block.mem().mov(xmm(xmmX + _xy), xy);

		setLoaded(LoadedRegX + _xy);
	}

	void JitDspRegs::storeAGU(int _agu)
	{
		const auto xm = xmm(xmmR0 + _agu);
		auto& mem = m_block.mem();

		mem.mov(m_dsp.regs().r[_agu], xm);
		m_asm.psrldq(xm, Imm(4));

		mem.mov(m_dsp.regs().n[_agu], xm);
		m_asm.psrldq(xm, Imm(4));

		mem.mov(m_dsp.regs().m[_agu], xm);

		setUnloaded(LoadedRegR0 + _agu);
	}

	void JitDspRegs::storeALU(int _alu)
	{
		auto& alu = _alu ? m_dsp.regs().b : m_dsp.regs().a;

		m_block.mem().mov(alu, xmm(xmmA + _alu));

		setUnloaded(LoadedRegA + _alu);
	}

	void JitDspRegs::storeXY(int _xy)
	{
		auto& xy = _xy ? m_dsp.regs().x : m_dsp.regs().y;

		m_block.mem().mov(xy, xmm(xmmX + _xy));

		setUnloaded(LoadedRegX + _xy);
	}
	void JitDspRegs::load24(const Gp& _dst, TReg24& _src)
	{
		m_block.mem().mov(_dst, _src);
	}
	void JitDspRegs::store24(TReg24& _dst, const Gp& _src)
	{
		m_block.mem().mov(_dst, _src);
	}

	Gp JitDspRegs::getPC()
	{
		if(!isLoaded(LoadedRegPC))
		{
			load24(regPC, m_dsp.regs().pc);
			setLoaded(LoadedRegPC);
		}

		return regPC;
	}

	Gp JitDspRegs::getSR()
	{
		if(!isLoaded(LoadedRegSR))
		{
			load24(regSR, m_dsp.regs().sr);
			setLoaded(LoadedRegSR);
		}

		return regSR;
	}

	Gp JitDspRegs::getLC()
	{
		if(!isLoaded(LoadedRegLC))
		{
			load24(regLC, m_dsp.regs().lc);			
			setLoaded(LoadedRegLC);
		}

		return regLC;
	}

	Gp JitDspRegs::getLA()
	{
		if(!isLoaded(LoadedRegLA))
		{
			load24(regLA, m_dsp.regs().la);
			setLoaded(LoadedRegLA);			
		}

		return regLA;
	}

	void JitDspRegs::getALU(const Gp _dst, const int _alu)
	{
		if(!isLoaded(LoadedRegA + _alu))
			loadALU(_alu);

		m_asm.movq(_dst, xmm(xmmA + _alu));
	}

	void JitDspRegs::setALU(int _alu, Gp _src)
	{
		assert(isLoaded(LoadedRegA + _alu));
		m_asm.movq(xmm(xmmA + _alu), _src);
	}

	void JitDspRegs::getXY(Gp _dst, int _xy)
	{
		if(!isLoaded(LoadedRegX + _xy))
			loadXY(_xy);

		m_asm.movq(_dst, xmm(xmmX + _xy));
	}

	void JitDspRegs::loadDSPRegs()
	{
		for(auto i=0; i<8; ++i)
			loadAGU(i);

		loadALU(0);
		loadALU(1);

		loadXY(0);
		loadXY(1);
	}

	void JitDspRegs::storeDSPRegs()
	{
		// TODO: we should skip all registers that have not been written but only read

		for(auto i=0; i<8; ++i)
		{
			if(isLoaded(LoadedRegR0 + i))
				storeAGU(i);
		}

		if(isLoaded(LoadedRegA))			storeALU(0);
		if(isLoaded(LoadedRegB))			storeALU(1);

		if(isLoaded(LoadedRegX))			storeXY(0);
		if(isLoaded(LoadedRegY))			storeXY(1);

		if(isLoaded(LoadedRegPC))			store24(m_dsp.regs().pc, regPC);
		if(isLoaded(LoadedRegSR))			store24(m_dsp.regs().sr, regSR);
		if(isLoaded(LoadedRegLC))			store24(m_dsp.regs().lc, regLC);
		if(isLoaded(LoadedRegLA))			store24(m_dsp.regs().la, regLA);
	}
}