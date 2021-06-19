#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <d2server.h>
#include "d2gs.h"
#include "vars.h"
#include "eventlog.h"
#include "charlist.h"
#include "d2gamelist.h"
#include "telnetd.h"
#include "handle_s2s.h"
#include "utils.h"


namespace pvpgn
{

	namespace d2gs
	{

/* vars */
static D2GAMEINFO		*lpGameInfoHead   = NULL;
static D2CHARINFO		*lpPendingChar    = NULL;
static D2GETDATAREQUEST	*lpGetDataReqHead = NULL;
static DWORD			currentgamenum    = 0;

static char desc_game_difficulty[][32] = {
	"normal", "nightmare", "hell"
};
static char desc_char_class[][16] = {
	"Ama", "Sor", "Nec", "Pal", "Bar", "Dur", "Ass"
};

static LIST_HEAD(list_motd);

/*********************************************************************
 * Purpose: to reset the GameList
 * Return: None
 *********************************************************************/
void D2GSResetGameList(void)
{
	D2GAMEINFO		*ph, *pnext;
	D2CHARINFO		*pChar, *pCharNext;

	/* to release all the memory */
	EnterCriticalSection(&csGameList);
	ph = lpGameInfoHead;
	while(ph)
	{
		pnext = ph->next;
		D2GSDeleteAllCharInGame(ph);
		free(ph);
		ph = pnext;
	}
	pChar = lpPendingChar;
	while(pChar)
	{
		pCharNext = pChar->next;
		free(pChar);
		pChar = pCharNext;
	}
	D2GSEndAllGames();
	charlist_flush();
	lpGameInfoHead = NULL;
	lpPendingChar  = NULL;
	currentgamenum = 0;
	LeaveCriticalSection(&csGameList);
	D2GSEventLog("D2GSResetGameList", "End all game in the Game List and in the GE");
	return;

} /* End of D2GSResetGameList() */


/*********************************************************************
 * Purpose: to get current game number
 * Return: int
 *********************************************************************/
int D2GSGetCurrentGameNumber(void)
{
	return currentgamenum;

} /* End of D2GSGetCurrentGameNumber() */


/*********************************************************************
 * Purpose: to get current game number and user numbers in games
 * Return: int
 *********************************************************************/
int D2GSGetCurrentGameStatistic(DWORD *gamenum, DWORD *usernum)
{
	D2GAMEINFO		*lpGame;
	int				gamecount, charcount;

	gamecount = charcount = 0;
	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		gamecount++;
		charcount += lpGame->CharCount;
		lpGame = lpGame->next;
		if (gamecount>500) break;
	}
	LeaveCriticalSection(&csGameList);

	*gamenum = gamecount;
	*usernum = charcount;
	return 0;

} /* End of D2GSGetCurrentGameStatistic() */


/*********************************************************************
 * Purpose: to kick all char out of a game
 * Return: None
 *********************************************************************/
void D2GSDeleteAllCharInGame(D2GAMEINFO *lpGameInfo)
{
	D2CHARINFO		*ph, *pnext;
	WORD			count;

	if (!lpGameInfo) return;
	ph = lpGameInfo->lpCharInfo;
	count = 0;
	while(ph)
	{
		pnext = ph->next;
		if (ph->CharLockStatus) {
			D2GSEventLog("D2GSDeleteAllCharInGame",
					"Unlock char %s(*%s) with unfinished loading status",
					ph->CharName, ph->AcctName);
			D2GSUnlockChar(ph->AcctName, ph->CharName);
		}
		D2GSEventLog("D2GSDeleteAllCharInGame",
				"Delete zombie char %s(*%s)", ph->CharName, ph->AcctName);
		charlist_delete(ph->CharName);
		free(ph);
		count++;
		ph = pnext;
	}
	lpGameInfo->lpCharInfo = NULL;
	D2GSEventLog("D2GSDeleteAllCharInGame",
		"Delete %u(%u) character in game '%s' (%u)",
		count, lpGameInfo->CharCount, lpGameInfo->GameName, lpGameInfo->GameId);
	lpGameInfo->CharCount = 0;
	return;

} /* End of D2GSDeleteAllCharInGame() */


/*********************************************************************
 * Purpose: to insert a new game info the game info list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSGameListInsert(
	const char* game_name,
	const char* game_pass,
	const char* game_desc,
	const char* creator_acct_name,
	const char* creator_char_name,
	const char* creator_ip,
	UCHAR expansion,
	UCHAR difficulty,
	UCHAR hardcore,
	UCHAR ladder,
	WORD wGameId)
{
	D2GAMEINFO		*lpGameInfo;
	D2GAMEINFO		*lpTemp;

	if (!game_name || !game_pass || !game_desc) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory for the structure */
	lpGameInfo = (D2GAMEINFO *)malloc(sizeof(D2GAMEINFO));
	if (!lpGameInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpGameInfo, sizeof(D2GAMEINFO));
	strncpy(lpGameInfo->GameName, game_name, MAX_GAMENAME_LEN-1);
	snprintf(lpGameInfo->game_pass, sizeof(lpGameInfo->game_pass), "%s", game_pass);
	snprintf(lpGameInfo->game_desc, sizeof(lpGameInfo->game_desc), "%s", game_desc);
	snprintf(lpGameInfo->creator_acct_name, sizeof(lpGameInfo->creator_acct_name), "%s", creator_acct_name);
	snprintf(lpGameInfo->creator_char_name, sizeof(lpGameInfo->creator_char_name), "%s", creator_char_name);
	snprintf(lpGameInfo->creator_ip, sizeof(lpGameInfo->creator_ip), "%s", creator_ip);
	lpGameInfo->ladder     = ladder;
	lpGameInfo->expansion  = expansion;
	lpGameInfo->difficulty = difficulty;
	lpGameInfo->hardcore   = hardcore;
	lpGameInfo->GameId     = wGameId;
	lpGameInfo->CharCount  = 0;
	lpGameInfo->CreateTime = time(NULL);
	lpGameInfo->disable    = FALSE;

