#include "watchdog.h"

#include <Windows.h>

#include "eventlog.h"
#include "vars.h"


namespace pvpgn
{

	namespace d2gs
	{

		static DWORD CheckWatchDogCounter(void);
		static DWORD WINAPI D2GSWatchDogThread(LPVOID p);


		static HANDLE hWatchDog = NULL;
		static CRITICAL_SECTION csWatchDog;
		static DWORD dwWatchDogCounter = 0;


		void D2GSWatchDogInit(void)
		{
			DWORD dwThreadId = 0;
			hWatchDog = 0;
			InitializeCriticalSection(&csWatchDog);
			hWatchDog = CreateThread(0, 0, D2GSWatchDogThread, NULL, 0, &dwThreadId);
			if (!hWatchDog)
			{
				DWORD error = GetLastError();
				D2GSEventLog("watchdog_init", "CreateThread watchdog_thread. Code: %lu", error);
				return;
			}
			CloseHandle(hWatchDog);
			D2GSEventLog("watchdog_init", "CreateThread watchdog_thread, %lu", dwThreadId);
			return;
		}

		void D2GSResetWatchDogCounter(void)
		{
			EnterCriticalSection(&csWatchDog);
			dwWatchDogCounter = 0;
			LeaveCriticalSection(&csWatchDog);
		}

		static DWORD CheckWatchDogCounter(void)
		{
			EnterCriticalSection(&csWatchDog);
			dwWatchDogCounter++;
			if (dwWatchDogCounter < 0xF)
			{
				LeaveCriticalSection(&csWatchDog);
				return 0;
			}
			LeaveCriticalSection(&csWatchDog);
			return 1;
		}

		static DWORD WINAPI D2GSWatchDogThread(LPVOID p)
		{
			while (TRUE)
			{
				Sleep(6000);
				if (CheckWatchDogCounter() != 0)
				{
					break;
				}
				if (D2GSCheckTickCount && D2GSCheckTickCount(0) != 0)
				{
					break;
				}
			}
			d2gsconf.enablegslog = TRUE;
			D2GSEventLog("watchdog_thread", "D2GS maybe in deadlock, restart it");
			d2gsconf.enablegslog = FALSE;
			D2GSShutdown(0);
			return 0;
		}

	}

}