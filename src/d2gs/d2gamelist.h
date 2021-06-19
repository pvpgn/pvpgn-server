#ifndef INCLUDED_D2GAMELIST_H
#define INCLUDED_D2GAMELIST_H

/*
 * define structure to store game info, store character info in the game
 * and the functions to maintain the game and character list
 */

#include <windows.h>
#include "d2gs.h"
#include "bn_types.h"
#include "list.h"

namespace pvpgn
{

	namespace d2gs
	{

		/* structures */
		typedef struct RAW_D2CHARINFO
		{
			char		AcctName[MAX_ACCTNAME_LEN];	/* account name */
			char		CharName[MAX_CHARNAME_LEN];	/* char name */
			char		RealmIPName[MAX_REALMIPNAME_LEN]; /* realm name */
			bn_int		token;
			bn_int		CharLevel;
			bn_short	CharClass;
			bn_short	TickCount;
			bn_short	EnterGame;
			bn_short	AllowLadder;
			bn_short	CharLockStatus;
			time_t		EnterTime;
			time_t		CharCreateTime;
			DWORD		ClientId;
			WORD		GameId;
			struct RAW_D2GAMEINFO* lpGameInfo;	/* pointer to the GAMEINFO */
			struct RAW_D2CHARINFO* prev;
			struct RAW_D2CHARINFO* next;
		} D2CHARINFO, * PD2CHARINFO, * LPD2CHARINFO;

		typedef struct RAW_D2GAMEINFO
		{
			char		GameName[MAX_GAMENAME_LEN];
			char		game_pass[MAX_GAMEPASS_LEN];
			char		game_desc[MAX_GAMEDESC_LEN];
			char		creator_acct_name[MAX_ACCTNAME_LEN];
			char		creator_char_name[MAX_CHARNAME_LEN];
			char		creator_ip[MAX_REALMIPNAME_LEN];
			bn_byte		ladder;
			bn_byte		expansion;
			bn_byte		difficulty;
			bn_byte		hardcore;
			bn_byte		reserved;
			WORD		GameId;
			WORD		CharCount;
			time_t		CreateTime;
			DWORD		disable;
			struct RAW_D2CHARINFO* lpCharInfo;
			struct RAW_D2GAMEINFO* prev;
			struct RAW_D2GAMEINFO* next;
		} D2GAMEINFO, * PD2GAMEINFO, * LPD2GAMEINFO;


		typedef struct RAW_D2GETDATAREQUEST
		{
			char		AcctName[MAX_ACCTNAME_LEN];	/* account name */
			char		CharName[MAX_CHARNAME_LEN];	/* char name */
			DWORD		ClientId;
			DWORD		Seqno;
			bn_short	TickCount;
			struct RAW_D2GETDATAREQUEST* prev;
			struct RAW_D2GETDATAREQUEST* next;
		} D2GETDATAREQUEST, * PD2GETDATAREQUEST, * LPD2GETDATAREQUEST;

		typedef struct RAW_MOTDCLIENT
		{
			struct list_head	list;
			DWORD				ClientId;
		} MOTDCLIENT, * PMOTDCLIENT, * LPMOTDCLIENT;

		/* macro defination */
#define D2GSIncCurrentGameNumber()		{ currentgamenum++; }
#define D2GSDecCurrentGameNumber()		{ currentgamenum--; }

/* functions */
		void D2GSResetGameList(void);
		int  D2GSGetCurrentGameNumber(void);
		int  D2GSGetCurrentGameStatistic(DWORD* gamenum, DWORD* usernum);
		void D2GSDeleteAllCharInGame(D2GAMEINFO* lpGameInfo);
		int  D2GSGameListInsert(const char* game_name, const char* game_pass, const char* game_desc,
			const char* creator_acct_name, const char* creator_char_name, const char* creator_ip,
			UCHAR expansion, UCHAR difficulty, UCHAR hardcore, UCHAR ladder, WORD wGameId);
		int  D2GSGameListDelete(D2GAMEINFO* lpGameInfo);
		int  D2GSInsertCharIntoGameInfo(D2GAMEINFO* lpGameInfo, DWORD token, const char* AcctName,
			const char* CharName, const char* RealmIPName, DWORD CharLevel, WORD CharClass, WORD EnterGame);
		int  D2GSDeleteCharFromGameInfo(D2GAMEINFO* lpGameInfo, D2CHARINFO* lpCharInfo);
		int  D2GSInsertCharIntoPendingList(DWORD token, const char* AcctName,
			const char* CharName, const char* RealmIPName, DWORD CharLevel, WORD CharClass, D2GAMEINFO* lpGame);
		int  D2GSDeletePendingChar(D2CHARINFO* lpCharInfo);
		int  D2GSInsertGetDataRequest(const char* AcctName, const char* CharName, DWORD dwClientId, DWORD dwSeqno);
		int  D2GSDeleteGetDataRequest(D2GETDATAREQUEST* lpGetDataReq);
		D2GAMEINFO* D2GSFindGameInfoByGameId(WORD GameId);
		D2GAMEINFO* D2GSFindGameInfoByGameName(const char* GameName);
		D2CHARINFO* D2GSFindCharInGameByCharName(D2GAMEINFO* lpGame, const char* CharName);
		D2CHARINFO* D2GSFindPendingCharByToken(DWORD token);
		D2CHARINFO* D2GSFindPendingCharByCharName(const char* CharName);
		D2GETDATAREQUEST* D2GSFindGetDataRequestBySeqno(DWORD dwSeqno);
		void D2GSPendingCharTimerRoutine(void);
		void D2GSGetDataRequestTimerRoutine(void);
		void FormatTimeString(time_t t, char* buf, int len, int format);
		void D2GSShowGameList(unsigned int ns);
		void D2GSShowCharInGame(unsigned int ns, WORD GameId);
		void D2GSDisableGameByGameId(unsigned int ns, WORD GameId);
		void D2GSEnableGameByGameId(unsigned int ns, WORD GameId);

		int chat_message_announce_all(DWORD dwMsgType, const char* msg);
		int chat_message_announce_game(DWORD dwMsgType, WORD GameId, const char* msg);
		int chat_message_announce_game2(DWORD dwMsgType, D2GAMEINFO* lpGame, const char* msg);
		int chat_message_announce_char(DWORD dwMsgType, const char* CharName, const char* msg);
		int chat_message_announce_char2(DWORD dwMsgType, DWORD dwClientId, const char* msg);

		int D2GSMOTDAdd(DWORD dwClientId);
		int D2GSSendMOTD(void);
		BOOL D2GSCheckGameInfo();
		void D2GSCheckGameLife();
		BOOL D2GSSaveAllGames(DWORD dwMilliseconds);

	}

}


#endif /* INCLUDED_D2GAMELIST_H */