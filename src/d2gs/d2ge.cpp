#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <d2server.h>
#include "d2gs.h"
#include "d2ge.h"
#include "eventlog.h"
#include "callback.h"
#include "vars.h"
#include "server.h"


namespace pvpgn
{

	namespace d2gs
	{

		/* functions in d2server.dll, got by QueryInterface() */
		D2GSStartFunc		 			D2GSStart;
		D2GSSendDatabaseCharacterFunc 	D2GSSendDatabaseCharacter;
		D2GSRemoveClientFromGameFunc	D2GSRemoveClientFromGame;
		D2GSNewEmptyGameFunc			D2GSNewEmptyGame;
		D2GSEndAllGamesFunc				D2GSEndAllGames;
		D2GSSendClientChatMessageFunc	D2GSSendClientChatMessage;
		D2GSSetTickCountFunc			D2GSSetTickCount;
		D2GSSetACDataFunc				D2GSSetACData;
		D2GSLoadConfigFunc				D2GSLoadConfig;
		D2GSAfterEndFunc				D2GSAfterEnd;
		D2GSInitConfigFunc				D2GSInitConfig;
		D2GSCheckTickCountFunc			D2GSCheckTickCount;


		/* variables */
		D2GSINFO						gD2GSInfo;
		static HANDLE					ghServerThread;
		static HMODULE					gD2ServerDll;
		static char						gD2GS_VERSION_STRING[256] = { 0 };
		DWORD							D2GEVersion;


		/*********************************************************************
		 * Purpose: to startup the D2 Game Engine
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GEStartup(void)
		{
			HANDLE		hEvent;
			DWORD		dwThreadId;
			DWORD		dwWait;

			/* init GE thread */
			if (!D2GEThreadInit())
			{
				D2GSEventLog("D2GEStartup", "Failed to Initialize Server");
				return FALSE;
			}

			/* create event for notification of GE startup */
			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!hEvent) return FALSE;

			/* startup the server thread */
			ghServerThread = CreateThread(NULL, 0, D2GEThread, (LPVOID)hEvent, 0, &dwThreadId);
			if (!ghServerThread)
			{
				D2GSEventLog("D2GEStartup", "Can't CreateThread D2GEThread. Code: %lu", GetLastError());
				CloseHandle(hEvent);
				return FALSE;
			}
			dwWait = WaitForSingleObject(hEvent, D2GE_INIT_TIMEOUT);
			CloseHandle(hEvent);

			if (dwWait != WAIT_OBJECT_0)
			{
				CloseHandle(ghServerThread);
				return FALSE;
			}

			if (bGERunning == 0)
			{
				CloseHandle(ghServerThread);
				return FALSE;
			}

