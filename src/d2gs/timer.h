#ifndef INCLUDED_TIMER_H
#define INCLUDED_TIMER_H


namespace pvpgn
{

	namespace d2gs
	{

		/* functions */
		int D2GSTimerInitialize(void);
		int CleanupRoutineForTimer(void);
		DWORD WINAPI D2GSTimerProcessor(LPVOID lpParameter);

	}

}


#endif /* INCLUDED_TIMER_H */