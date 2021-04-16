#pragma once

#if defined(_WIN32) && defined(_DEBUG)
#	define USE_MOTOROLA_UNASM
#endif

#include <sstream>
#include <string>

#include "opcodes.h"

namespace dsp56k
{
	struct OpcodeInfo;
	
	class Disassembler
	{
	public:
		Disassembler();

		int disassemble(std::string& dst, TWord op, TWord opB, TWord sr, TWord omr);
	private:
		int disassembleAlu(std::string& _dst, const OpcodeInfo& _oiAlu, TWord op);
		int disassembleAlu(Instruction inst, TWord op);
		int disassembleParallelMove(const OpcodeInfo& oi, TWord _op, TWord _opB);
		int disassemble(const OpcodeInfo& oi, TWord op, TWord opB);
		void disassembleDC(std::string& dst, TWord op);

		int disassembleNonParallel(const OpcodeInfo& oi, TWord op, TWord opB, const TWord sr, const TWord omr);

		std::string mmmrrr(TWord mmmrrr, TWord S, TWord opB, bool _addMemSpace = true, bool _long = true);

		std::stringstream m_ss;
		std::string m_str;
		Opcodes m_opcodes;
	};

	uint32_t disassembleMotorola( char* _dst, TWord _opA, TWord _opB, TWord _sr, TWord _omr);
	uint32_t disassemble( char* _dst, TWord _opA, TWord _opB, TWord _sr, TWord _omr);
}
