#include <windows.h>
#include <stdio.h>
#include <d2server.h>
#include "callback.h"
#include "debug.h"
#include "vars.h"
#include "eventlog.h"
#include "handle_s2s.h"


#define _D(v) #v,v


namespace pvpgn
{

	namespace d2gs
	{

		EVENTCALLBACKTABLE	gEventCallbackTable;


		extern void __fastcall CloseGame(WORD wGameId, DWORD dwClientTag, DWORD dwTotalEnter, DWORD dwGameLife)
		{
			DebugEventCallback("CloseGame", 4, _D(wGameId), _D(dwClientTag), _D(dwTotalEnter), _D(dwGameLife));
			D2GSCBCloseGame(wGameId);
			return;
		}


		extern void __fastcall LeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass,
			DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
			WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
			BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
			LPCSTR lpAccountName, PLAYERDATA PlayerData,
			PLAYERMARK PlayerMark)
		{
			DebugEventCallback("LeaveGame", 14, _D(lpGameData), _D(wGameId), _D(wCharClass),
				_D(dwCharLevel), _D(dwExpLow), _D(dwExpHigh), _D(wCharStatus),
				_D(lpCharName), _D(lpCharPortrait), _D(bUnlock), _D(dwZero1),
				_D(dwZero2), _D(lpAccountName), _D(PlayerData), _D(PlayerMark));
			D2GSCBLeaveGame(lpGameData, wGameId, wCharClass, dwCharLevel, dwExpLow,
				dwExpHigh, wCharStatus, lpCharName, lpCharPortrait, bUnlock,
				dwZero1, dwZero2, lpAccountName, PlayerData, PlayerMark);
			return;
		}


