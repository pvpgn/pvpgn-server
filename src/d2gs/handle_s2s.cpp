#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <d2server.h>
#include "d2gs.h"
#include "vars.h"
#include "eventlog.h"
#include "net.h"
#include "bn_types.h"
#include "handle_s2s.h"
#include "d2gamelist.h"
#include "connection.h"
#include "charlist.h"
#include "d2cs_d2gs_protocol.h"
#include "d2cs_d2gs_character.h"
#include "d2dbs_d2gs_protocol.h"
#include "utils.h"
#include "d2ge.h"


namespace pvpgn
{

	namespace d2gs
	{

		static D2GSPARAM	d2gsparam;

		static char desc_game_difficulty[][32] = {
			"normal", "nightmare", "hell"
		};

		static char desc_char_class[][16] = {
			"Ama", "Sor", "Nec", "Pal", "Bar", "Dur", "Ass"
		};


		/*********************************************************************
		 * Purpose: to lower case the given string
		 * Return: sequence
		 *********************************************************************/
		int D2GSInitializeS2S(void)
		{
			ZeroMemory(&d2gsparam, sizeof(d2gsparam));
			d2gsparam.gsactive   = FALSE;
			d2gsparam.sessionnum = 0;
			return 0;

		} /* End of D2GSInitializeS2S() */


		/*********************************************************************
		 * Purpose: to active or deactive this game server for D2CS
		 * Return: none
		 *********************************************************************/
		void D2GSActive(int flag)
		{
			d2gsparam.gsactive = flag;
			if (!flag)
			{
				d2gsconf.curgsmaxgames = 0;
			}
			return;
		} /* End of D2GSActive() */


		/*********************************************************************
		 * Purpose: to check if game server is active or deactive
		 * Return: TRUE of FALSE
		 *********************************************************************/
		int D2GSIsActive(void)
		{
			return (d2gsparam.gsactive);

		} /* End of D2GSIsActive() */


		/*********************************************************************
		 * Purpose: to lower case the given string
		 * Return: sequence
		 *********************************************************************/
		void str2lower(unsigned char *str)
		{
			unsigned char	*p;

			if (!str) return;
			p = str;
			while((*p) != '\0')
			{
				if (isalpha(*p)) (*p)|=0x20;
				p++;
			}
			return;
		}


		/*********************************************************************
		 * Purpose: build up sequence
		 * Return: sequence
		 *********************************************************************/
		DWORD D2GSGetSequence(void)
		{
			static DWORD	sequence = 0;

			return ++sequence;

		} /* End of D2GSGetSequence() */


		/*********************************************************************
		 * Purpose: get the check of the server file
		 * Return: sequence
		 *********************************************************************/
		DWORD D2GSGetCheckSum(void)
		{
			DWORD				checksum;
			DWORD				port, addr;
			unsigned int		sessionnum;
			unsigned int		i, len, ch;
			const char*			realmname;
			const char*			d2cssecrect;

			checksum    = d2gsconf.checksum;
			realmname   = d2gsparam.realmname;
			sessionnum  = d2gsparam.sessionnum;
			d2cssecrect = d2gsconf.d2cssecrect;
			if (D2GSGetSockName(D2CSERVER, &addr, &port)) return 0;

			len = strlen(realmname);
			for(i=0; i<len ; i++) {
				ch = (unsigned int)(realmname[i]);
				checksum ^= ROTL(sessionnum, i, sizeof(unsigned int) * CHAR_BIT);
				checksum ^= ROTL(port, ch, sizeof(unsigned int) * CHAR_BIT);
			}
			len = strlen(d2cssecrect);
			for(i=0; i<len ; i++) {
				ch = (unsigned int)(d2cssecrect[i]);
				checksum ^= ROTL(sessionnum, i, sizeof(unsigned int) * CHAR_BIT);
				checksum ^= ROTL(port, ch, sizeof(unsigned int) * CHAR_BIT);
			}
			checksum ^= addr;

			return checksum;

		} /* End of D2GSGetFileCheckSum() */


		/*********************************************************************
		 * Purpose: to send an identified class char to D2CS when connected
		 * Return: None
		 *********************************************************************/
		void D2GSSendClassToD2CS(void)
		{
			D2GSPACKET		packet;
			t_d2gs_connect	*pcon;

			pcon = (t_d2gs_connect *)(packet.data);
			packet.peer = PACKET_PEER_SEND_TO_D2CS;
			packet.datalen = sizeof(t_d2gs_connect);
			pcon->bnclass = CONNECT_CLASS_D2GS_TO_D2CS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSSendClassToD2CS", "Send connection class packet to D2CS");
			return;
		}


		/*********************************************************************
		 * Purpose: to send an identified class char to D2DBS when connected
		 * Return: None
		 *********************************************************************/
		void D2GSSendClassToD2DBS(void)
		{
			D2GSPACKET		packet;
			t_d2gs_connect	*pcon;

			pcon = (t_d2gs_connect *)(packet.data);
			packet.peer = PACKET_PEER_SEND_TO_D2DBS;
			packet.datalen = sizeof(t_d2gs_connect);
			pcon->bnclass = CONNECT_CLASS_D2GS_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSSendClassToD2DBS", "Send connection class packet to D2DBS");
			return;
		}


		std::unordered_map<decltype(t_d2cs_d2gs_header::type), std::function<void(int, LPVOID)>> s2s_packet_table = {
			{ D2CS_D2GS_AUTHREQ,			D2GSAuthreq },
			{ D2CS_D2GS_AUTHREPLY,			D2GSAuthReply },
			{ D2GS_D2CS_SETGSINFO,			D2GSSetGameInfoByD2CS },
			{ D2CS_D2GS_ECHOREQ,			D2XSEchoReply },
			{ D2CS_D2GS_CONTROL,			D2CSControlCmd },
			{ D2CS_D2GS_SETINITINFO,		D2GSSetInitInfo },
			{ D2CS_D2GS_SETCONFFILE,		D2GSSetConfFile },
			{ D2CS_D2GS_CREATEGAMEREQ,		D2CSCreateEmptyGame },
			{ D2CS_D2GS_JOINGAMEREQ,		D2CSClientJoinGameRequest },
			{ D2DBS_D2GS_SAVE_DATA_REPLY,	D2DBSSaveDataReply },
			{ D2DBS_D2GS_GET_DATA_REPLY,	D2DBSGetDataReply },
			{ D2DBS_D2GS_ECHOREQUEST,		D2XSEchoReply }
		};


		/*********************************************************************
		 * Purpose: to deal with a packet received
		 * Return: None
		 *********************************************************************/
		void D2GSHandleS2SPacket(D2GSPACKET *lpPacket)
		{
			t_d2cs_d2gs_header* lpcshead;

			if (!lpPacket) return;

			lpcshead = (t_d2cs_d2gs_header*)(lpPacket->data);

			if (lpPacket->datalen != bn_ntohs(lpcshead->size))
			{
				D2GSEventLog("D2GSHandleS2SPacket",
					"Packet size incorrect!",
					lpcshead->type
				);
				return;
			}

			std::function<void(int, LPVOID)> packet_handler;
			try
			{
				packet_handler = s2s_packet_table.at(lpcshead->type);
			}
			catch (const std::out_of_range&)
			{
				D2GSEventLog("D2GSHandleS2SPacket", "No packet handler for packet type 0x%02x", lpcshead->type);
				return;
			}

			try
			{
				packet_handler(lpPacket->peer, static_cast<LPVOID>(lpPacket->data));
			}
			catch (const std::exception& e)
			{
				D2GSEventLog("D2GSHandleS2SPacket", "Caught exception: %s", e.what());
				return;
			}
		} /* End of D2GSHandleS2SPacket() */


		/*********************************************************************
		 * Purpose: D2GSAutherq
		 * Return:  None
		 *********************************************************************/
		void D2GSAuthreq(int peer, LPVOID lpdata)
		{
			DWORD		seqno;
			D2GSPACKET	packet;
			t_d2cs_d2gs_authreq		*preq;

			preq = (t_d2cs_d2gs_authreq *)(lpdata);

			// Deprecated
			//bn_int signlen = bn_ntohl(preq->signlen);

			/* get realm name */
			char realm_name[MAX_REALMNAME_LEN] = { 0 };
			snprintf(realm_name, sizeof(realm_name), "%s", (char*)(preq + 1));
			strcpy(d2gsparam.realmname, realm_name);

			/* get session number */
			d2gsparam.sessionnum = bn_ntohl(preq->sessionnum);
			if ((strlen(realm_name)<=0)  || (strlen(realm_name)>=MAX_REALMNAME_LEN)) return;
			seqno = bn_ntohl(preq->h.seqno);
	
			ZeroMemory(&packet, sizeof(packet));
			t_d2gs_d2cs_authreply* preply = (t_d2gs_d2cs_authreply *)(packet.data);
			preply->h.type   = bn_htons(D2GS_D2CS_AUTHREPLY);
			preply->h.size   = bn_ntohs(sizeof(t_d2gs_d2cs_authreply));
			preply->h.seqno  = bn_htonl(seqno);
			preply->version  = bn_ntohl(D2GS_VERSION);
			preply->checksum = bn_ntohl(D2GSGetCheckSum());
			preply->randnum = bn_htonl(time(0)); // Deprecated
			preply->signlen = bn_htonl(0); // Deprecated
			memset(preply->sign, 0, sizeof(preply->sign)); // Deprecated
			packet.peer      = PACKET_PEER_SEND_TO_D2CS;
			packet.datalen   = sizeof(t_d2gs_d2cs_authreply);
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSAuthreq", "Auth request for '%s'", realm_name);
		} /* End of D2GSAuthreq() */


