/*
 * vars.h: header file for vars.c, declare global variables here
 */

#ifndef INCLUDED_VARS_H
#define INCLUDED_VARS_H


#include <windows.h>
#include <d2server.h>
#include "d2gs.h"


namespace pvpgn
{

	namespace d2gs
	{

		/* variales */
		extern D2GSCONFIGS			d2gsconf;
		extern AutoUpdateSetting	gAutoUpdate;
		extern BOOL					bGERunning;
		extern HANDLE				hGEThread;
		extern char					gWorldEventMessage[256];
		extern char					PathName[260];
		extern const char* gD2GEVersionString;
		extern CRITICAL_SECTION		csGameList;
		extern CRITICAL_SECTION		gsShutDown;
		extern HANDLE				hD2GSMutex;
		extern HANDLE				hStopEvent;


		extern D2GSStartFunc		 			D2GSStart;
		extern D2GSSendDatabaseCharacterFunc 	D2GSSendDatabaseCharacter;
		extern D2GSRemoveClientFromGameFunc		D2GSRemoveClientFromGame;
		extern D2GSNewEmptyGameFunc				D2GSNewEmptyGame;
		extern D2GSEndAllGamesFunc				D2GSEndAllGames;
		extern D2GSSendClientChatMessageFunc	D2GSSendClientChatMessage;
		extern D2GSSetTickCountFunc			    D2GSSetTickCount;
		extern D2GSSetACDataFunc				D2GSSetACData;
		extern D2GSLoadConfigFunc				D2GSLoadConfig;
		extern D2GSAfterEndFunc				    D2GSAfterEnd;
		extern D2GSInitConfigFunc				D2GSInitConfig;
		extern D2GSCheckTickCountFunc			D2GSCheckTickCount;


		/* functions */
		int  D2GSVarsInitialize(void);
		int  CleanupRoutineForVars(void);

	}

}


#endif /* INCLUDED_VARS_H */