	EnterCriticalSection(&csGameList);

	/* add to game list */
	lpTemp = lpGameInfoHead;
	lpGameInfoHead = lpGameInfo;
	if (lpTemp) {
		lpGameInfo->next = lpTemp;
		lpTemp->prev = lpGameInfo;
	}
	D2GSIncCurrentGameNumber();

	LeaveCriticalSection(&csGameList);

	D2GSEventLog("D2GSGameListInsert",
		"Insert into game list '%s' (%u)", game_name, wGameId);
	return 0;

} /* End of D2GSGameListInsert() */


/*********************************************************************
 * Purpose: to delete a game from the game info list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSGameListDelete(D2GAMEINFO *lpGameInfo)
{
	D2GAMEINFO		*lpPrev, *lpNext;

	if (!lpGameInfo) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	D2GSDeleteAllCharInGame(lpGameInfo);
	lpPrev = lpGameInfo->prev;
	lpNext = lpGameInfo->next;
	if (lpPrev) lpPrev->next = lpNext;
	else lpGameInfoHead = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	lpGameInfo->GameId = 0;
	free(lpGameInfo);
	D2GSDecCurrentGameNumber();
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSGameListDelete() */


/*********************************************************************
 * Purpose: to insert a char into the game
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertCharIntoGameInfo(D2GAMEINFO *lpGameInfo, DWORD token, const char* AcctName,
	const char* CharName, const char* RealmIPName, DWORD CharLevel, WORD CharClass, WORD EnterGame)
{
	D2CHARINFO		*lpCharInfo;
	D2CHARINFO		*lpTemp;
	int				val;

	if (!lpGameInfo) return D2GSERROR_BAD_PARAMETER;
	if (!AcctName || !CharName) return D2GSERROR_BAD_PARAMETER;
	if (lpGameInfo->CharCount >= MAX_CHAR_IN_GAME) return D2GSERROR_GAME_IS_FULL;

	/* alloc memory */
	lpCharInfo = (D2CHARINFO *)malloc(sizeof(D2CHARINFO));
	if (!lpCharInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpCharInfo, sizeof(D2CHARINFO));
	strncpy(lpCharInfo->AcctName, AcctName, MAX_ACCTNAME_LEN-1);
	strncpy(lpCharInfo->CharName, CharName, MAX_CHARNAME_LEN-1);
	strncpy(lpCharInfo->RealmIPName, RealmIPName, MAX_REALMIPNAME_LEN - 1);
	lpCharInfo->token          = token;
	lpCharInfo->CharLevel      = CharLevel;
	lpCharInfo->CharClass      = CharClass;
	lpCharInfo->EnterGame      = EnterGame;
	lpCharInfo->AllowLadder    = FALSE;
	lpCharInfo->CharLockStatus = FALSE;
	lpCharInfo->EnterTime      = time(NULL);
	lpCharInfo->CharCreateTime = lpCharInfo->EnterTime;
	lpCharInfo->GameId         = lpGameInfo->GameId;
	lpCharInfo->lpGameInfo     = lpGameInfo;
	lpCharInfo->ClientId       = 0;

	/* insert char into char list table */
	if ((val=charlist_insert(CharName, lpCharInfo, lpGameInfo))!=0) {
		D2GSEventLog("D2GSInsertCharIntoGameInfo",
				"failed insert info charlist for %s(*%s), code: %d", CharName, AcctName, val);
		free(lpCharInfo);
		return D2GSERROR_CHAR_ALREADY_IN_GAME;
	}

	EnterCriticalSection(&csGameList);

	/* add to game info */
	lpTemp = lpGameInfo->lpCharInfo;
	lpGameInfo->lpCharInfo = lpCharInfo;
	if (lpTemp) {
		lpCharInfo->next = lpTemp;
		lpTemp->prev = lpCharInfo;
	}
	(lpGameInfo->CharCount)++;

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertCharIntoGameInfo() */