		/*********************************************************************
		 * Purpose: D2GSAutherq
		 * Return:  None
		 *********************************************************************/
		void D2GSAuthReply(int peer, LPVOID lpdata)
		{
			t_d2cs_d2gs_authreply	*preq;

			preq = (t_d2cs_d2gs_authreply *)(lpdata);

			if (preq->reply == D2CS_D2GS_AUTHREPLY_SUCCEED)
			{
				D2GEReloadConfig();

				D2GSActive(TRUE);
				D2GSEventLog(__FUNCTION__, "Game server activated by D2CS");

				D2GSSetD2CSMaxGameNumber(d2gsconf.gsmaxgames);

				return;
			}
			else if (preq->reply == D2CS_D2GS_AUTHREPLY_BAD_VERSION)
			{
				d2gsconf.enablegslog = TRUE;
				D2GSEventLog("D2GSAuthReply", "version mismatch, need update");
				D2GSBeforeShutdown(1, 0);
				d2gsconf.enablegslog = FALSE;

			}
			else if (preq->reply == D2CS_D2GS_AUTHREPLY_BAD_CHECKSUM)
			{
				d2gsconf.enablegslog = TRUE;
				D2GSEventLog("D2GSAuthReply", "version mismatch, need update");
				D2GSBeforeShutdown(1, 0);
				d2gsconf.enablegslog = FALSE;

			}

			CloseConnectionToD2CS(); // error occur, disconnect

		} /* End of D2GSAuthReply() */


		void D2GSSetGameInfoByD2CS(int peer, LPVOID lpPacketData)
		{
			t_d2gs_d2cs_setgsinfo* pData = (t_d2gs_d2cs_setgsinfo*)lpPacketData;
			if (pData->maxgame <= d2gsconf.gsmaxgames)
			{
				D2GSEventLog("D2GSSetGameInfoByD2CS", "Set current maxgame to %ld", pData->maxgame);
				d2gsconf.curgsmaxgames = pData->maxgame;
				return;
			}
			D2GSEventLog("D2GSSetGameInfoByD2CS", "Got invalid maxgame(%ld) from D2CS", pData->maxgame);
		} /* End of D2GSSetGameInfoByD2CS() */


		/*********************************************************************
		 * Purpose: D2GSSetD2CSMaxGameNumber
		 * Return:  None
		 *********************************************************************/
		void D2GSSetD2CSMaxGameNumber(DWORD maxgamenum)
		{
			D2GSPACKET				packet;
			t_d2gs_d2cs_setgsinfo	*preply;
			DWORD finalMaxGames = (maxgamenum < d2gsconf.gemaxgames) ? maxgamenum : d2gsconf.gemaxgames;
	
			if (D2GSIsActive())
			{
				/* send update info to D2CS */
				ZeroMemory(&packet, sizeof(packet));
				preply = (t_d2gs_d2cs_setgsinfo*)(packet.data);
				preply->h.type = bn_htons(D2GS_D2CS_SETGSINFO);
				preply->h.size = bn_htons(sizeof(t_d2gs_d2cs_setgsinfo));
				preply->h.seqno = bn_htonl(D2GSGetSequence());
				preply->maxgame = bn_htonl(finalMaxGames);
				preply->gameflag = bn_htonl(0);
				packet.datalen = sizeof(t_d2gs_d2cs_setgsinfo);
				packet.peer = PACKET_PEER_SEND_TO_D2CS;
				D2GSNetSendPacket(&packet);
				D2GSEventLog("D2GSSetD2CSMaxGameNumber", "Tell D2CS to set max game to %lu", finalMaxGames);
			}
			else
			{
				D2GSEventLog("D2GSSetD2CSMaxGameNumber", "Not Active.");
			}

			return;

		} /* End of D2GSSetD2CSMaxGameNumber() */


		/*********************************************************************
		 * Purpose: echo reply to the echo reuest 
		 * Return: None
		 *********************************************************************/
		void D2XSEchoReply(int peer, LPVOID lpdata)
		{
			D2GSPACKET				packet;
			t_d2gs_d2cs_echoreply* pcsreply;
			t_d2gs_d2dbs_echoreply* pdbsreply;

			ZeroMemory(&packet, sizeof(packet));
			if (peer == D2CSERVER)
			{
				pcsreply = (t_d2gs_d2cs_echoreply*)(packet.data);
				pcsreply->h.type = bn_htons(D2GS_D2CS_ECHOREPLY);
				pcsreply->h.size = bn_htons(sizeof(t_d2gs_d2cs_echoreply));
				pcsreply->h.seqno = bn_htonl(D2GSGetSequence());
				packet.datalen = sizeof(t_d2gs_d2cs_echoreply);
				packet.peer = PACKET_PEER_SEND_TO_D2CS;
				D2GSNetSendPacket(&packet);
			}
			else
			{
				pdbsreply = (t_d2gs_d2dbs_echoreply*)(packet.data);
				pdbsreply->h.type = bn_htons(D2GS_D2DBS_ECHOREPLY);
				pdbsreply->h.size = bn_htons(sizeof(t_d2gs_d2dbs_echoreply));
				pdbsreply->h.seqno = bn_htonl(D2GSGetSequence());
				packet.datalen = sizeof(t_d2gs_d2dbs_echoreply);
				packet.peer = PACKET_PEER_SEND_TO_D2DBS;
				D2GSNetSendPacket(&packet);
			}
			/*else
			{
				D2GSEventLog("D2XSEchoReply", "unknow peer %d", peer);
			}*/
			return;

		} /* End of D2XSEchoReply() */


		/*********************************************************************
		 * Purpose: Fulfill server restart and shutdown commands by D2CS.
		 * Return: None
		 *********************************************************************/
		void D2CSControlCmd(int peer, LPVOID lpdata)
		{
			t_d2cs_d2gs_control* pCtrl = (t_d2cs_d2gs_control*)lpdata;
			if (pCtrl->cmd == 1)
			{
				if (pCtrl->value == 0)
				{
					D2GSBeforeShutdown(3, 0x12C);
				}
				else
				{
					D2GSBeforeShutdown(3, pCtrl->value);
				}
			}
			else if (pCtrl->cmd == 2)
			{
				if (pCtrl->value == 0)
				{
					D2GSBeforeShutdown(4, 0x12C);
				}
				else
				{
					D2GSBeforeShutdown(4, pCtrl->value);
				}
			}
		} /* End of D2CSControlCmd() */


		/*********************************************************************
		 * Purpose: To set anticheat settings from D2CS.
		 * Return: None
		 * Deprecated
		 *********************************************************************/
		void D2GSSetInitInfo(int peer, LPVOID lpdata)
		{
			DWORD	dwCount = 0;
			DWORD* lpCheckSumNumberArray = gD2GSInfo.dwCheckSumArray;
			const char* lpCheckSum = 0;
			char** lpArrayTemp = NULL;
			char** lpArray = NULL;
			t_d2cs_d2gs_setinitinfo* ptr = (t_d2cs_d2gs_setinitinfo*)lpdata;
			D2GSSetTickCount(ptr->time);
			gD2GSInfo.dwACVersion = ptr->ac_version;
			gD2GSInfo.dwGSId = ptr->gs_id;
			lpCheckSum = (const char*)(ptr + 1);
			lpArray = strtoarray(lpCheckSum, ",", (int*)&dwCount);
			if (!lpArray)
			{
				gD2GSInfo.dwCheckSum0 = 0;
				gD2GSInfo.dwRealCheckSumCount = 0;
				D2GSSetACData((const char*)(lpCheckSum + strlen(lpCheckSum) + 1));
				return;
			}

			if (dwCount < 1)
			{
				gD2GSInfo.dwCheckSum0 = 0;
				gD2GSInfo.dwRealCheckSumCount = 0;
				free(lpArray);
				D2GSSetACData((const char*)(lpCheckSum + strlen(lpCheckSum) + 1));
				return;
			}

			gD2GSInfo.dwCheckSum0 = strtoul(*lpArray, 0, 0);
			dwCount--;
			if (dwCount > 16)
			{
				dwCount = 16;
			}

			gD2GSInfo.dwRealCheckSumCount = dwCount;

			if (dwCount <= 0)
			{
				free(lpArray);
				D2GSSetACData((const char*)(lpCheckSum + strlen(lpCheckSum) + 1));
				return;
			}

			lpArrayTemp = lpArray + 1;
			for (DWORD i = 0; i < dwCount; i++, lpCheckSumNumberArray++, lpArrayTemp++)
			{
				*lpCheckSumNumberArray = strtoul(*lpArrayTemp, 0, 0);
			}
			free(lpArray);
			D2GSSetACData((const char*)(lpCheckSum + strlen(lpCheckSum) + 1));
			return;

		} /* End of D2GSSetInitInfo() */


