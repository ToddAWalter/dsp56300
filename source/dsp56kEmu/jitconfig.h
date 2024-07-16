#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include "types.h"

namespace dsp56k
{
	struct JitConfig
	{
		bool aguSupportBitreverse = false;
		bool aguSupportMultipleWrapModulo = true;
		bool cacheSingleOpBlocks = true;
		bool linkJitBlocks = true;
		bool splitOpsByNops = false;
		bool dynamicPeripheralAddressing = false;
		bool debugDynamicPeripheralAddressing = false;	// x86-64 only: Will issue int3() = breakpoint interrupt if a memory address is detected that points to peripherals but DPA is disabled
		uint32_t maxInstructionsPerBlock = 0;
		bool memoryWritesCallCpp = false;
		bool support16BitSCMode = false;

		bool asmjitDiagnostics = false;
		uint32_t maxDoIterations = 0;	// maximum number of iterations of a do loop before the Jit block is exited (and later re-entered), giving a time slice for interrupts/peripherals

		// retrieves a JitConfig for a specific PC. If null, the global default config is used
		std::function<std::optional<JitConfig>(TWord)> getBlockConfig;
	};
}