/*********************************************************************
 * Purpose: to delete a char from the game
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeleteCharFromGameInfo(D2GAMEINFO *lpGameInfo, D2CHARINFO *lpCharInfo)
{
	D2CHARINFO		*lpPrev, *lpNext;

	if (!lpGameInfo || !lpCharInfo) return D2GSERROR_BAD_PARAMETER;
	//if (lpCharInfo->lpGameInfo != lpGameInfo) return D2GSERROR_BAD_PARAMETER;

	EnterCriticalSection(&csGameList);
	lpPrev = lpCharInfo->prev;
	lpNext = lpCharInfo->next;
	if (lpPrev) lpPrev->next = lpNext;
	else lpGameInfo->lpCharInfo = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	(lpGameInfo->CharCount)--;
	charlist_delete(lpCharInfo->CharName);
	free(lpCharInfo);
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSDeleteCharFromGameInfo() */


/*********************************************************************
 * Purpose: to insert a char into the pending char list
 *          (receive join game request, but not EnterGame event callback)
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertCharIntoPendingList(DWORD token, const char* AcctName,
	const char* CharName, const char* RealmIPName, DWORD CharLevel, WORD CharClass, D2GAMEINFO *lpGame)
{
	D2CHARINFO		*lpCharInfo;
	D2CHARINFO		*lpTemp;

	if (!AcctName || !CharName || !lpGame) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory */
	lpCharInfo = (D2CHARINFO *)malloc(sizeof(D2CHARINFO));
	if (!lpCharInfo) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpCharInfo, sizeof(D2CHARINFO));
	strncpy(lpCharInfo->AcctName, AcctName, MAX_ACCTNAME_LEN-1);
	strncpy(lpCharInfo->CharName, CharName, MAX_CHARNAME_LEN-1);
	strncpy(lpCharInfo->RealmIPName, RealmIPName, MAX_REALMIPNAME_LEN - 1);
	lpCharInfo->token      = token;
	lpCharInfo->CharLevel  = CharLevel;
	lpCharInfo->CharClass  = CharClass;
	lpCharInfo->EnterGame  = FALSE;
	lpCharInfo->EnterTime  = 0;
	lpCharInfo->GameId     = lpGame->GameId;
	lpCharInfo->lpGameInfo = lpGame;
	lpCharInfo->ClientId   = 0;

	EnterCriticalSection(&csGameList);

	/* add to pending char list */
	lpTemp = lpPendingChar;
	lpPendingChar = lpCharInfo;
	if (lpTemp) {
		lpCharInfo->next = lpTemp;
		lpTemp->prev = lpCharInfo;
	}
	(lpGame->CharCount)++;

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertCharIntoPendingList() */


/*********************************************************************
 * Purpose: to delete a char from the pending char list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeletePendingChar(D2CHARINFO *lpCharInfo)
{
	D2CHARINFO		*lpPrev, *lpNext;

	if (!lpCharInfo) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	lpPrev = lpCharInfo->prev;
	lpNext = lpCharInfo->next;
	if (lpPrev)	lpPrev->next = lpNext;
	else lpPendingChar = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	if ((!IsBadReadPtr(lpCharInfo->lpGameInfo, sizeof(D2GAMEINFO)))
					&& (lpCharInfo->lpGameInfo->GameId == lpCharInfo->GameId)) {
		(lpCharInfo->lpGameInfo->CharCount)--;
	} else {
		D2GSEventLog("D2GSDeletePendingChar",
			"Delete a pending char %s(*%s) in an already closed game",
			lpCharInfo->CharName, lpCharInfo->AcctName);
	}
	free(lpCharInfo);
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSGameListDelete() */


/*********************************************************************
 * Purpose: to insert get data request to the list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSInsertGetDataRequest(const char* AcctName, const char* CharName, DWORD dwClientId, DWORD dwSeqno)
{
	D2GETDATAREQUEST	*lpGetDataReq;
	D2GETDATAREQUEST	*lpTemp;

	if (!AcctName || !CharName) return D2GSERROR_BAD_PARAMETER;

	/* alloc memory */
	lpGetDataReq = (D2GETDATAREQUEST*)malloc(sizeof(D2GETDATAREQUEST));
	if (!lpGetDataReq) return D2GSERROR_NOT_ENOUGH_MEMORY;

	/* fill the fields */
	ZeroMemory(lpGetDataReq, sizeof(D2GETDATAREQUEST));
	strncpy(lpGetDataReq->AcctName, AcctName, MAX_ACCTNAME_LEN-1);
	strncpy(lpGetDataReq->CharName, CharName, MAX_CHARNAME_LEN-1);
	lpGetDataReq->ClientId  = dwClientId;
	lpGetDataReq->Seqno     = dwSeqno;
	lpGetDataReq->TickCount = 0;

	EnterCriticalSection(&csGameList);

	/* add to request list */
	lpTemp = lpGetDataReqHead;
	lpGetDataReqHead = lpGetDataReq;
	if (lpTemp) {
		lpGetDataReq->next = lpTemp;
		lpTemp->prev = lpGetDataReq;
	}

	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSInsertGetDataRequest() */


/*********************************************************************
 * Purpose: to delete a get data request from the list
 * Return: 0(success), other(failed)
 *********************************************************************/