		/*********************************************************************
		 * Purpose: To update d2server.ini from D2CS.
		 * Return: None
		 *********************************************************************/
		void D2GSSetConfFile(int peer, LPVOID lpdata)
		{
			t_d2cs_d2gs_setconffile* ptr = (t_d2cs_d2gs_setconffile*)lpdata;
			FILE* f = 0;
			DWORD size = ptr->size;
			char buffer[260] = { 0 };
			UCHAR* lpBuffer = 0;
			DWORD bytes;
			if (size > 0x2800)
			{
				size = 0x2800;
			}
			lpBuffer = static_cast<UCHAR*>(malloc(size + 8));
			if (!lpBuffer)
			{
				D2GSEventLog("D2GSSetConfFile", "Not enough memory");
				return;
			}
			CopyMemory(lpBuffer, (UCHAR*)(ptr + 1), size);
			snprintf(buffer, 259, "%s\\%s", PathName, d2gsconf.serverconffile);
			f = fopen(buffer, "w");
			if (!f)
			{
				D2GSEventLog("D2GSSetConfFile", "failed writing %s", d2gsconf.serverconffile);
				free(lpBuffer);
				return;
			}
			bytes = fwrite(lpBuffer, 1, size, f);
			fclose(f);
			D2GSEventLog("D2GSSetConfFile", "get %s from D2CS", d2gsconf.serverconffile);
			free(lpBuffer);
			return;

		} /* End of D2GSSetConfFile() */


		/*********************************************************************
		 * Purpose: to create a new empty game on GE
		 * Return: None
		 *********************************************************************/
		void D2CSCreateEmptyGame(int peer, LPVOID lpdata)
		{
			char		GamePass[MAX_GAMEPASS_LEN] = { 0 };
			char		GameName[MAX_GAMENAME_LEN] = { 0 };
			char		AcctName[MAX_ACCTNAME_LEN] = { 0 };
			char		CharName[MAX_CHARNAME_LEN] = { 0 };
			char		IPName[MAX_REALMIPNAME_LEN] = { 0 };
			char		GameDesc[MAX_GAMEDESC_LEN] = { 0 };
			DWORD		dwGameFlag;
			DWORD		wGameId = 0;
			DWORD		seqno;
			DWORD		size = 0;
			const UCHAR* ptr = 0;
			D2GSPACKET* packet = static_cast<D2GSPACKET *>(malloc(sizeof(D2GSPACKET)));
			t_d2cs_d2gs_creategamereq	*preq;
			t_d2gs_d2cs_creategamereply	*preply;

			preq = (t_d2cs_d2gs_creategamereq *)(lpdata);
			ptr = (UCHAR*)(preq + 1);
			CopyMemory(GameName, ptr, sizeof(GameName));
			GameName[MAX_GAMENAME_LEN - 1] = '\0';

			if (strlen(GameName) <= 0)
			{
				return;
			}

			size += strlen(GameName) + 1;
			ptr += strlen(GameName) + 1;

			CopyMemory(GamePass, ptr, sizeof(GamePass));
			GamePass[MAX_GAMEPASS_LEN - 1] = '\0';

			size += strlen(GamePass) + 1;
			ptr += strlen(GamePass) + 1;

			CopyMemory(GameDesc, ptr, sizeof(GameDesc));
			GameDesc[MAX_GAMEDESC_LEN - 1] = '\0';

			size += strlen(GameDesc) + 1;
			ptr += strlen(GameDesc) + 1;

			CopyMemory(AcctName, ptr, sizeof(AcctName));
			AcctName[MAX_ACCTNAME_LEN - 1] = '\0';

			size += strlen(AcctName) + 1;
			ptr += strlen(AcctName) + 1;

			CopyMemory(CharName, ptr, sizeof(CharName));
			CharName[MAX_CHARNAME_LEN - 1] = '\0';

			size += strlen(CharName) + 1;
			ptr += strlen(CharName) + 1;

			CopyMemory(IPName, ptr, sizeof(IPName));
			IPName[MAX_REALMIPNAME_LEN - 1] = '\0';

			seqno = bn_ntohl(preq->h.seqno);

			ZeroMemory(packet, sizeof(D2GSPACKET));
			preply = (t_d2gs_d2cs_creategamereply*)(packet->data);


			dwGameFlag = 0x00000004;
			if (preq->expansion) dwGameFlag |= 0x00100000;
			if (preq->ladder)    dwGameFlag |= 0x00200000;
			if (preq->hardcore)  dwGameFlag |= 0x00000800;
			if (preq->difficulty>2) preq->difficulty = 0;
			dwGameFlag |= ((preq->difficulty) << 0x0c);
			if (D2GSIsActive()) {
				if (D2GSGetCurrentGameNumber()>=(int)(d2gsconf.gsmaxgames)) {
					D2GSEventLog("D2CSCreateEmptyGame", "Reach max game number");
					preply->result = bn_htonl(D2GS_D2CS_CREATEGAME_FAILED);
					preply->gameid = bn_htonl(0);
				}
				else if (D2GSNewEmptyGame(GameName, GamePass, GameDesc, dwGameFlag, 0x11, 0x22, 0x33, (LPWORD)&wGameId)) {
					preply->result = bn_htonl(D2GS_D2CS_CREATEGAME_SUCCEED);

					/* add the game info into the game queue */
					D2GSGameListInsert(GameName, GamePass, GameDesc, AcctName, CharName, IPName, (UCHAR)(preq->expansion),
						(UCHAR)(preq->difficulty), (UCHAR)(preq->hardcore), (UCHAR)(preq->ladder), (WORD)wGameId);

					D2GSSetDifficultyCount(preq->difficulty, 1, 0);

					D2GSEventLog("D2CSCreateEmptyGame",
						"Created game '%s', %u,%s,%s,%s,%s, seqno=%lu",
						GameName,
						wGameId,
						preq->expansion ? "expansion" : "classic",
						desc_game_difficulty[preq->difficulty % 3],
						preq->hardcore ? "hardcore" : "softcore",
						preq->ladder ? "ladder" : "non-ladder",
						seqno);

					D2GSEventLog("D2CSCreateEmptyGame",
						"GameInfo: '%s','%s','%s', By %s(*%s)@%s", GameName, GamePass, GameDesc, CharName, AcctName, IPName);
				} else {
					D2GSEventLog("D2CSCreateEmptyGame", "Failed creating game '%s'", GameName);
					preply->result = bn_htonl(D2GS_D2CS_CREATEGAME_FAILED);
				}
			} else {
				D2GSEventLog("D2CSCreateEmptyGame", "Game Server is not Authorized.");
				preply->result = bn_htonl(D2GS_D2CS_CREATEGAME_FAILED);
			}

			preply->h.type  = bn_htons(D2GS_D2CS_CREATEGAMEREPLY);
			preply->h.size  = bn_htons(sizeof(t_d2gs_d2cs_creategamereply));
			preply->h.seqno = bn_htonl(seqno);
			packet->peer     = PACKET_PEER_SEND_TO_D2CS;
			packet->datalen  = sizeof(t_d2gs_d2cs_creategamereply);
			preply->gameid = bn_htonl(wGameId);
			D2GSNetSendPacket(packet);
			free(packet);

			return;

		} /* End of D2CSCreateEmptyGame() */


