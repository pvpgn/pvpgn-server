#include <windows.h>
#include "d2gs.h"
#include "eventlog.h"
#include "d2gamelist.h"
#include "net.h"
#include "timer.h"
#include "vars.h"
#include "watchdog.h"
#include "server.h"


namespace pvpgn
{

	namespace d2gs
	{

		/* vars */
		static HANDLE	hStopEvent = NULL;
		static HANDLE	ghTimerThread = NULL;


		/* functions */
		extern void		D2GSResetWatchDogCounter();


		/*********************************************************************
		 * Purpose: to initialize the timer
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSTimerInitialize(void)
		{
			DWORD	dwThreadId;

			/* create stop event */
			hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!hStopEvent)
			{
				D2GSEventLog("D2GSTimerInitialize",
					"Failed in creating event object. Code: %lu", GetLastError());
				return FALSE;
			}

			/* create the working thread */
			ghTimerThread = CreateThread(NULL, 0, D2GSTimerProcessor, NULL, 0, &dwThreadId);
			if (!ghTimerThread)
			{
				D2GSEventLog("D2GSTimerInitialize",
					"Can't CreateThread D2GSTimerProcessor. Code: %lu", GetLastError());
				CleanupRoutineForTimer();
				return FALSE;
			}

			/* add to the cleanup routine list */
			if (CleanupRoutineInsert(CleanupRoutineForTimer, "D2GS Timer"))
			{
				return TRUE;
			}
			else
			{
				/* do some cleanup before quiting */
				CleanupRoutineForTimer();
				return FALSE;
			}

		} /* End of D2GSTimerInitialize() */


		/*********************************************************************
		 * Purpose: to clearup the timer
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int CleanupRoutineForTimer(void)
		{
			if (hStopEvent)
			{
				SetEvent(hStopEvent);
				if (ghTimerThread)
				{
					WaitForSingleObject(ghTimerThread, INFINITE);
					CloseHandle(ghTimerThread);
					ghTimerThread = NULL;
				}
				CloseHandle(hStopEvent);
				hStopEvent = NULL;
			}
			return TRUE;

		} /* End of CleanupRoutineForTimer() */


		/*********************************************************************
		 * Purpose: resets watchdog counter when called 20 times
		 * Return: none
		 *********************************************************************/
		static void D2GSCheckTimerCount(void)
		{
			static DWORD count = 0;
			count++;
			if (count >= 20)
			{
				count = 0;
				EnterCriticalSection(&csGameList);
				D2GSResetWatchDogCounter();
				LeaveCriticalSection(&csGameList);
			}
		}


		/*********************************************************************
		 * Purpose: timer processor
		 * Return: return value of the thread
		 *********************************************************************/
		DWORD WINAPI D2GSTimerProcessor(LPVOID lpParameter)
		{
			DWORD	dwWait;

			while (TRUE)
			{
				dwWait = WaitForSingleObject(hStopEvent, TIMER_TICK_IN_MS);
				if (dwWait == WAIT_FAILED)
				{
					D2GSEventLog("D2GSTimerProcessor",
						"WaitForSingleObject failed. Code: %lu", GetLastError());
					continue;
				}
				else if (dwWait == WAIT_OBJECT_0)
				{
					/* stop event be set, quit */
					D2GSEventLog("D2GSTimerProcessor", "Terminate timer thread");
					return TRUE;
				}
				else if (dwWait == WAIT_TIMEOUT)
				{
					/* a tick passed, call the routine to do something */
					D2GSPendingCharTimerRoutine();
					D2GSGetDataRequestTimerRoutine();
					D2GSCalculateNetStatistic();
					D2GSSendMOTD();
					D2GSCheckGameLife();
					D2GSCheckTimerCount();
					D2GSShutdownTimer();
				}
				else
				{
					continue;
				}
			}
			return 0;

		} /* End of D2GSTimerProcessor() */

	}

}