int D2GSDeleteGetDataRequest(D2GETDATAREQUEST *lpGetDataReq)
{
	D2GETDATAREQUEST	*lpPrev, *lpNext;

	if (!lpGetDataReq) return 0;	/* nothing to be deleted */

	EnterCriticalSection(&csGameList);
	lpPrev = lpGetDataReq->prev;
	lpNext = lpGetDataReq->next;
	if (lpPrev)	lpPrev->next = lpNext;
	else lpGetDataReqHead = lpNext;
	if (lpNext) lpNext->prev = lpPrev;
	free(lpGetDataReq);
	LeaveCriticalSection(&csGameList);

	return 0;

} /* End of D2GSDeleteGetDataRequest() */


/*********************************************************************
 * Purpose: to find a game info by game id
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GAMEINFO *D2GSFindGameInfoByGameId(WORD GameId)
{
	D2GAMEINFO	*lpGame;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		if (lpGame->GameId == GameId) break;
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGame;

} /* End of D2GSFindGameInfoByGameId() */


/*********************************************************************
 * Purpose: to find a game info by game name
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GAMEINFO *D2GSFindGameInfoByGameName(const char* GameName)
{
	D2GAMEINFO	*lpGame;

	if (!GameName) return NULL;

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		if (!strcmp(lpGame->GameName, GameName)) break;
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGame;

} /* End of D2GSFindGameInfoByGameName() */


/*********************************************************************
 * Purpose: to find a char by char name in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindCharInGameByCharName(D2GAMEINFO *lpGame, const char* CharName)
{
	D2CHARINFO		*lpChar;

	if (!CharName || !lpGame) return NULL;

	EnterCriticalSection(&csGameList);
	lpChar = lpGame->lpCharInfo;
	while(lpChar)
	{
		if (!strcmp(lpChar->CharName, CharName)) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindCharInGameByCharName() */


/*********************************************************************
 * Purpose: to find a char by its token in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindPendingCharByToken(DWORD token)
{
	D2CHARINFO		*lpChar;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		if (lpChar->token == token) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindPendingCharByToken() */


/*********************************************************************
 * Purpose: to find a char by char name in pending char list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2CHARINFO *D2GSFindPendingCharByCharName(const char *CharName)
{
	D2CHARINFO		*lpChar;

	if (!CharName) return NULL;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		if (!strcmp(lpChar->CharName, CharName)) break;
		lpChar = lpChar->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpChar;

} /* End of D2GSFindPendingCharByCharName() */


/*********************************************************************
 * Purpose: to find a get data request from the list
 * Return: NULL(not found or failed), other(success)
 *********************************************************************/
D2GETDATAREQUEST *D2GSFindGetDataRequestBySeqno(DWORD dwSeqno)
{
	D2GETDATAREQUEST	*lpGetDataReq;

	EnterCriticalSection(&csGameList);
	lpGetDataReq = lpGetDataReqHead;
	while(lpGetDataReq)
	{
		if (lpGetDataReq->Seqno==dwSeqno) break;
		lpGetDataReq = lpGetDataReq->next;
	}
	LeaveCriticalSection(&csGameList);
	return lpGetDataReq;

} /* End of D2GSFindGetDataRequestBySeqno() */


/*********************************************************************
 * Purpose: to do sth in the timer for pending char list
 * Return: None
 *********************************************************************/
void D2GSPendingCharTimerRoutine(void)
{
	D2CHARINFO		*lpChar, *lpTimeoutChar;

	EnterCriticalSection(&csGameList);
	lpChar = lpPendingChar;
	while(lpChar)
	{
		lpChar->TickCount++;
		if ((lpChar->TickCount) > (d2gsconf.charpendingtimeout))
			lpTimeoutChar = lpChar;
		else
			lpTimeoutChar = NULL;
		lpChar = lpChar->next;
		if (lpTimeoutChar)
			D2GSDeletePendingChar(lpTimeoutChar);
	}
	LeaveCriticalSection(&csGameList);

	return;

} /* End of D2GSPendingCharTimerRoutine() */


/*********************************************************************
 * Purpose: to do sth in the timer for get data request list
 * Return: None
 *********************************************************************/