		/*********************************************************************
		 * Purpose: to deal with client join game request
		 * Return: None
		 *********************************************************************/
		void D2CSClientJoinGameRequest(int peer, LPVOID lpdata)
		{
			char		CharName[MAX_CHARNAME_LEN] = { 0 };
			char		AcctName[MAX_ACCTNAME_LEN] = { 0 };
			char		ClientIPName[MAX_REALMIPNAME_LEN] = { 0 };
			UCHAR		*ptr;
			DWORD		dwGameId;
			DWORD		dwToken;
			D2GAMEINFO	*lpGame;
			D2CHARINFO	*lpChar;
			D2GSPACKET* packet = static_cast<D2GSPACKET *>(malloc(sizeof(D2GSPACKET)));
			t_d2cs_d2gs_joingamereq		*preq;
			t_d2gs_d2cs_joingamereply	*preply;
			DWORD		result;

			if (!lpdata) return;

			/* get out parameter */
			preq = (t_d2cs_d2gs_joingamereq *)(lpdata);
			dwGameId = bn_ntohl(preq->gameid);
			dwToken = bn_ntohl(preq->token);
			ptr = (UCHAR *)(preq+1);
			CopyMemory(CharName, ptr, strlen((char*)ptr));
			CharName[MAX_CHARNAME_LEN-1] = '\0';
			ptr += (strlen(CharName)+1);
			CopyMemory(AcctName, ptr, strlen((char*)ptr));
			AcctName[MAX_ACCTNAME_LEN-1] = '\0';
			ptr += (strlen(AcctName) + 1);
			CopyMemory(ClientIPName, ptr, strlen((char*)ptr));
			ClientIPName[MAX_REALMIPNAME_LEN - 1] = '\0';

			/* reset reply packet */
			ZeroMemory(packet, sizeof(D2GSPACKET));
			preply = (t_d2gs_d2cs_joingamereply *)(packet->data);
			result = D2GS_D2CS_JOINGAME_SUCCEED;	/* it is 0 */

			EnterCriticalSection(&csGameList);

			/* find the game by gameid */
			if (D2GSIsActive()) {
				lpGame = D2GSFindGameInfoByGameId(dwGameId);
				if (lpGame) {
					/* try to find the user */
					lpChar = D2GSFindPendingCharByCharName(CharName);
					if (lpChar) {
						/* user found, delete it from the pending list */
						D2GSDeletePendingChar(lpChar);
					}
					if (lpGame->disable) {
						result = D2GS_D2CS_JOINGAME_FAILED;
						D2GSEventLog("D2CSClientJoinGameRequest",
							"%s(*%s) failed joining a disabled game '%s'", CharName, AcctName, lpGame->GameName);
					} else if ( (time(NULL)-(lpGame->CreateTime)) > d2gsconf.maxgamelife ) {
						result = D2GS_D2CS_JOINGAME_FAILED;
						D2GSEventLog("D2CSClientJoinGameRequest",
							"%s(*%s) failed joining an auld game '%s'", CharName, AcctName, lpGame->GameName);
					} else if (lpGame->CharCount>=8) {
						result = D2GS_D2CS_JOINGAME_FAILED;
						D2GSEventLog("D2CSClientJoinGameRequest",
							"%s(*%s) failed joining a CONGEST game '%s'", CharName, AcctName, lpGame->GameName);
					}
				} else {
					/* the game not found, error */ 
					result = D2GS_D2CS_JOINGAME_FAILED;
					D2GSEventLog("D2CSClientJoinGameRequest",
						"%s(*%s) join game %u not exist", CharName, AcctName, dwGameId);
				}
			} else {
				result = D2GS_D2CS_JOINGAME_FAILED;
				D2GSEventLog("D2CSClientJoinGameRequest", "Game Server is not Authorized.");
			}

			/* if game found, insert the char into pending list */
			if (!result) {
				if (D2GSInsertCharIntoPendingList(dwToken, AcctName, CharName, ClientIPName, 0, 0xffff, lpGame)) {
					result = D2GS_D2CS_JOINGAME_FAILED;
					D2GSEventLog("D2CSClientJoinGameRequest",
						"%s(*%s) failed insert into pending list, game %s(%u)", 
						CharName, AcctName, lpGame->GameName, dwGameId);
				} else {
					D2GSEventLog("D2CSClientJoinGameRequest",
						"%s(*%s) join game '%s', id=%u(%s,%s,%s,%s)",
						CharName, AcctName, lpGame->GameName, dwGameId,
						lpGame->expansion ? "exp" : "classic",
						desc_game_difficulty[lpGame->difficulty % 3],
						lpGame->hardcore ? "hardcore" : "softcore",
						lpGame->ladder ? "ladder" : "non-ladder");
					result = D2GS_D2CS_JOINGAME_SUCCEED;
				}
			}

			LeaveCriticalSection(&csGameList);

			preply->result  = bn_htonl(result);
			preply->gameid  = bn_htonl(dwGameId);
			preply->h.type  = bn_htons(D2GS_D2CS_JOINGAMEREPLY);
			preply->h.size  = bn_htons(sizeof(t_d2gs_d2cs_joingamereply));
			preply->h.seqno = preq->h.seqno;
			packet->peer    = PACKET_PEER_SEND_TO_D2CS;
			packet->datalen = sizeof(t_d2gs_d2cs_joingamereply);

			D2GSNetSendPacket(packet);
			free(packet);

			return;

		} /* End of D2CSClientJoinGameRequest() */


		/*=====================================================================================*/
		/* the following function called in the callback event, by gaem engine */


		/*********************************************************************
		 * Purpose: FindPlayerToken
		 * Return: TRUE of FALSE
		 *********************************************************************/
		BOOL D2GSCBFindPlayerToken(LPCSTR lpCharName, DWORD dwToken, WORD wGameId,
						LPSTR lpAccountName, LPPLAYERDATA lpPlayerData)
		{
			D2CHARINFO		*lpChar;
			D2GAMEINFO		*lpGame;
			char			RealmIPName[MAX_REALMIPNAME_LEN] = { 0 };
			int				val;

			if (!lpCharName || !lpAccountName || !lpPlayerData) return FALSE;

			EnterCriticalSection(&csGameList);
			lpChar = D2GSFindPendingCharByCharName(lpCharName);
			if (!lpChar) {
				LeaveCriticalSection(&csGameList);
				return FALSE;	/* not found */
			}

			/* check the token */
			if (lpChar->token != dwToken) {
				/* token doesn't matched */
				D2GSEventLog("D2GSCBFindPlayerToken", "Bad Token for %s(*%s)",
					lpCharName, lpChar->AcctName);
				D2GSDeletePendingChar(lpChar);
				LeaveCriticalSection(&csGameList);
				return FALSE;
			}

			/* find if the game exist, by gameid */
			lpGame = D2GSFindGameInfoByGameId(wGameId);
			if (!lpGame) {
				/* the specified game not found */
				D2GSEventLog("D2GSCBFindPlayerToken", "Bad GameId(%u) for char %s(*%s)",
						wGameId, lpCharName, lpChar->AcctName);
				D2GSDeletePendingChar(lpChar);
				LeaveCriticalSection(&csGameList);
				return FALSE;
			}

			/* Game found, check if it is the origin Game request in JoinGameRequest */
			if (lpGame!=(lpChar->lpGameInfo)) {
				/* game not match (may occur???) */
				D2GSEventLog("D2GSCBFindPlayerToken",
						"Game 0x%lx not match 0x%lx for char %s(*%s)",
						lpGame, lpChar->lpGameInfo, lpCharName, lpChar->AcctName);
				D2GSDeletePendingChar(lpChar);
				LeaveCriticalSection(&csGameList);
				return FALSE;
			}

			/* set some value to be return  */
			strncpy(lpAccountName, lpChar->AcctName, MAX_ACCTNAME_LEN-1);
			lpAccountName[MAX_ACCTNAME_LEN - 1] = '\0';
			strncpy(RealmIPName, lpChar->RealmIPName, MAX_REALMIPNAME_LEN - 1);
			RealmIPName[MAX_REALMIPNAME_LEN - 1] = 0;
			*lpPlayerData = (PLAYERDATA)0x01;

			/* delete the char from the peding list, and insert the char into game info */
			/*
			 * In 1.09c, if the callback function GetDatabaseCharacter() can NOT provide
			 * character save data, the GE will never call LeaveGame() function.
			 * So if this happend, should delete the char from the list in GetDatabaseCharacter()
			 */
			D2GSDeletePendingChar(lpChar);
			if ((val=D2GSInsertCharIntoGameInfo(lpGame, dwToken,
					lpAccountName, lpCharName, RealmIPName, 0, 0, FALSE))!=0)
			{
				D2GSEventLog("D2GSCBFindPlayerToken",
					"failed insert into char list for %s(*%s) to game '%s'(%u), code: %d",
					lpCharName, lpAccountName, lpGame->GameName, lpGame->GameId, val);
				LeaveCriticalSection(&csGameList);
				return FALSE;
			}

			D2GSSetDifficultyCount(lpGame->difficulty, 0, 1);

			/* it is ok now */
			D2GSEventLog("D2GSCBFindPlayerToken",
				"Found token of %s(*%s) for game '%s'(%u)",
				lpCharName, lpAccountName, lpGame->GameName, lpGame->GameId);
			LeaveCriticalSection(&csGameList);
			return TRUE;

		} /* End of D2GSCBFindPlayerToken() */