		extern void __fastcall GetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			DWORD dwClientId, LPCSTR lpAccountName)
		{
			DebugEventCallback("GetDatabaseCharacter", 4, _D(lpGameData), _D(lpCharName),
				_D(dwClientId), _D(lpAccountName));

			D2GSCBGetDatabaseCharacter(lpGameData, lpCharName, dwClientId, lpAccountName);
			return;
		}


		extern void __fastcall SaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			LPCSTR lpAccountName, LPVOID lpSaveData,
			DWORD dwSize, PLAYERDATA PlayerData)
		{
			DebugEventCallback("SaveDatabaseCharacter", 6, _D(lpGameData), _D(lpCharName),
				_D(lpAccountName), _D(lpSaveData), _D(dwSize), _D(PlayerData));
			D2GSCBSaveDatabaseCharacter(lpGameData, lpCharName,
				lpAccountName, lpSaveData, dwSize, PlayerData);
			return;
		}


		extern void __cdecl	ServerLogMessage(DWORD dwCount, LPCSTR lpFormat, ...)
		{
			va_list     ap;

			va_start(ap, lpFormat);
			LogAP("ServerLogMessage", lpFormat, ap);
			va_end(ap);
			return;
		}


		extern void __fastcall EnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass,
			DWORD dwCharLevel, DWORD dwReserved)

		{
			DebugEventCallback("EnterGame", 5, _D(wGameId), _D(lpCharName), _D(wCharClass),
				_D(dwCharLevel), _D(dwReserved));
			D2GSCBEnterGame(wGameId, lpCharName, wCharClass, dwCharLevel, dwReserved);
			return;
		}


		extern BOOL __fastcall FindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
			LPSTR lpAccountName, LPPLAYERDATA lpPlayerData, void* unused1, void* unused2)
		{
			DebugEventCallback("FindPlayerToken", 7, _D(lpCharName), _D(dwToken),
				_D(wGameId), _D(lpAccountName), _D(lpPlayerData), _D(unused1), _D(unused2));
			return D2GSCBFindPlayerToken(lpCharName, dwToken, wGameId, lpAccountName, lpPlayerData);
		}


		extern void __fastcall UnlockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			LPCSTR lpAccountName)
		{
			DebugEventCallback("UnlockDatabaseCharacter", 3, _D(lpGameData), _D(lpCharName),
				_D(lpAccountName));
			return;
		}


		extern void __fastcall RelockDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			LPCSTR lpAccountName)
		{
			DebugEventCallback("RelockDatabaseCharacter", 3, _D(lpGameData), _D(lpCharName),
				_D(lpAccountName));
			return;
		}


		extern void __fastcall UpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass,
			DWORD dwCharLevel, DWORD dwCharExpLow,
			DWORD dwCharExpHigh, WORD wCharStatus,
			PLAYERMARK PlayerMark)
		{
			DebugEventCallback("UpdateCharacterLadder", 7, _D(lpCharName), _D(wCharClass),
				_D(dwCharLevel), _D(dwCharExpLow), _D(dwCharExpHigh),
				_D(wCharStatus), _D(PlayerMark));
			D2GSUpdateCharacterLadder(lpCharName, wCharClass, dwCharLevel, dwCharExpLow,
				dwCharExpHigh, wCharStatus);
			return;
		}


		extern void __fastcall UpdateGameInformation(WORD wGameId, LPCSTR lpCharName,
			WORD wCharClass, DWORD dwCharLevel)
		{
			DebugEventCallback("UpdateGameInformation", 4, _D(lpCharName),
				_D(wCharClass), _D(dwCharLevel));
			D2GSCBUpdateGameInformation(wGameId, lpCharName, wCharClass, dwCharLevel);
			return;
		}


		extern GAMEDATA __fastcall SetGameData(void)
		{
			DebugEventCallback("SetGameData", 0);
			return (GAMEDATA)0x87654321;
		}


		extern void __fastcall SaveDatabaseGuild(DWORD dwReserved1, DWORD dwReserved2,
			DWORD dwReserved3)

		{
			DebugEventCallback("SaveDatabaseGuild", 3, _D(dwReserved1), _D(dwReserved2),
				_D(dwReserved3));
			return;
		}


		extern void __fastcall ReservedCallback1(DWORD dwReserved1, DWORD dwReserved2)
		{
			DebugEventCallback("ReservedCallback1", 2, _D(dwReserved1), _D(dwReserved2));
			return;
		}


		extern void __fastcall ReservedCallback2(DWORD dwReserved1, DWORD dwReserved2,
			DWORD dwReserved3)
		{
			DebugEventCallback("ReservedCallback2", 3, _D(dwReserved1), _D(dwReserved2),
				_D(dwReserved3));
			return;
		}


		extern void __fastcall LoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion)
		{
			DebugEventCallback("LoadComplete", 3, _D(wGameId), _D(lpCharName),
				_D(bExpansion));
			//D2GSLoadComplete(wGameId, lpCharName, bExpansion);
			return;
		}


		extern PEVENTCALLBACKTABLE EventCallbackTableInit(void)
		{
			gEventCallbackTable.fpCloseGame = CloseGame;
			gEventCallbackTable.fpLeaveGame = LeaveGame;
			gEventCallbackTable.fpGetDatabaseCharacter = GetDatabaseCharacter;
			gEventCallbackTable.fpSaveDatabaseCharacter = SaveDatabaseCharacter;
			gEventCallbackTable.fpServerLogMessage = ServerLogMessage;
			gEventCallbackTable.fpEnterGame = EnterGame;
			gEventCallbackTable.fpFindPlayerToken = FindPlayerToken;
			gEventCallbackTable.fpUnlockDatabaseCharacter = UnlockDatabaseCharacter;
			gEventCallbackTable.fpRelockDatabaseCharacter = RelockDatabaseCharacter;
			gEventCallbackTable.fpUpdateCharacterLadder = UpdateCharacterLadder;
			gEventCallbackTable.fpUpdateGameInformation = UpdateGameInformation;
			gEventCallbackTable.fpSetGameData = SetGameData;
			gEventCallbackTable.fpReserved1 = ReservedCallback1;
			gEventCallbackTable.fpReserved2 = ReservedCallback2;
			gEventCallbackTable.fpSaveDatabaseGuild = SaveDatabaseGuild;
			gEventCallbackTable.fpLoadComplete = LoadComplete;
			return &gEventCallbackTable;
		}

	}

}