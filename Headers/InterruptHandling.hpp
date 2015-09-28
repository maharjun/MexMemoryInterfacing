#ifndef INTERRUPT_HANDLING_HPP
#define INTERRUPT_HANDLING_HPP

void EnableInterruptHandling();
void DisableInterruptHandling();
void ExeInterruptHandler(int SIGNAL_TYPE);
bool IsProgramInterrupted();
void ResetInterrupt();

#endif