		/*********************************************************************
		 * Purpose: EnterGame
		 * Return:  None
		 *********************************************************************/
		void D2GSCBEnterGame(WORD wGameId, LPCSTR lpCharName, WORD wCharClass,
						DWORD dwCharLevel, DWORD dwReserved)
		{
			D2GAMEINFO		*lpGame;
			D2CHARINFO		*lpChar;
			char			AcctName[MAX_ACCTNAME_LEN];
			char			*pstrRealmName = "unknown";
			D2GSPACKET		packet;
			t_d2gs_d2cs_updategameinfo	*pUpdateInfo;
			BOOL			entergame;

			if (!lpCharName) return;

			/* delete the char info in the pending list */
			ZeroMemory(AcctName, sizeof(AcctName));

			entergame = FALSE;
			EnterCriticalSection(&csGameList);
			lpChar = D2GSFindPendingCharByCharName(lpCharName);
			if (lpChar) {
				/* found the char in the pending list, delete it */
				D2GSDeletePendingChar(lpChar);
			}

			/* add to game info */
			lpChar = NULL;
			lpGame = D2GSFindGameInfoByGameId(wGameId);
			if (lpGame) {
				lpChar = D2GSFindCharInGameByCharName(lpGame, lpCharName);
				if (lpChar) {
					/* found, update the into */
					CopyMemory(AcctName, lpChar->AcctName, sizeof(AcctName));
					lpChar->CharLevel = dwCharLevel;
					lpChar->CharClass = wCharClass;
					lpChar->EnterGame = TRUE;
					lpChar->EnterTime = time(NULL);
				} else {
					/* no such char info in the game, insert one */
					D2GSInsertCharIntoGameInfo(lpGame, 0xffffffff,
						AcctName, lpCharName, pstrRealmName, dwCharLevel,wCharClass, TRUE);
					D2GSEventLog("D2GSCBEnterGame",
						"char %s not in game '%s'(%u), reinsert",
						lpCharName, lpGame->GameName, wGameId);
				}
				entergame = TRUE;
				D2GSEventLog("D2GSCBEnterGame",
					"%s(*%s)[L=%lu,C=%s]@%s enter game '%s', id=%u(%s,%s,%s,%s)",
					lpCharName,
					AcctName,
					dwCharLevel,
					desc_char_class[wCharClass % 7],
					pstrRealmName,
					lpGame->GameName,
					wGameId,
					lpGame->expansion ? "exp" : "classic",
					desc_game_difficulty[lpGame->difficulty % 3],
					lpGame->hardcore ? "hardcore" : "softcore",
					lpGame->ladder ? "ladder" : "non-ladder");

			} else {
				/* if reach here, sth wrong may had happened!!! */
				D2GSEventLog("D2GSCBEnterGame",
					"%s enter a phantom game, id %u", lpCharName, wGameId);
			}

			/* send motd */
			if (lpChar && (strlen(d2gsconf.motd)!=0)) {
				D2GSMOTDAdd(lpChar->ClientId);
			}

			if (lpChar && d2gsconf.maxpreferusers != 0)
			{
				UpdateMaxPreferUsers();
			}

			LeaveCriticalSection(&csGameList);

			/* send update info to D2CS */
			if (entergame) {
				ZeroMemory(&packet, sizeof(packet));
				pUpdateInfo = (t_d2gs_d2cs_updategameinfo *)(packet.data);
				pUpdateInfo->h.type  = bn_htons(D2GS_D2CS_UPDATEGAMEINFO);
				pUpdateInfo->h.size  = bn_htons(sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1);
				pUpdateInfo->h.seqno = bn_htonl(D2GSGetSequence());
				pUpdateInfo->flag    = bn_htonl(D2GS_D2CS_UPDATEGAMEINFO_FLAG_ENTER);
				pUpdateInfo->gameid  = bn_htonl(wGameId);
				pUpdateInfo->charlevel = bn_htonl(dwCharLevel);
				pUpdateInfo->charclass = bn_htons(wCharClass);
				strcpy((char*)(packet.data)+sizeof(t_d2gs_d2cs_updategameinfo), lpCharName);
				packet.datalen = sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1;
				packet.peer    = PACKET_PEER_SEND_TO_D2CS;
				D2GSNetSendPacket(&packet);
			}

			/* ok */
			return;

		} /* End of D2GSCBEnterGame() */


		void UpdateMaxPreferUsers(void)
		{
			DWORD gameNum = 0;
			DWORD userNum = 0;

			D2GSEventLog("UpdateMaxPreferUsers",
				"d2gsconf.maxpreferusers=%d", d2gsconf.maxpreferusers);
			if (d2gsconf.maxpreferusers == 0)
			{
				return;
			}

			D2GSGetCurrentGameStatistic(&gameNum, &userNum);

			D2GSEventLog("UpdateMaxPreferUsers",
				"gameNum=%d, userNum=%d", gameNum, userNum);

			if (userNum >= d2gsconf.maxpreferusers)
			{
				D2GSEventLog("UpdateMaxPreferUsers",
					"userNum >= d2gsconf.maxpreferusers");
				if (d2gsconf.curgsmaxgames != 0)
				{
					D2GSEventLog("UpdateMaxPreferUsers",
						"D2GSSetD2CSMaxGameNumber 0");
					D2GSSetD2CSMaxGameNumber(0);
				}
				return;
			}

			if (d2gsconf.maxpreferusers < 0x0A)
			{
				if (d2gsconf.curgsmaxgames != d2gsconf.gsmaxgames)
				{
					D2GSSetD2CSMaxGameNumber(d2gsconf.gsmaxgames);
				}
			}
			else if (userNum < (d2gsconf.maxpreferusers - 8))
			{
				if (d2gsconf.curgsmaxgames != d2gsconf.gsmaxgames)
				{
					D2GSSetD2CSMaxGameNumber(d2gsconf.gsmaxgames);
				}
			}
		}


		/*********************************************************************
		 * Purpose: LeaveGame
		 * Return:  None
		 *********************************************************************/
		void D2GSCBLeaveGame(LPGAMEDATA lpGameData, WORD wGameId, WORD wCharClass,
						DWORD dwCharLevel, DWORD dwExpLow, DWORD dwExpHigh,
						WORD wCharStatus, LPCSTR lpCharName, LPCSTR lpCharPortrait,
						BOOL bUnlock, DWORD dwZero1, DWORD dwZero2,
						LPCSTR lpAccountName, PLAYERDATA PlayerData,
						PLAYERMARK PlayerMark)
		{
			D2GAMEINFO		*lpGame;
			D2CHARINFO		*lpChar;
			D2GSPACKET		packet;
			t_d2gs_d2cs_updategameinfo	*pUpdateInfo;
			DWORD			dwEnterGame;
			DWORD			CharLockStatus;

			EnterCriticalSection(&csGameList);

			/* find the game first */
			dwEnterGame = CharLockStatus = FALSE;
			lpChar = NULL;
			lpGame = D2GSFindGameInfoByGameId(wGameId);
			if (lpGame) {
				/* find the char in the game */
				lpChar = D2GSFindCharInGameByCharName(lpGame, lpCharName);
				if (lpChar) {
					dwEnterGame = lpChar->EnterGame;
					CharLockStatus = lpChar->CharLockStatus;
					D2GSEventLog("D2GSCBLeaveGame",
						"%s(*%s)[L=%lu,C=%s] leave game '%s', id=%u(%s,%s,%s,%s)",
						lpCharName, lpAccountName, dwCharLevel,
						desc_char_class[wCharClass % 7],
						lpGame->GameName,
						wGameId,
						lpGame->expansion ? "exp" : "classic",
						desc_game_difficulty[lpGame->difficulty % 3],
						lpGame->hardcore ? "hardcore" : "softcore",
						lpGame->ladder ? "ladder" : "non-ladder");
					D2GSSetDifficultyCount(lpGame->difficulty, 0, -1);
				} else {
					/* if reach here, sth wrong may had happened!!! */
					D2GSEventLog("D2GSCBLeaveGame",
						"phantom user %s(*%s) in game '%s'(%u)",
						lpCharName, lpAccountName, lpGame->GameName, wGameId);
				}
			} else {
				/* if reach here, sth wrong may have happened!!! */
				D2GSEventLog("D2GSCBLeaveGame",
					"%s(*%s) leave a phantom game, id %u",
					lpCharName, lpAccountName, wGameId);
			}

			/* write charinfo file */
			if (bUnlock && dwEnterGame) {
				D2GSWriteCharInfoFile(lpAccountName, lpCharName, wCharClass,
					dwCharLevel, dwExpLow, wCharStatus, lpCharPortrait);
			}

			/* delete the char from the game */
			if (lpGame && lpChar)
			{
				D2GSDeleteCharFromGameInfo(lpGame, lpChar);
				if (d2gsconf.maxpreferusers != 0)
				{
					UpdateMaxPreferUsers();
				}
			}

			LeaveCriticalSection(&csGameList);

			/* unlock the char in dbserver */
			if (CharLockStatus)
				D2GSSetCharLockStatus(lpAccountName, lpCharName, d2gsparam.realmname, FALSE);

			/* send update info to D2CS */
			if (dwEnterGame) {
				ZeroMemory(&packet, sizeof(packet));
				pUpdateInfo = (t_d2gs_d2cs_updategameinfo *)(packet.data);
				pUpdateInfo->h.type  = bn_htons(D2GS_D2CS_UPDATEGAMEINFO);
				pUpdateInfo->h.size  = bn_htons(sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1);
				pUpdateInfo->h.seqno = bn_htonl(D2GSGetSequence());
				pUpdateInfo->flag    = bn_htonl(D2GS_D2CS_UPDATEGAMEINFO_FLAG_LEAVE);
				pUpdateInfo->gameid  = bn_htonl(wGameId);
				pUpdateInfo->charlevel = bn_htonl(dwCharLevel);
				pUpdateInfo->charclass = bn_htons(wCharClass);
				strcpy((char*)(packet.data)+sizeof(t_d2gs_d2cs_updategameinfo), lpCharName);
				packet.datalen = sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1;
				packet.peer    = PACKET_PEER_SEND_TO_D2CS;
				D2GSNetSendPacket(&packet);
			}


			return;

		} /* End of D2GSCBLeaveGame() */