void D2GSGetDataRequestTimerRoutine(void)
{
	MOTDCLIENT* lpBufferTemp = NULL;
	MOTDCLIENT* lpBuffer = NULL;
	D2GETDATAREQUEST	*lpGetDataReq, *lpTimeOutReq;
	DWORD				dwClientId;
	char				AcctName[MAX_ACCTNAME_LEN];
	char				CharName[MAX_CHARNAME_LEN];
	D2GAMEINFO			*lpGameInfo;
	D2CHARINFO			*lpCharInfo;

	EnterCriticalSection(&csGameList);
	lpGetDataReq = lpGetDataReqHead;
	while(lpGetDataReq)
	{
		lpGetDataReq->TickCount++;
		if ((lpGetDataReq->TickCount) > GET_DATA_TIMEOUT)
			lpTimeOutReq = lpGetDataReq;
		else
			lpTimeOutReq = NULL;
		lpGetDataReq = lpGetDataReq->next;
		if (lpTimeOutReq) {
			dwClientId = lpTimeOutReq->ClientId;
			lpBufferTemp = static_cast<MOTDCLIENT*>(malloc(sizeof(MOTDCLIENT)));
			if (!lpBufferTemp)
			{
				D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
				D2GSEventLog("D2GSGetDataRequestTimerRoutine", "out of memory");
			}
			else
			{
				lpBufferTemp->ClientId = dwClientId;
				lpBufferTemp->list.next = (struct list_head*)lpBuffer;
				lpBuffer = lpBufferTemp;
			}

			D2GSEventLog("D2GSGetDataRequestTimerRoutine",
				"Failed get CHARSAVE data for '%s'(*%s)",
				lpTimeOutReq->CharName, lpTimeOutReq->AcctName);
			/* check if this char is still in the list? if so, delete it */
			strncpy(AcctName, lpTimeOutReq->AcctName, MAX_ACCTNAME_LEN-1);
			AcctName[MAX_ACCTNAME_LEN-1] = '\0';
			strncpy(CharName, lpTimeOutReq->CharName, MAX_CHARNAME_LEN-1);
			CharName[MAX_CHARNAME_LEN-1] = '\0';
			lpGameInfo = (D2GAMEINFO*)charlist_getdata(CharName, CHARLIST_GET_GAMEINFO);
			lpCharInfo = (D2CHARINFO*)charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
			if (lpCharInfo && lpGameInfo &&
						!IsBadReadPtr(lpCharInfo, sizeof(D2CHARINFO)) &&
						!IsBadReadPtr(lpGameInfo, sizeof(D2GAMEINFO)) &&
						(lpCharInfo->lpGameInfo == lpGameInfo) &&
						(lpCharInfo->GameId == lpGameInfo->GameId) &&
						(lpCharInfo->ClientId == dwClientId)) {
				D2GSDeleteCharFromGameInfo(lpGameInfo, lpCharInfo);
				D2GSEventLog("D2GSGetDataRequestTimerRoutine",
					"delete char %s(*%s) still in game '%s'(%u)",
					CharName, AcctName, lpGameInfo->GameName, lpGameInfo->GameId);
			} else {
				D2GSEventLog("D2GSGetDataRequestTimerRoutine",
					"char %s(*%s) NOT in game now",	CharName, AcctName);
			}
			/* delete the overdue quest */
			D2GSDeleteGetDataRequest(lpTimeOutReq);
		}
	}
	LeaveCriticalSection(&csGameList);

	while (lpBuffer)
	{
		lpBufferTemp = lpBuffer;
		lpBuffer = (MOTDCLIENT*)lpBuffer->list.next;
		D2GSSendDatabaseCharacter(lpBufferTemp->ClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
		free(lpBufferTemp);
	}

	return;

} /* End of D2GSGetDataRequestTimerRoutine() */


/*-------------------------------------------------------------------*/

void FormatTimeString(time_t t, char* buf, int len, int format)
{
	struct tm		*tm;
	time_t			now;

	ZeroMemory(buf, len);
	now = t;
	tm = localtime(&now);
	if (format == 0)
	{
		_snprintf(buf, len - 1, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else if (format == 1)
	{
		_snprintf(buf, len - 1, "%04d-%02d-%02d %02d:%02d:%02d",
			tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else
	{
		buf[0] = 0;
	}
	return;

} /* End of FormatTimeString() */


/*********************************************************************
 * Purpose: to show active game list
 * Return: None
 *********************************************************************/
void D2GSShowGameList(unsigned int ns)
{
	D2GAMEINFO		*lpGame;
	int				gamecount, charcount;
	char			buf[256];
	char			timestr[32];

	gamecount = charcount = 0;
	sprintf(buf, "+-No.--GameName---------GamePass---------ID----GameVer--Type--Difficulty--Ladder-----Users-CreateTime-Dis-+\r\n");
	SENDSTR(ns, buf);

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		FormatTimeString(lpGame->CreateTime, timestr, sizeof(timestr), 0);
		sprintf(buf, "| %03d  %-15s  %-15s  %-4u  %-7s  %-4s  %-11s %-10s %-5u %-10s %-3s |\r\n",
			gamecount + 1,
			lpGame->GameName,
			lpGame->game_pass,
			lpGame->GameId,
			lpGame->expansion ? "exp" : "classic",
			lpGame->hardcore ? "hc" : "sc",
			desc_game_difficulty[lpGame->difficulty % 3],
			lpGame->ladder ? "ladder" : "non-ladder",
			lpGame->CharCount,
			timestr,
			lpGame->disable ? "Y" : "N");
		SENDSTR(ns, buf);
		gamecount++;
		charcount += lpGame->CharCount;
		lpGame = lpGame->next;
		if (gamecount>500) break;
	}
	LeaveCriticalSection(&csGameList);

	sprintf(buf, "+---------------------------------------------------------------------------------------------------------+\r\n");
	SENDSTR(ns, buf);
	sprintf(buf, "\r\nTotal: %d games running, %d users in game.\r\n\r\n", gamecount, charcount);
	SENDSTR(ns, buf);
	return;

} /* End of D2GSShowGameList() */


/*********************************************************************
 * Purpose: to show active game list
 * Return: None
 *********************************************************************/
void D2GSShowCharInGame(unsigned int ns, WORD GameId)
{
	D2GAMEINFO		*lpGame;
	D2CHARINFO		*lpChar;
	char			buf[256];
	char			timestr[32];
	int				count;

	EnterCriticalSection(&csGameList);
	lpGame = D2GSFindGameInfoByGameId(GameId);
	if (!lpGame) {
		LeaveCriticalSection(&csGameList);
		sprintf(buf, "Game %u not found.\r\n\r\n", GameId);
		SENDSTR(ns, buf);
		return;
	}

	FormatTimeString(lpGame->CreateTime, timestr, sizeof(timestr), 0);

	sprintf(buf, "\r\n[GameName   : %-15s] [GamePass   : %-15s] [GameDesc : %-31s]\r\n[GameID     : %-15u] [GameVer    : %-15s] [GameType : %-31s]\r\n",
		lpGame->GameName,
		lpGame->game_pass,
		lpGame->game_desc,
		lpGame->GameId,
		lpGame->expansion ? "exp" : "classic",
		lpGame->hardcore ? "hc" : "sc"
	);
	SENDSTR(ns, buf);

	sprintf(buf, "[Difficult  : %-15s] [IsLadder   : %-15s] [UserCount: %-31u]\r\n[CreateTime : %-15s] [Disable    : %-15s] [         : %-31s]\r\n",
		desc_game_difficulty[lpGame->difficulty % 3],
		lpGame->ladder ? "ladder" : "non-ladder",
		lpGame->CharCount,
		timestr,
		lpGame->disable ? "Yes" : "No",
		""//sotre_D2CSSecrect_and_UpdateUrl
	);
	SENDSTR(ns, buf);

	sprintf(buf, "[CreatorAcct: %-15s] [CreatorChar: %-15s] [CreatorIP: %-31s]\r\n\r\n",
		lpGame->creator_acct_name,
		lpGame->creator_char_name,
		lpGame->creator_ip);
	SENDSTR(ns, buf);

	sprintf(buf, "+-No.--AcctName---------CharName---------IPAddress--------Class--Level--EnterTime-+\r\n");
	SENDSTR(ns, buf);
	count = 0;
	lpChar = lpGame->lpCharInfo;
	while(lpChar)
	{
		FormatTimeString(lpChar->EnterTime, timestr, sizeof(timestr), 0);
		sprintf(buf, "| %03d  %-15s  %-15s  %-15s  %-5s  %-5u  %-9s |\r\n",
			count + 1,
			lpChar->AcctName,
			lpChar->CharName,
			lpChar->RealmIPName,
			desc_char_class[lpChar->CharClass % 7],
			lpChar->CharLevel,
			timestr);
		SENDSTR(ns, buf);
		count++;
		lpChar = lpChar->next;
		if (count>8) break;
	}
	LeaveCriticalSection(&csGameList);
	sprintf(buf, "+---------------------------------------------------------------------------------+\r\n");
	SENDSTR(ns, buf);
	sprintf(buf, "\r\nTotal: %d charaters in this game \r\n\r\n", count);
	SENDSTR(ns, buf);

	return;

} /* End of D2GSShowCharInGame() */


/*********************************************************************
 * Purpose: to disable a game
 * Return: None
 *********************************************************************/
void D2GSDisableGameByGameId(unsigned int ns, WORD GameId)
{
	D2GAMEINFO		*lpGame;
	char			buf[128];

	lpGame = D2GSFindGameInfoByGameId(GameId);
	if (!lpGame) {
		SENDSTR(ns, "Game not found\r\n\r\n");
		return;
	}
	lpGame->disable = TRUE;
	sprintf(buf, "Game '%s' disable\r\n\r\n", lpGame->GameName);
	SENDSTR(ns, buf);
	return;

} /* End of D2GSDisableGameByGameId() */


/*********************************************************************
 * Purpose: to enable a game
 * Return: None
 *********************************************************************/
void D2GSEnableGameByGameId(unsigned int ns, WORD GameId)
{
	D2GAMEINFO		*lpGame;
	char			buf[128];

	lpGame = D2GSFindGameInfoByGameId(GameId);
	if (!lpGame) {
		SENDSTR(ns, "Game not found\r\n\r\n");
		return;
	}
	lpGame->disable = FALSE;
	sprintf(buf, "Game '%s' enable\r\n\r\n", lpGame->GameName);
	SENDSTR(ns, buf);
	return;

} /* End of D2GSEnableGameByGameId() */


/*********************************************************************
 * Purpose: to announce to all
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_all(DWORD dwMsgType, const char *msg)
{
	D2GAMEINFO		*lpGame;
	int				gamecount;
	int				charcount;
	MOTDCLIENT		*lpClient = 0, * lpClientTemp = 0;
	D2CHARINFO		*lpChar;

	EnterCriticalSection(&csGameList);
	gamecount = 0;
	lpGame = lpGameInfoHead;
	while(lpGame)
	{
		charcount = 0;
		lpChar = lpGame->lpCharInfo;
		while (lpChar)
		{
			charcount++;
			lpClientTemp = (MOTDCLIENT*)malloc(sizeof(MOTDCLIENT));
			if (!lpClientTemp)
			{
				D2GSSendClientChatMessage(lpChar->ClientId, dwMsgType,
					D2COLOR_ID_RED, "[administrator]", msg);
				D2GSEventLog("chat_message_announce_all", "out of memory");
			}
			else
			{
				lpClientTemp->list.next = (struct list_head*)lpClient;
				lpClientTemp->ClientId = lpChar->ClientId;
				lpClient = lpClientTemp;
			}
			lpChar = lpChar->next;
			if (charcount > 8) break;
		}
		lpGame = lpGame->next;
		if (gamecount>500) break;
	}
	LeaveCriticalSection(&csGameList);
	while (lpClient)
	{
		lpClientTemp = lpClient;
		lpClient = (MOTDCLIENT*)lpClient->list.next;
		D2GSSendClientChatMessage(lpClientTemp->ClientId, dwMsgType,
			D2COLOR_ID_RED, "[administrator]", msg);
		free(lpClientTemp);
	}

	return 0;
} /* End of chat_message_announce_all() */


/*********************************************************************
 * Purpose: to announce to a game
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_game(DWORD dwMsgType, WORD GameId, const char *msg)
{
	D2GAMEINFO	*lpGame;
	
	EnterCriticalSection(&csGameList);
	lpGame = D2GSFindGameInfoByGameId(GameId);
	if (lpGame==NULL) {
		LeaveCriticalSection(&csGameList);
		return -1;
	}
	chat_message_announce_game2(dwMsgType, lpGame, msg);
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of chat_message_announce_game() */


/*********************************************************************
 * Purpose: to announce to a game
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_game2(DWORD dwMsgType, D2GAMEINFO *lpGame, const char *msg)
{
	D2CHARINFO		*lpChar;
	int				charcount;
	MOTDCLIENT* lpClient = 0, * lpClientTemp = 0;

	EnterCriticalSection(&csGameList);
	charcount = 0;
	lpChar = lpGame->lpCharInfo;
	while(lpChar)
	{
		charcount++;
		lpClientTemp = (MOTDCLIENT*)malloc(sizeof(MOTDCLIENT));
		if (!lpClientTemp)
		{
			D2GSSendClientChatMessage(lpChar->ClientId, dwMsgType,
				D2COLOR_ID_RED, "[administrator]", msg);
			D2GSEventLog("chat_message_announce_game2", "out of memory");
		}
		else
		{
			lpClientTemp->list.next = (struct list_head*)lpClient;
			lpClientTemp->ClientId = lpChar->ClientId;
			lpClient = lpClientTemp;
		}
		lpChar = lpChar->next;
		if (charcount>8) break;
	}
	LeaveCriticalSection(&csGameList);
	while (lpClient)
	{
		lpClientTemp = lpClient;
		lpClient = (MOTDCLIENT*)lpClient->list.next;
		D2GSSendClientChatMessage(lpClientTemp->ClientId, dwMsgType,
			D2COLOR_ID_RED, "[administrator]", msg);
		free(lpClientTemp);
	}
	return 0;
} /* End of chat_message_announce_game2() */


/*********************************************************************
 * Purpose: to announce to a char
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_char(DWORD dwMsgType, const char *CharName, const char *msg)
{
	D2CHARINFO		*lpChar;

	EnterCriticalSection(&csGameList);
	lpChar = (D2CHARINFO*)charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
	if (!lpChar) {
		LeaveCriticalSection(&csGameList);
		return -1;
	}
	LeaveCriticalSection(&csGameList);
	D2GSSendClientChatMessage(lpChar->ClientId, dwMsgType,
		D2COLOR_ID_RED, "[administrator]", msg);
	return 0;
} /* End of chat_message_announce_char() */


/*********************************************************************
 * Purpose: to announce to a char
 * Return: 0 or -1
 *********************************************************************/
int chat_message_announce_char2(DWORD dwMsgType, DWORD dwClientId, const char *msg)
{
	D2GSSendClientChatMessage(dwClientId, dwMsgType, D2COLOR_ID_RED, "[administrator]", msg);
	return 0;
} /* End of chat_message_announce_char2() */


/*********************************************************************
 * Purpose: to add a client to the MOTD list
 * Return: 0 or -1
 *********************************************************************/
int D2GSMOTDAdd(DWORD dwClientId)
{
	MOTDCLIENT	*pmotd;

	pmotd = (MOTDCLIENT*)malloc(sizeof(MOTDCLIENT));
	if (pmotd==NULL) return -1;
	pmotd->ClientId = dwClientId;
	EnterCriticalSection(&csGameList);
	list_add_tail((struct list_head*)pmotd, &list_motd);
	LeaveCriticalSection(&csGameList);
	return 0;
} /* End of D2GSMOTDAdd() */


/*********************************************************************
 * Purpose: to add a client to the MOTD list
 * Return: 0 or -1
 *********************************************************************/
int D2GSSendMOTD(void)
{
	static int			count = 0;

	struct list_head	*p, *ptemp;
	MOTDCLIENT			*pmotd;
	char				motd_str[256];

	/* we do this one time per second */
	count++;
	if (count<(SEND_MOTD_INTERVAL/TIMER_TICK_IN_MS)) return 0;
	count = 0;

	strcpy(motd_str, d2gsconf.motd);
	string_color(motd_str);

	EnterCriticalSection(&csGameList);
	list_for_each_safe(p, ptemp, &list_motd)
	{
		pmotd = list_entry(p, MOTDCLIENT, list);
		chat_message_announce_char2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, "\xFF" "c8[D2GS] - No commercial purpose is allowed!\xFF" "c4");
		chat_message_announce_char2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, motd_str);
		chat_message_announce_char2(CHAT_MESSAGE_TYPE_SYS_MESSAGE, pmotd->ClientId, gWorldEventMessage);
		list_del(p);
		free(pmotd);
	}
	LeaveCriticalSection(&csGameList);
	return 0;

} /* End of D2GSSendMOTD() */


BOOL D2GSCheckGameInfo()
{
	return (lpGameInfoHead != 0);
}


DWORD WINAPI SaveAllGamesThread(LPVOID ptr)
{
	D2GSEventLog("SaveAllGamesThread", "Calling D2GSEndAllGames()");
	D2GSEndAllGames();
	D2GSEventLog("SaveAllGamesThread", "Waiting for all games to be saved");
	while (lpGameInfoHead != 0)
	{
		Sleep(1);
	}
	D2GSEventLog("SaveAllGamesThread", "All games saved");
	return 0;
}


BOOL D2GSSaveAllGames(DWORD dwMilliseconds)
{
	DWORD dwThreadId = 0;
	HANDLE hTempThread = INVALID_HANDLE_VALUE;
	if (D2GSEndAllGames == NULL)
	{
		return FALSE;
	}
	hTempThread = CreateThread(0, 0, SaveAllGamesThread, NULL, 0, &dwThreadId);
	if (WaitForSingleObject(hTempThread, dwMilliseconds) == WAIT_TIMEOUT)
	{
		TerminateThread(hTempThread, 1);
		CloseHandle(hTempThread);
		D2GSEventLog("SaveAllGames", "SaveAllGamesThread timetout");
		return FALSE;
	}
	CloseHandle(hTempThread);
	return TRUE;
}


typedef struct CheckCharLife
{
	struct CheckCharLife* Next;
	DWORD OverLastMinute;
	DWORD ClientId;
}CheckCharLife;


void D2GSCheckGameLife()
{
	static DWORD CheckCount = 0;
	time_t curTime = 0;
	D2CHARINFO* lpChar = 0;
	D2GAMEINFO* lpGame = 0;
	DWORD overLastMinute = 0;
	CheckCharLife* pCheckCharLife = 0;
	CheckCharLife* pCheckCharHead = NULL;
	if (d2gsconf.maxgamelife == 0)
	{
		return;
	}

	CheckCount++;

	if (CheckCount < 600)
	{
		return;
	}

	curTime = time(NULL);

	EnterCriticalSection(&csGameList);
	lpGame = lpGameInfoHead;
	while (lpGame != 0)
	{
		if (lpGame->hardcore == 0)
		{
			if ((curTime - lpGame->CreateTime) > d2gsconf.maxgamelife)
			{
				lpChar = lpGame->lpCharInfo;
				overLastMinute = ((curTime - lpGame->CreateTime) >= (d2gsconf.maxgamelife + 60));
				/*
					cmp eax, edx; >= cf=0, < cf=1
					sbb edi,edi -> edi - edi - cf -> 0, -1
					inc edi : 1 0
					eax > edx -> edi = 1
					eax < edx -> edi = 0
				*/
				while (lpChar)
				{
					pCheckCharLife = (CheckCharLife*)malloc(sizeof(CheckCharLife));
					if (pCheckCharLife != 0)
					{
						pCheckCharLife->Next = pCheckCharHead;
						pCheckCharLife->OverLastMinute = overLastMinute;
						pCheckCharLife->ClientId = lpChar->ClientId;
						pCheckCharHead = pCheckCharLife;
					}
					else
					{
						if (overLastMinute)
						{
							// remove immediately
							D2GSRemoveClientFromGame(lpChar->ClientId);
						}
						else
						{
							// send warning
							chat_message_announce_char2(
								CHAT_MESSAGE_TYPE_SYS_MESSAGE,
								lpChar->ClientId,
								"[SERVER]: This game will end after 1 minute for time limit!"
							);
						}
						D2GSEventLog("D2GSCheckGamelife", "out of memory");
					}
					lpChar = lpChar->next;
				}
			}
		}
		lpGame = lpGame->next;
	}
	LeaveCriticalSection(&csGameList);

	while (pCheckCharHead != 0)
	{
		if (pCheckCharHead->OverLastMinute)
		{
			// remove immediately
			D2GSRemoveClientFromGame(pCheckCharHead->ClientId);
		}
		else
		{
			// send warning
			chat_message_announce_char2(
				CHAT_MESSAGE_TYPE_SYS_MESSAGE,
				pCheckCharHead->ClientId,
				"[SERVER]: This game will end after 1 minute for time limit!"
			);
		}
		pCheckCharLife = pCheckCharHead;
		pCheckCharHead = pCheckCharHead->Next;
		free(pCheckCharLife);
	}
}

}

}