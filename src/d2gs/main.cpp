/*
 * main.c: main routine of this program
 * 
 * 2001-08-20 faster
 *   add initialization routine and main loop of this program
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <conio.h>
#include <d2server.h>
#include "d2gs.h"
#include "eventlog.h"
#include "vars.h"
#include "config.h"
#include "d2ge.h"
#include "net.h"
#include "timer.h"
#include "telnetd.h"
#include "d2gamelist.h"
#include "handle_s2s.h"
#include "watchdog.h"
#include "server.h"

using namespace pvpgn::d2gs;
using namespace pvpgn;

/* function declarations */
BOOL D2GSCheckRunning(void);
void pvpgn::d2gs::D2GSShutdown(unsigned int exitCode);

/* CTRL+C or CTRL+Break signal handler */
BOOL WINAPI ControlHandler(DWORD dwCtrlType);


/* some variables used just in this file */
static DWORD			dwShutdownStatus = 0;
static DWORD			dwShutdownTickCount = 0;


/********************************************************************************
 * Main procedure begins here
 ********************************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD	dwWait;

	InitializeCriticalSection(&gsShutDown);

	/* init log system first */
	if (!D2GSEventLogInitialize()) return -1;

	d2gsconf.enablegslog = TRUE;
	D2GSEventLog("main", "Starting GS Server...");

	/* setup signal capture */
	SetConsoleCtrlHandler(ControlHandler, TRUE);

	/* check if another instance is running */
	if (D2GSCheckRunning()) {
		D2GSEventLog("main", "Seems another server is running");
		DoCleanup();
		return -1;
	}

	/* create a name event, for "d2gssvc" server controler to terminate me */
	hStopEvent= CreateEvent(NULL, TRUE, FALSE, D2GS_STOP_EVENT_NAME);
	if (!hStopEvent) {
		D2GSEventLog("main", "failed create stop event object");
		DoCleanup();
		return -1;
	}

	/* init variables */
	if (!D2GSVarsInitialize()) {
		D2GSEventLog("main", "Failed initialize global variables");
		DoCleanup();
		return -1;
	}

	/* read configurations */
	if (!D2GSReadConfig()) {
		D2GSEventLog("main", "Failed getting configurations from registry");
		DoCleanup();
		return -1;
	}

	/* create timer */
	if (!D2GSTimerInitialize()) {
		D2GSEventLog("main", "Failed Startup Timer");
		DoCleanup();
		return -1;
	}

	/* start up game engine */
	if (!D2GEStartup()) {
		D2GSEventLog("main", "Failed Startup Game Engine");
		D2GSEventLog("main", "Please enable GELog, and see the message");
		DoCleanup();
		return -1;
	}

	/* initialize the net connection */
	if (!D2GSNetInitialize()) {
		D2GSEventLog("main", "Failed Startup Net Connector");
		DoCleanup();
		return -1;
	}

	/* administration console */
	if (!D2GSAdminInitialize()) {
		D2GSEventLog("main", "Failed Startup Administration Console");
		DoCleanup();
		return -1;
	}

	/* create timer */
	D2GSWatchDogInit();
	if (!D2GSTimerInitialize())
	{
		D2GSEventLog("main", "Failed Startup Timer");
		DoCleanup();
		return -1;
	}

	/* main server loop */
	D2GSEventLog("main", "Entering Main Server Loop");
	while(TRUE) {
		dwWait = WaitForSingleObject(hStopEvent, 1000);
		if (dwWait!=WAIT_OBJECT_0) continue;
		/* service controler tell me to stop now. "Yes, sir!" */
		D2GSSetD2CSMaxGameNumber(0);
		D2GSActive(FALSE);
		d2gsconf.enablegslog = TRUE;
		D2GSEventLog("main", "I am going to stop");
		d2gsconf.enablegslog = TRUE;
		D2GSSaveAllGames(5000);
		d2gsconf.enablegslog = FALSE;
		Sleep(3000);
		D2GSShutdown(0);
		break;
	}

	/*DoCleanup();*/
	return 0;

} /* End of main() */


/*********************************************************************
 * Purpose: check if other instance is running
 * Return: TRUE(server is running) or FALSE(not running)
 *********************************************************************/
BOOL D2GSCheckRunning(void)
{
	HANDLE	hMutex;

	hD2GSMutex = NULL;
	hMutex = CreateMutex(NULL, TRUE, D2GSERVER_MUTEX_NAME);
	if (hMutex==NULL) {
		return TRUE;
	} else if (GetLastError()==ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex);
		return TRUE;
	} else {
		if (CleanupRoutineInsert(CleanupRoutineForServerMutex, "Server Mutex")) {
			hD2GSMutex = hMutex;
			return FALSE;
		} else {
			/* insert cleanup routine failed, assume server is running */
			CloseHandle(hMutex);
			return TRUE;
		}
	}

} /* End of D2GServerCheckRun() */


/*********************************************************************
 * Purpose: catch CTRL+C or CTRL+Break signal
 * Return: TRUE or FALSE
 *********************************************************************/