		/*********************************************************************
		 * Purpose: CloseGame
		 * Return:  None
		 *********************************************************************/
		void D2GSCBCloseGame(WORD wGameId)
		{
			D2GAMEINFO		*lpGame;
			D2GSPACKET		packet;
			t_d2gs_d2cs_closegame	*pclosegame;

			EnterCriticalSection(&csGameList);
			lpGame = D2GSFindGameInfoByGameId(wGameId);
			if (lpGame) {
				/* delete it */
				D2GSEventLog("D2GSCBCloseGame",
					"Close game '%s', id=%u(%s,%s,%s,%s)",
					lpGame->GameName,
					wGameId,
					lpGame->expansion ? "exp" : "classic",
					desc_game_difficulty[lpGame->difficulty % 3],
					lpGame->hardcore ? "hardcore" : "softcore",
					lpGame->ladder ? "ladder" : "non-ladder"
				);
				D2GSSetDifficultyCount(lpGame->difficulty, -1, 0);
				D2GSGameListDelete(lpGame);
			} else {
				/* if reach here, sth wrong may had happened!!! */
				D2GSEventLog("D2GSCBCloseGame",	"Close phantom game, id %u", wGameId);
			}

			LeaveCriticalSection(&csGameList);

			/* send notification to D2CS */
			ZeroMemory(&packet, sizeof(packet));
			pclosegame = (t_d2gs_d2cs_closegame *)(packet.data);
			pclosegame->h.type  = bn_htons(D2GS_D2CS_CLOSEGAME);
			pclosegame->h.size  = bn_htons(sizeof(t_d2gs_d2cs_closegame));
			pclosegame->h.seqno = bn_htonl(D2GSGetSequence());
			pclosegame->gameid  = bn_htonl((DWORD)wGameId);
			packet.datalen = sizeof(t_d2gs_d2cs_closegame);
			packet.peer    = PACKET_PEER_SEND_TO_D2CS;
			D2GSNetSendPacket(&packet);

			return;

		} /* End of D2GSCBCloseGame() */


		/*********************************************************************
		 * Purpose: UpdateGameInformation
		 * Return:  None
		 *********************************************************************/
		void D2GSCBUpdateGameInformation(WORD wGameId, LPCSTR lpCharName, 
						WORD wCharClass, DWORD dwCharLevel)
		{
			D2GAMEINFO		*lpGame;
			D2CHARINFO		*lpChar;
			D2GSPACKET		packet;
			t_d2gs_d2cs_updategameinfo	*pUpdateInfo;

			EnterCriticalSection(&csGameList);
			lpGame = D2GSFindGameInfoByGameId(wGameId);
			if (lpGame) {
				lpChar = D2GSFindCharInGameByCharName(lpGame, lpCharName);
				if (lpChar) {
					lpChar->CharClass = wCharClass;
					lpChar->CharLevel = dwCharLevel;
				}
			}

			LeaveCriticalSection(&csGameList);

			ZeroMemory(&packet, sizeof(packet));
			pUpdateInfo = (t_d2gs_d2cs_updategameinfo *)(packet.data);
			pUpdateInfo->h.type  = bn_htons(D2GS_D2CS_UPDATEGAMEINFO);
			pUpdateInfo->h.size  = bn_htons(sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1);
			pUpdateInfo->h.seqno = bn_htonl(D2GSGetSequence());
			pUpdateInfo->flag    = bn_htonl(D2GS_D2CS_UPDATEGAMEINFO_FLAG_UPDATE);
			pUpdateInfo->gameid  = bn_htonl(wGameId);
			pUpdateInfo->charlevel = bn_htonl(dwCharLevel);
			pUpdateInfo->charclass = bn_htons(wCharClass);
			strcpy((char*)(packet.data)+sizeof(t_d2gs_d2cs_updategameinfo), lpCharName);
			packet.datalen = sizeof(t_d2gs_d2cs_updategameinfo)+strlen(lpCharName)+1;
			packet.peer    = PACKET_PEER_SEND_TO_D2CS;
			D2GSNetSendPacket(&packet);

			D2GSEventLog("D2GSCBUpdateGameInformation",
				"Update game info for char '%s'(L=%lu,%s), GameId %d",
				lpCharName, dwCharLevel, desc_char_class[wCharClass%7], wGameId);

			return;

		} /* End of D2GSCBUpdateGameInformation() */


		/*********************************************************************
		 * Purpose: GetDatabaseCharacter
		 * Return:  None
		 *********************************************************************/
		void D2GSCBGetDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
							DWORD dwClientId, LPCSTR lpAccountName)
		{
			D2GSPACKET						packet;
			t_d2gs_d2dbs_get_data_request	*preq;
			u_char							*ptr;
			u_short							size;
			DWORD							seqno;
			D2GAMEINFO						*lpGameInfo;
			D2CHARINFO						*lpCharInfo;

			EnterCriticalSection(&csGameList);

			/* insert request info list */
			seqno = D2GSGetSequence();
			if (D2GSInsertGetDataRequest(lpAccountName, lpCharName, dwClientId, seqno)) {
				D2GSEventLog("D2GSCBGetDatabaseCharacter",
					"Failed insert get data request for %s(*%s)", lpCharName, lpAccountName);
				D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
				LeaveCriticalSection(&csGameList);
				return;
			}
			lpGameInfo = (D2GAMEINFO*)charlist_getdata(lpCharName, CHARLIST_GET_GAMEINFO);
			lpCharInfo = (D2CHARINFO*)charlist_getdata(lpCharName, CHARLIST_GET_CHARINFO);
			if (lpCharInfo && lpGameInfo &&
						!IsBadReadPtr(lpCharInfo, sizeof(D2CHARINFO)) &&
						!IsBadReadPtr(lpGameInfo, sizeof(D2GAMEINFO)) &&
						(lpCharInfo->lpGameInfo == lpGameInfo) &&
						(lpCharInfo->GameId == lpGameInfo->GameId)) {
				lpCharInfo->ClientId = dwClientId;
			} else {
				D2GSEventLog("D2GSCBGetDatabaseCharacter",
					"Call back to get save for %s(*%s), but the char or the game is invalid",
					lpCharName, lpAccountName);
				D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
				LeaveCriticalSection(&csGameList);
				return;
			}

			LeaveCriticalSection(&csGameList);

			/* send get data request to D2DBS */
			preq = (t_d2gs_d2dbs_get_data_request*)(packet.data);
			ZeroMemory(&packet, sizeof(packet));
			size = sizeof(t_d2gs_d2dbs_get_data_request);
			ptr = packet.data + size;
			strcpy((char*)ptr, lpAccountName);
			//str2lower(ptr);
			size += (strlen(lpAccountName)+1);
			ptr  += (strlen(lpAccountName)+1);
			strcpy((char*)ptr, lpCharName);
			//str2lower(ptr);
			size += (strlen(lpCharName)+1);
			ptr  += (strlen(lpCharName)+1);
			strcpy((char*)ptr, d2gsparam.realmname);
			//str2lower(ptr);
			size += (strlen(d2gsparam.realmname)+1);
			ptr  += (strlen(d2gsparam.realmname)+1);
			preq->datatype = bn_htons(D2GS_DATA_CHARSAVE);
			preq->h.type   = bn_htons(D2GS_D2DBS_GET_DATA_REQUEST);
			preq->h.size   = bn_htons(size);
			preq->h.seqno  = bn_htonl(seqno);
			packet.datalen = size;
			packet.peer    = PACKET_PEER_SEND_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSCBGetDatabaseCharacter",
				"Send GetDataRequest to D2DBS for %s(*%s)", lpCharName, lpAccountName);

