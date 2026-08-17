#ifndef INCLUDED_HANDLE_S2S_H
#define INCLUDED_HANDLE_S2S_H


namespace pvpgn
{

	namespace d2gs
	{

		/* structure */
		typedef struct
		{
			char		realmname[MAX_REALMNAME_LEN];
			DWORD		sessionnum;
			DWORD		gsactive;
		} D2GSPARAM, * PD2GSPARAM, * LPD2GAPARAM;


		/* for check sum calculation */
#ifndef ROTL
#define ROTL(x,n,w) ( ((n)%(w)) ? (((x)<<((n)%(w))) | ((x)>>((w)-((n)%(w))))) : (x) )
#endif

/* functions */
		int D2GSInitializeS2S(void);
		void D2GSActive(int flag);
		void str2lower(unsigned char* str);
		DWORD D2GSGetSequence(void);
		DWORD D2GSGetCheckSum(void);
		void D2GSSendClassToD2CS(void);
		void D2GSSendClassToD2DBS(void);
		void D2GSHandleS2SPacket(D2GSPACKET* lpPacket);
		void D2GSAuthreq(int peer, LPVOID lpdata);
		void D2GSAuthReply(int peer, LPVOID lpdata);
		void D2GSSetD2CSMaxGameNumber(DWORD maxgamenum);
		void D2XSEchoReply(int peer, LPVOID lpdata);
		void D2CSControlCmd(int peer, LPVOID lpdata);
		void D2GSSetInitInfo(int peer, LPVOID lpdata);
		void D2GSSetConfFile(int peer, LPVOID lpdata);
		void D2CSCreateEmptyGame(int peer, LPVOID lpdata);
		void D2CSClientJoinGameRequest(int peer, LPVOID lpdata);

		/* by callback function */
		BOOL D2GSCBFindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
			LPSTR lpAccountName, LPPLAYERDATA lpPlayerData);
		void D2GSCBEnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass,
			DWORD dwCharLevel, DWORD dwReserved);
		void D2GSCBLeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass,
			DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
			WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
			BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
			LPCSTR lpAccountName, PLAYERDATA PlayerData,
			PLAYERMARK PlayerMark);
		void D2GSCBCloseGame(WORD wGameId);
		void D2GSCBUpdateGameInformation(WORD wGameId, LPCSTR lpCharName,
			WORD wCharClass, DWORD dwCharLevel);
		void D2GSCBGetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			DWORD dwClientId, LPCSTR lpAccountName);
		void D2GSCBSaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
			LPCSTR lpAccountName, LPVOID lpSaveData,
			DWORD dwSize, PLAYERDATA PlayerData);
		void D2GSWriteCharInfoFile(LPCSTR lpAccountName, LPCSTR lpCharName,
			WORD wCharClass, DWORD dwCharLevel, DWORD dwExpLow,
			WORD wCharStatus, LPCSTR lpCharPortrait);
		void D2GSUpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass, DWORD dwCharLevel,
			DWORD dwCharExpLow, DWORD dwCharExpHigh, WORD wCharStatus);
		void D2GSLoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion);

		/* by d2dbs */
		void D2DBSSaveDataReply(int peer, LPVOID lpdata);
		void D2DBSGetDataReply(int peer, LPVOID lpdata);
		void D2GSSetCharLockStatus(const char* lpAccountName, const char* lpCharName, const char* RealmName, DWORD CharLockStatus);
		void D2GSUnlockChar(LPCSTR lpAccountName, LPCSTR lpCharName);


		void D2GSSetGameInfoByD2CS(int peer, LPVOID  lpPacketData);
		void D2GSSetConfFile(int peer, LPVOID  lpPacketData);
		void D2GSSetDifficultyCount(int difficulty, int R0/*=0*/, int R1/*=0xFFFFFFFF*/);
		void UpdateMaxPreferUsers();

	}

}


#endif /* INCLUDED_HANDLE_S2S_H */