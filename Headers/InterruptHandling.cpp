#include <csignal>
#include "InterruptHandling.hpp"

extern "C" bool utIsInterruptPending();
extern "C" bool utSetInterruptPending(bool);
extern "C" bool utSetInterruptEnabled(bool);

bool __IS_PROGRAM_INTERRUPTED;

void EnableInterruptHandling() {
	#ifdef MEX_EXE
		std::signal(SIGINT        , ExeInterruptHandler);
	#elif defined MEX_LIB
		utSetInterruptEnabled(true);
	#endif
}

void DisableInterruptHandling() {
	#ifdef MEX_EXE
		std::signal(SIGINT, SIG_IGN);
	#elif defined MEX_LIB
		utSetInterruptEnabled(false);
	#endif
}

void ExeInterruptHandler(int SIGNAL_TYPE) {
	if (SIGNAL_TYPE == SIGINT) {
		__IS_PROGRAM_INTERRUPTED = true;
	}
}

bool IsProgramInterrupted() {
	#ifdef MEX_EXE
		return __IS_PROGRAM_INTERRUPTED;
	#elif defined MEX_LIB
		return utIsInterruptPending();
	#endif
}

void ResetInterrupt() {
	#ifdef MEX_EXE
		__IS_PROGRAM_INTERRUPTED = false;
	#elif defined MEX_LIB
		utSetInterruptPending(false);
	#endif
}