BOOL WINAPI ControlHandler(DWORD dwCtrlType)
{
	switch( dwCtrlType )
	{
		case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			D2GSEventLog("ControlHandler", "CTRL_BREAK or CTRL_C event caught");
			DoCleanup();
			ExitProcess(0);
			return TRUE;
			break;
    }
    return FALSE;

} /* End of ControlHandler */


/*********************************************************************
 * Purpose: to close the server mutex
 * Return: none
 *********************************************************************/
static void CloseServerMutex(void)
{
	if (hD2GSMutex) CloseHandle(hD2GSMutex);
	hD2GSMutex = NULL;

} /* End of CloseServerMutex() */


namespace pvpgn
{

	namespace d2gs
	{

		void D2GSBeforeShutdown(DWORD status, DWORD dwSecondsRemaining)
		{
			// GetTickCount milliseconds
			DWORD temp = 0;
			D2GSSetD2CSMaxGameNumber(0);
			D2GSActive(FALSE);
			EnterCriticalSection(&gsShutDown);
			dwShutdownStatus = status;
			dwShutdownTickCount = GetTickCount() + dwSecondsRemaining * 1000;
			switch (status)
			{
			case 1:
			case 3:
				D2GSEventLog("D2GSShutdown", "Restart GS in %lu seconds", dwSecondsRemaining);
				break;
			case 2:
			case 4:
				D2GSEventLog("D2GSShutdown", "Shutdown GS in %lu seconds", dwSecondsRemaining);
				break;
			}
			LeaveCriticalSection(&gsShutDown);
		}


		DWORD D2GSGetShutdownStatus(void)
		{
			DWORD status = 0;
			EnterCriticalSection(&gsShutDown);
			status = dwShutdownStatus;
			LeaveCriticalSection(&gsShutDown);
			return status;
		}


		void D2GSShutdown(unsigned int exitCode)
		{
			CloseServerMutex();

			if (D2GSCheckGameInfo())
			{
				D2GSSaveAllGames(5000);
				Sleep(3000);
			}

			if (bGERunning != 0)
			{
				if (D2GSAfterEnd() != 0)
				{
					D2GSAfterEnd();
				}
			}

			ExitProcess(exitCode);
		}


		void D2GSShutdownTimer(void)
		{
			static DWORD ShutdownCount = 0;
			char buffer[256] = { 0 };
			DWORD curTickCount = 0;
			DWORD overSeconds = 0;
			ShutdownCount++;
			if (ShutdownCount >= 0xA)
			{
				ShutdownCount = 0;
				EnterCriticalSection(&gsShutDown);

				if (dwShutdownStatus >= 1 && dwShutdownStatus <= 4)
				{
					curTickCount = GetTickCount();
					if (curTickCount > dwShutdownTickCount)
					{
						d2gsconf.enablegslog = 1;
						D2GSSaveAllGames(5000);
						Sleep(0xBB8);
						switch (dwShutdownStatus - 1)
						{
						case 0:
							d2gsconf.enablegslog = 1;
							D2GSEventLog("D2GSShutdownTimer", "Restart GS now");
							D2GSEventLogCleanup();
							d2gsconf.enablegslog = 0;
							D2GSShutdown(0);
							break;
						case 1:
							d2gsconf.enablegslog = 1;
							D2GSEventLog("D2GSShutdownTimer", "Shutdown GS now");
							D2GSEventLogCleanup();
							d2gsconf.enablegslog = 0;
							D2GSShutdown(1);
							break;
						case 2:
							d2gsconf.enablegslog = 1;
							D2GSEventLog("D2GSShutdownTimer", "D2CS Restart GS now");
							D2GSEventLogCleanup();
							d2gsconf.enablegslog = 0;
							D2GSShutdown(0);
							break;
						case 3:
							d2gsconf.enablegslog = 1;
							D2GSEventLog("D2GSShutdownTimer", "D2CS Shutdown GS now");
							D2GSEventLogCleanup();
							d2gsconf.enablegslog = 0;
							D2GSShutdown(1);
							break;
						default:
							break;
						}
					}
					else
					{
						overSeconds = (dwShutdownTickCount - curTickCount) / 1000;
						if ((overSeconds / 15) == 0)
						{
							switch (dwShutdownStatus - 1)
							{
							case 0:
								sprintf(buffer, "The game server will restart in %lu seconds", overSeconds);
								chat_message_announce_all(CHAT_MESSAGE_TYPE_SYS_MESSAGE, buffer);
								break;
							case 1:
								sprintf(buffer, "The game server will shutdown in %lu seconds", overSeconds);
								chat_message_announce_all(CHAT_MESSAGE_TYPE_SYS_MESSAGE, buffer);
								break;
							case 2:
								sprintf(buffer, "The realm will restart in %lu seconds", overSeconds);
								chat_message_announce_all(CHAT_MESSAGE_TYPE_SYS_MESSAGE, buffer);
								break;
							case 3:
								sprintf(buffer, "The realm will shutdown in %lu seconds", overSeconds);
								chat_message_announce_all(CHAT_MESSAGE_TYPE_SYS_MESSAGE, buffer);
								break;
							default:
								buffer[0] = 0;
								break;
							}
						}
					}
				}
				LeaveCriticalSection(&gsShutDown);
			}
			return;
		}

	}

}