			if (CleanupRoutineInsert(D2GECleanup, "Diablo II Game Engine"))
			{
				return TRUE;
			}
			else
			{
				/* do some cleanup before quiting */
				D2GECleanup();
				return FALSE;
			}
		} /* End of D2GEStartup() */


		/*********************************************************************
		 * Purpose: to shutdown the D2 Game Engine
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GECleanup(void)
		{
			D2GSEndAllGames();

			Sleep(1000);
			D2GSAfterEnd();

			gD2GSInfo.bStop = TRUE;
			if (ghServerThread)
			{
				WaitForSingleObject(ghServerThread, D2GE_SHUT_TIMEOUT);
				CloseHandle(ghServerThread);
				ghServerThread = NULL;

				D2GSStart = NULL;
				D2GSSendDatabaseCharacter = NULL;
				D2GSRemoveClientFromGame = NULL;
				D2GSNewEmptyGame = NULL;
				D2GSEndAllGames = NULL;
				D2GSSendClientChatMessage = NULL;
				D2GSSetTickCount = NULL;
				D2GSSetACData = NULL;
				// D2GSUnknown1
				D2GSLoadConfig = NULL;
				D2GSAfterEnd = NULL;
				D2GSInitConfig = NULL;
				// unknow 2 here
				//
			}

			return TRUE;

		} /* End of D2GEShutdown() */

		/*********************************************************************
		 * Purpose: to initialize the D2 Game Engine thread
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GEThreadInit(void)
		{
			if (!D2GSGetInterface())
			{
				D2GSEventLog("D2GSThread", "Failed to Get Server Interface");
				return FALSE;
			}

			ZeroMemory(&gD2GSInfo, sizeof(gD2GSInfo));
			strcpy(gD2GS_VERSION_STRING, D2GS_VERSION_STRING);
			gD2GSInfo.szVersion = D2GS_VERSION_STRING;
			gD2GSInfo.dwLibVersion = D2GS_LIBRARY_VERSION;
			gD2GSInfo.bIsNT = d2gsconf.enablentmode;
			gD2GSInfo.bEnablePatch = d2gsconf.enablegepatch;
			gD2GSInfo.fpEventLog = D2GEEventLog;
			gD2GSInfo.fpErrorHandle = D2GSErrorHandle;
			gD2GSInfo.fpCallback = EventCallbackTableInit();
			gD2GSInfo.bPreCache = d2gsconf.enableprecachemode;
			gD2GSInfo.dwIdleSleep = d2gsconf.idlesleep;
			gD2GSInfo.dwBusySleep = d2gsconf.busysleep;
			gD2GSInfo.dwMaxGame = d2gsconf.gemaxgames;
			gD2GSInfo.dwMaxPacketPerSecond = d2gsconf.maxpacketpersecond; //d2gs use maxpacketpersecond, ithink it is bug

			return TRUE;

		} /* D2GEThreadInit() */


		/*********************************************************************
		 * Purpose: to set the GE interface
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		static BOOL D2GSGetInterface(void)
		{
			LPD2GSINTERFACE		lpD2GSInterface;

			lpD2GSInterface = QueryInterface();
			if (!lpD2GSInterface) return FALSE;

			D2GSStart = lpD2GSInterface->D2GSStart;
			D2GSSendDatabaseCharacter = lpD2GSInterface->D2GSSendDatabaseCharacter;
			D2GSRemoveClientFromGame = lpD2GSInterface->D2GSRemoveClientFromGame;
			D2GSNewEmptyGame = lpD2GSInterface->D2GSNewEmptyGame;
			D2GSEndAllGames = lpD2GSInterface->D2GSEndAllGames;
			D2GSSendClientChatMessage = lpD2GSInterface->D2GSSendClientChatMessage;
			D2GSSetTickCount = lpD2GSInterface->D2GSSetTickCount;
			D2GSSetACData = lpD2GSInterface->D2GSSetACData;
			D2GSLoadConfig = lpD2GSInterface->D2GSLoadConfig;
			D2GSAfterEnd = lpD2GSInterface->D2GSAfterEnd;
			D2GSInitConfig = lpD2GSInterface->D2GSInitConfig;
			D2GSCheckTickCount = lpD2GSInterface->D2GSCheckTickCount;

			return TRUE;

		} /* End of D2GSGetInterface() */


		/*********************************************************************
		 * Purpose: called by Game Engine when error occur
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		static int WINAPI D2GSErrorHandle(void)
		{
			D2GSEventLog("D2GSErrorHandle", "Error occur, exiting...\n\n");

#ifdef DEBUG_ON_CONSOLE
			printf("Press Any Key to Continue");
			_getch();
#endif

			d2gsconf.enablegslog = 1;
			D2GSEventLog("D2GSErrorHandle", "Error occur, exiting...\n\n");
			d2gsconf.enablegslog = 0;

			D2GSShutdown(0);
			return 0;

		} /* End of D2GSErrorHandle() */


		/*********************************************************************
		 * Purpose: D2 GE main thread
		 * Return: return value of the thread
		 *********************************************************************/
		DWORD WINAPI D2GEThread(LPVOID lpParameter)
		{
			DWORD			dwThreadId;
			DWORD			dwRetval;
			HANDLE			hObjects[2];
			DWORD			dwExitCode;
			HANDLE			hEvent;

			hEvent = (HANDLE)lpParameter;
			if (!hEvent) return FALSE;

			bGERunning = FALSE;
			gD2GSInfo.bStop = FALSE;
			hObjects[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			gD2GSInfo.hEventInited = hObjects[0];
			if (!hObjects[0])
			{
				D2GSEventLog("D2GEThread", "Error in CreateEvent. Code: %lu", GetLastError());
				SetEvent(hEvent);
				return FALSE;
			}

			hObjects[1] = CreateThread(NULL, 0, D2GSStart, &gD2GSInfo, 0, &dwThreadId);
			if (!hObjects[1])
			{
				D2GSEventLog("D2GEThread", "Error Creating Server Thread. Code: %lu", GetLastError());
				CloseHandle(hObjects[0]);
				SetEvent(hEvent);
				return FALSE;
			}
			else
			{
				D2GSEventLog("D2GEThread", "Server Thread %lu Created", dwThreadId);
			}
			dwRetval = WaitForMultipleObjects(NELEMS(hObjects), hObjects, FALSE, D2GE_INIT_TIMEOUT);

			if (dwRetval == WAIT_FAILED)
			{
				D2GSEventLog("D2GEThread", "Wait Server Thread Failed. Code: %lu", GetLastError());
				SetEvent(hEvent);
			}
			else if (dwRetval == WAIT_TIMEOUT)
			{
				D2GSEventLog("D2GEThread", "Game Server Thread Timedout");
				SetEvent(hEvent);
			}
			else if (dwRetval == WAIT_OBJECT_0 + 1)
			{
				GetExitCodeThread(hObjects[1], &dwExitCode);
				D2GSEventLog("D2GEThread", "Game Server Thread Exit with %d", dwExitCode);
				SetEvent(hEvent);
			}
			else if (dwRetval == WAIT_OBJECT_0)
			{
				D2GSEventLog("D2GEThread", "Game Server Thread Start Successfully");
				SetEvent(hEvent);
				bGERunning = TRUE;
				hGEThread = hObjects[1];
				SetThreadAffinityMask(hObjects[1], d2gsconf.multicpumask);
			}
			else
			{
				D2GSEventLog("D2GEThread", "Wait Server Thread Returned %d", dwRetval);
				SetEvent(hEvent);
			}
			WaitForSingleObject(hObjects[1], INFINITE);
			CloseHandle(hObjects[0]);
			CloseHandle(hObjects[1]);
			bGERunning = FALSE;
			hGEThread = NULL;
			D2GSEventLog("D2GEThread", "Game Engine Thread terminated");
			return TRUE;

		} /* End of D2GEThread */


		/*********************************************************************
		 * Purpose: Init from conf faile
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		void D2GEReloadConfig(void)
		{
			char buffer[260] = { 0 };
			const WORLDEVENT* ret = 0;
			snprintf(buffer, 259, "%s\\%s", PathName, d2gsconf.serverconffile);
			buffer[259] = 0;
			D2GSLoadConfig(buffer);
			ret = D2GSInitConfig();
			if (ret->bHasMsg)
			{
				snprintf(gWorldEventMessage, 255, "\xFF" "c9The World Event enables on this server. The key item is \"%s\"." "\xFF" "c1", (UCHAR*)(ret + 1));
			}
		} /* End of D2GEInitConfig() */

	}

}