			return;

		} /* End of D2GSCBGetDatabaseCharacter() */


		/*********************************************************************
		 * Purpose: SaveDatabaseCharacter
		 * Return:  None
		 *********************************************************************/
		void D2GSCBSaveDatabaseCharacter(LPGAMEDATA lpGameData, LPCSTR lpCharName,
						LPCSTR lpAccountName, LPVOID lpSaveData,
						DWORD dwSize, PLAYERDATA PlayerData)
		{
			D2GSPACKET						packet;
			t_d2gs_d2dbs_save_data_request	*preq;
			u_short							size;
			u_char							*ptr, *pdata;

			preq = (t_d2gs_d2dbs_save_data_request *)(packet.data);
			pdata = (u_char*)lpSaveData;

			ZeroMemory(&packet, sizeof(packet));
			size = sizeof(t_d2gs_d2dbs_save_data_request);
			ptr = packet.data + size;
			strcpy((char*)ptr, lpAccountName);
			//str2lower(ptr);
			size += (strlen(lpAccountName)+1);
			ptr  += (strlen(lpAccountName)+1);
			strcpy((char*)ptr, lpCharName);
			//str2lower(ptr);
			size += (strlen(lpCharName)+1);
			ptr  += (strlen(lpCharName)+1);
			strcpy((char*)ptr, d2gsparam.realmname);
			//str2lower(ptr);
			size += (strlen(d2gsparam.realmname)+1);
			ptr  += (strlen(d2gsparam.realmname)+1);
			CopyMemory(ptr, pdata+sizeof(short), dwSize-sizeof(short));
			size += (u_short)(dwSize-sizeof(short));
			preq->datatype = bn_htons(D2GS_DATA_CHARSAVE);
			preq->datalen  = bn_htons((u_short)(dwSize-sizeof(short)));
			preq->h.type   = bn_htons(D2GS_D2DBS_SAVE_DATA_REQUEST);
			preq->h.size   = bn_htons(size);
			preq->h.seqno  = bn_htonl(D2GSGetSequence());
			packet.datalen = size;
			packet.peer    = PACKET_PEER_SEND_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSCBSaveDatabaseCharacter", "Save CHARSAVE for %s(*%s)",
					lpCharName, lpAccountName);

			return;

		} /* End of D2GSCBSaveDatabaseCharacter() */


		/*********************************************************************
		 * Purpose: D2GSWriteCharInfoFile
		 * Return:  None
		 *********************************************************************/
		void D2GSWriteCharInfoFile(LPCSTR lpAccountName, LPCSTR lpCharName,
							WORD wCharClass, DWORD dwCharLevel, DWORD dwExpLow,
							WORD wCharStatus, LPCSTR lpCharPortrait)
		{
			D2CHARINFO				*lpCharInfo;
			t_d2charinfo_file		d2charinfo;
			t_d2charinfo_header		*lpheader;
			t_d2charinfo_portrait	*lpportrait;
			t_d2charinfo_summary	*lpsummary;
			DWORD					create_time;
			D2GSPACKET						packet;
			t_d2gs_d2dbs_save_data_request	*preq;
			u_short							size;
			u_char							*ptr, *pdata;

			lpCharInfo = (D2CHARINFO*)charlist_getdata(lpCharName, CHARLIST_GET_CHARINFO);
			if (!lpCharInfo) {
				D2GSEventLog("D2GSWriteCharInfoFile", "%s(*%s) not found in charlist",
					lpCharName, lpAccountName);
				return;
			}
			create_time = lpCharInfo->CharCreateTime;

			lpheader   = &(d2charinfo.header);
			lpportrait = &(d2charinfo.portrait);
			lpsummary  = &(d2charinfo.summary);
			ZeroMemory(&d2charinfo, sizeof(t_d2charinfo_file));

			lpheader->magicword = D2CHARINFO_MAGICWORD;
			lpheader->version   = D2CHARINFO_VERSION;
			/*create_time = time(NULL);*/
			lpheader->create_time = create_time;
			lpheader->last_time = time(NULL);
			lpheader->total_play_time += (lpheader->last_time - lpCharInfo->EnterTime);
			strncpy(lpheader->account, lpAccountName, sizeof(lpheader->account)-1);
			lpheader->account[sizeof(lpheader->account) - 1] = 0;
			strncpy(lpheader->charname, lpCharName, sizeof(lpheader->charname)-1);
			lpheader->charname[sizeof(lpheader->charname) - 1] = 0;
			/*str2lower(lpheader->account);
			str2lower(lpheader->charname);*/
			strcpy(lpheader->realmname, d2gsparam.realmname);
	
			lpsummary->experience = dwExpLow;
			lpsummary->charlevel  = dwCharLevel;
			lpsummary->charclass  = (DWORD)wCharClass;
			lpsummary->charstatus = (DWORD)wCharStatus;

			if ( sizeof(d2charinfo.portrait) < (strlen(lpCharPortrait)+1) )
			{
				CopyMemory(lpportrait, lpCharPortrait, sizeof(d2charinfo.portrait));
				D2GSEventLog("D2GSWriteCharInfoFile",
					"Portrait data too large for %s(%s) as %d",
					lpCharName, lpAccountName, strlen(lpCharPortrait)+1);
			} else {
				CopyMemory(lpportrait, lpCharPortrait, (strlen(lpCharPortrait)+1));
			}

			/* to check if the portrait if valid */
			if ( (lpportrait->level==0) || (lpportrait->level>99)
				|| ((lpportrait->chclass-1)!=(BYTE)wCharClass) )
			{
				D2GSEventLog("D2GSWriteCharInfoFile",
					"Bad Portrait data for %s(*%s)", lpCharName, lpAccountName);
				return;
			}
	

		#ifdef DEBUG
			PortraitDump(lpAccountName, lpCharName, lpCharPortrait);
		#endif

			/* save CharInfo */
			preq = (t_d2gs_d2dbs_save_data_request *)(packet.data);
			pdata = (u_char*)(&d2charinfo);

			ZeroMemory(&packet, sizeof(packet));
			size = sizeof(t_d2gs_d2dbs_save_data_request);
			ptr  = packet.data + sizeof(t_d2gs_d2dbs_save_data_request);
			strcpy((char*)ptr, lpAccountName);
			//str2lower(ptr);
			size += (strlen(lpAccountName)+1);
			ptr  += (strlen(lpAccountName)+1);
			strcpy((char*)ptr, lpCharName);
			//str2lower(ptr);
			size += (strlen(lpCharName)+1);
			ptr  += (strlen(lpCharName)+1);
			strcpy((char*)ptr, d2gsparam.realmname);
			//str2lower(ptr);
			size += (strlen(d2gsparam.realmname)+1);
			ptr  += (strlen(d2gsparam.realmname)+1);
			CopyMemory(ptr, pdata, sizeof(d2charinfo));
			size += sizeof(d2charinfo);
			preq->datatype = bn_htons(D2GS_DATA_PORTRAIT);
			preq->datalen  = bn_htons(sizeof(d2charinfo));
			preq->h.type   = bn_htons(D2GS_D2DBS_SAVE_DATA_REQUEST);
			preq->h.size   = bn_htons(size);
			preq->h.seqno  = bn_htonl(D2GSGetSequence());
			packet.datalen = size;
			packet.peer    = PACKET_PEER_SEND_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSWriteCharInfoFile",
					"Send CHARINFO data for %s(*%s)", lpCharName, lpAccountName);

			return;

		} /* End of D2GSWriteCharInfoFile() */


		/*********************************************************************
		 * Purpose: D2GSWriteCharInfoFile
		 * Return:  None
		 *********************************************************************/
		void D2GSUpdateCharacterLadder(LPCSTR lpCharName, WORD wCharClass, DWORD dwCharLevel,
							DWORD dwCharExpLow, DWORD dwCharExpHigh,  WORD wCharStatus)
		{
			D2CHARINFO					*lpCharInfo;
			D2GSPACKET					packet;
			t_d2gs_d2dbs_update_ladder	*preq;
			u_short						size;
			u_char						*ptr;

			lpCharInfo = (D2CHARINFO*)charlist_getdata(lpCharName, CHARLIST_GET_CHARINFO);
			if (!lpCharInfo) {
				D2GSEventLog("D2GSUpdateCharacterLadder", "%s not found in charlist", lpCharName);
				return;
			}

			if (!(lpCharInfo->AllowLadder)) return;

			preq = (t_d2gs_d2dbs_update_ladder *)(packet.data);
			ZeroMemory(&packet, sizeof(packet));
			size = sizeof(t_d2gs_d2dbs_update_ladder);
			ptr = packet.data + size;

			strcpy((char*)ptr, lpCharName);
			//str2lower(ptr);
			size += (strlen(lpCharName)+1);
			ptr  += (strlen(lpCharName)+1);
			strcpy((char*)ptr, d2gsparam.realmname);
			//str2lower(ptr);
			size += (strlen(d2gsparam.realmname)+1);
			ptr  += (strlen(d2gsparam.realmname)+1);

			preq->charlevel   = bn_htonl(dwCharLevel);
			preq->charexplow  = bn_htonl(dwCharExpLow);
			preq->charexphigh = bn_htonl(dwCharExpHigh);
			preq->charclass   = bn_htons(wCharClass);
			preq->charstatus  = bn_htons(wCharStatus);
			preq->h.type      = bn_htons(D2GS_D2DBS_UPDATE_LADDER);
			preq->h.size      = bn_htons(size);
			preq->h.seqno     = bn_htonl(D2GSGetSequence());
			packet.datalen    = size;
			packet.peer       = PACKET_PEER_SEND_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSUpdateCharacterLadder", "Update ladder for %s@%s",
					lpCharName, d2gsparam.realmname);

			return;

		} /* End of D2GSUpdateCharacterLadder() */


		/*********************************************************************
		 * Purpose: D2GSLoadComplete
		 * Return:  None
		 *********************************************************************/
		void D2GSLoadComplete(WORD wGameId, LPCSTR lpCharName, BOOL bExpansion)
		{
			//char	buf[256];

			/* move this code to enter game now */
			/*
			strcpy(buf, d2gsconf.motd);
			string_color(buf);
			chat_message_announce_char(CHAT_MESSAGE_TYPE_SYS_MESSAGE,
					lpCharName, buf);
			*/
			return;
		}

		/*=====================================================================================*/
		/* the following function handle packet from and to D2DBS */

		/*********************************************************************
		 * Purpose: D2DBSSaveDataReply
		 * Return:  None
		 *********************************************************************/
		void D2DBSSaveDataReply(int peer, LPVOID lpdata)
		{
			t_d2dbs_d2gs_save_data_reply	*preply;
			u_char							*lpCharName;

			preply = (t_d2dbs_d2gs_save_data_reply*)lpdata;
			lpCharName = (u_char*)lpdata + sizeof(t_d2dbs_d2gs_save_data_reply);
			D2GSEventLog("D2DBSSaveDataReply",
				"Save %s data to D2DBS for %s %s",
				(preply->datatype)==D2GS_DATA_CHARSAVE ? "<CHARSAVE>" : "<CHARINFO>",
				lpCharName, preply->result ? "failed" : "success");
			return;
		} /* End of D2DBSSaveDataReply() */


		/*********************************************************************
		 * Purpose: D2DBSGetDataReply
		 * Return:  None
		 *********************************************************************/
		void D2DBSGetDataReply(int peer, LPVOID lpdata)
		{
			t_d2dbs_d2gs_get_data_reply		*preply;
			DWORD							seqno;
			D2GETDATAREQUEST				*lpGetDataReq;
			char							AcctName[MAX_ACCTNAME_LEN];
			char							CharName[MAX_CHARNAME_LEN];
			u_char							*pSaveData;
			DWORD							size;
			DWORD							dwClientId;
			PLAYERINFO						PlayerInfo;
			D2GAMEINFO						*lpGameInfo;
			D2CHARINFO						*lpCharInfo;

			preply = (t_d2dbs_d2gs_get_data_reply*)lpdata;
			switch(bn_ntohs(preply->datatype))
			{
			case D2GS_DATA_CHARSAVE:
				pSaveData = (u_char*)lpdata + sizeof(t_d2dbs_d2gs_get_data_reply);
				strncpy(CharName, (char*)pSaveData, MAX_CHARNAME_LEN-1);
				CharName[MAX_CHARNAME_LEN-1] = '\0';
				snprintf(PlayerInfo.CharName, sizeof(PlayerInfo.CharName), "%s", CharName);
				pSaveData += strlen(CharName)+1;
				size = (DWORD)(bn_ntohs(preply->datalen));
				PlayerInfo.PlayerMark = 0;
				PlayerInfo.dwReserved = 0;
				/* find get data request in the list */
				seqno = bn_ntohl(preply->h.seqno);
				EnterCriticalSection(&csGameList);
				lpGetDataReq = D2GSFindGetDataRequestBySeqno(seqno);
				if (!lpGetDataReq) {
					D2GSEventLog("D2DBSGetDataReply", "%s(*) not found in DataRequest list", CharName);
					LeaveCriticalSection(&csGameList);
					/* set the char to unlock status */
					if (bn_ntohl(preply->result) == D2DBS_GET_DATA_SUCCESS)
						D2GSSetCharLockStatus("#", CharName, d2gsparam.realmname, FALSE);
					return;	/* not found, just go away */
				}
				/* found, get save data */
				dwClientId = lpGetDataReq->ClientId;
				strncpy(AcctName, lpGetDataReq->AcctName, MAX_ACCTNAME_LEN-1);
				AcctName[MAX_ACCTNAME_LEN-1] = '\0';
				snprintf(PlayerInfo.AcctName, sizeof(PlayerInfo.AcctName), "%s", AcctName);
				D2GSDeleteGetDataRequest(lpGetDataReq);
				LeaveCriticalSection(&csGameList);
				/* send the save data to GE */
				if (bn_ntohl(preply->result) == D2DBS_GET_DATA_SUCCESS) {
					lpGameInfo = (D2GAMEINFO*)charlist_getdata(CharName, CHARLIST_GET_GAMEINFO);
					lpCharInfo = (D2CHARINFO*)charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
					if (lpCharInfo && lpGameInfo &&
								!IsBadReadPtr(lpCharInfo, sizeof(D2CHARINFO)) &&
								!IsBadReadPtr(lpGameInfo, sizeof(D2GAMEINFO)) &&
								(lpCharInfo->lpGameInfo == lpGameInfo) &&
								(lpCharInfo->GameId == lpGameInfo->GameId) &&
								(lpCharInfo->ClientId == dwClientId))
					{
						lpCharInfo->CharLockStatus = TRUE;
						lpCharInfo->AllowLadder    = bn_ntohl(preply->allowladder);
						lpCharInfo->CharCreateTime = bn_ntohl(preply->charcreatetime);
						if (D2GSSendDatabaseCharacter(dwClientId, pSaveData, size, size, FALSE, 0, &PlayerInfo, 1)) {
							D2GSEventLog("D2DBSGetDataReply",
									"send CHARSAVE to GE for %s(*%s) success, %lu bytes",
									CharName, AcctName, size);
						} else {
							D2GSSetCharLockStatus(AcctName, CharName, d2gsparam.realmname, FALSE);
							/* need to delete the char from the list in gameinfo structure */
							D2GSDeleteCharFromGameInfo(lpGameInfo, lpCharInfo);
							D2GSEventLog("D2DBSGetDataReply",
									"failed sending CHARSAVE to GE for %s(*%s), %lu bytes",
									CharName, AcctName, size);
						}
					} else {
						/* char NOT found, set the char to unlock status */
						D2GSSetCharLockStatus(AcctName, CharName, d2gsparam.realmname, FALSE);
						D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
						D2GSEventLog("D2DBSGetDataReply", "%s(*%s) not found in charlist",
								CharName, AcctName);
					}
				} else {
					D2GSSendDatabaseCharacter(dwClientId, NULL, 0, 0, TRUE, 0, NULL, 1);
					/* need to delete the char from the list in gameinfo structure */
					lpGameInfo = (D2GAMEINFO*)charlist_getdata(CharName, CHARLIST_GET_GAMEINFO);
					lpCharInfo = (D2CHARINFO*)charlist_getdata(CharName, CHARLIST_GET_CHARINFO);
					if (lpCharInfo && lpGameInfo &&
								!IsBadReadPtr(lpCharInfo, sizeof(D2CHARINFO)) &&
								!IsBadReadPtr(lpGameInfo, sizeof(D2GAMEINFO)) &&
								(lpCharInfo->lpGameInfo == lpGameInfo) &&
								(lpCharInfo->GameId == lpGameInfo->GameId))
					{
						D2GSDeleteCharFromGameInfo(lpGameInfo, lpCharInfo);
					} else {
						D2GSEventLog("D2DBSGetDataReply",
							"Failed delete char info for %s(*%s)", CharName, AcctName);
					}
					/* log this event */
					D2GSEventLog("D2DBSGetDataReply",
						"Failed get CHARSAVE data for %s(*%s)", CharName, AcctName);
				}
				break;
			case D2GS_DATA_PORTRAIT:
				break;
			}

			return;

		} /* End of D2DBSGetDataReply() */


		/*********************************************************************
		 * Purpose: D2GSSetCharLockStatus
		 * Return:  None
		 *********************************************************************/
		void D2GSSetCharLockStatus(const char* lpAccountName, const char* lpCharName,
									const char* RealmName, DWORD CharLockStatus)
		{
			D2GSPACKET				packet;
			t_d2gs_d2dbs_char_lock	*preq;
			u_short					size;
			u_char					*ptr;

			preq = (t_d2gs_d2dbs_char_lock *)(packet.data);
			ZeroMemory(&packet, sizeof(packet));
			size = sizeof(t_d2gs_d2dbs_char_lock);
			ptr = packet.data + size;

			strcpy((char*)ptr, lpAccountName);
			//str2lower(ptr);
			size += (strlen(lpAccountName)+1);
			ptr  += (strlen(lpAccountName)+1);
			strcpy((char*)ptr, lpCharName);
			//str2lower(ptr);
			size += (strlen(lpCharName)+1);
			ptr  += (strlen(lpCharName)+1);
			strcpy((char*)ptr, RealmName);
			//str2lower(ptr);
			size += (strlen(RealmName)+1);
			ptr  += (strlen(RealmName)+1);

			preq->lockstatus = bn_htonl(CharLockStatus);
			preq->h.type     = bn_htons(D2GS_D2DBS_CHAR_LOCK);
			preq->h.size     = bn_htons(size);
			preq->h.seqno    = bn_htonl(D2GSGetSequence());
			packet.datalen   = size;
			packet.peer      = PACKET_PEER_SEND_TO_D2DBS;
			D2GSNetSendPacket(&packet);
			D2GSEventLog("D2GSSetCharLockStatus",
					"Set charlock status to %s for %s(*%s)@%s",
					CharLockStatus ? "LOCKED" : "UNLOCKED", lpCharName, lpAccountName, RealmName);

			return;

		} /* End of D2GSSetCharLockStatus() */


		/*********************************************************************
		 * Purpose: D2GSUnlockChar
		 * Return:  None
		 *********************************************************************/
		void D2GSUnlockChar(LPCSTR lpAccountName, LPCSTR lpCharName)
		{
			D2GSSetCharLockStatus(lpAccountName, lpCharName, d2gsparam.realmname, FALSE);
			return;

		} /* End of D2GSUnlockChar() */

	}

}