#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <d2server.h>
#include "d2gs.h"
#include "eventlog.h"
#include "vars.h"
#include "net.h"
#include "handle_s2s.h"
#include "debug.h"
#include "d2gamelist.h"
#include "d2cs_d2gs_protocol.h"
#include "d2ge.h"
#include "server.h"


namespace pvpgn
{

	namespace d2gs
	{

		/* vars */
		static u_int	sockCS = INVALID_SOCKET;	/* socket connect to D2CS */
		static u_int	sockDBS = INVALID_SOCKET;	/* socket connect to D2DBS */
		static WSAEVENT	hRecvCS = NULL;
		static WSAEVENT	hRecvDBS = NULL;
		static HANDLE	hStopEvent = NULL;
		static BOOL		bConnectedToCS = FALSE;
		static BOOL		bConnectedToDBS = FALSE;
		static HANDLE	ghNetThread = FALSE;

		/* for net receive buffer */
		static NETRECVBUFFER	nrbCS;
		static NETRECVBUFFER	nrbDBS;
		static NETSENDBUFFER	nsbCS;
		static NETSENDBUFFER	nsbDBS;
		static CRITICAL_SECTION	csNet;

		/* for statistic */
		static D2GSNETSTATISTIC	d2netstat;


		void D2GSSetDifficultyCount(int difficulty, int R0/*=0*/, int R1/*=0xFFFFFFFF*/)
		{
			int diffIdx = difficulty % 3; // 0, 1, 2
			gD2GSInfo.dwGameDifficultyCount[diffIdx][0] += R0;
			gD2GSInfo.dwGameDifficultyCount[diffIdx][1] += R1;
		}

		void D2GSInitCountDifficulty()
		{
			ZeroMemory(gD2GSInfo.dwGameDifficultyCount, sizeof(gD2GSInfo.dwGameDifficultyCount));
		}


		/*********************************************************************
		 * Purpose: to initialize the net sub-system
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSNetInitialize(void)
		{
			WORD		wVersionRequested;
			WSADATA		wsaData;
			int			err;
			DWORD		dwThreadId;
			DWORD		tick;

			/* s2s init */
			D2GSInitializeS2S();

			/* initialize the CriticalSection Objects */
			InitializeCriticalSection(&csNet);

			/* create stop event */
			hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!hStopEvent)
			{
				D2GSEventLog("D2GSNetInitialize",
					"Failed in creating event object. Code: %lu", GetLastError());
				return FALSE;
			}

			/* init recv buffer */
			NRBInitialize(&nrbCS, PACKET_PEER_RECV_FROM_D2CS);
			NRBInitialize(&nrbDBS, PACKET_PEER_RECV_FROM_D2DBS);
			NSBInitialize(&nsbCS, PACKET_PEER_SEND_TO_D2CS);
			NSBInitialize(&nsbDBS, PACKET_PEER_SEND_TO_D2DBS);
			ZeroMemory(&d2netstat, sizeof(d2netstat));
			tick = GetTickCount();
			d2netstat.d2cs.tick = tick;
			d2netstat.d2dbs.tick = tick;

			/* initialize */
			wVersionRequested = MAKEWORD(2, 0);
			err = WSAStartup(wVersionRequested, &wsaData);
			if (err != 0)
			{
				D2GSEventLog("D2GSNetInitialize", "Error in initializing Winsock DLL");
				return FALSE;
			}

			/* create event object for packet receiving from D2CS or D2DBS */
			hRecvCS = WSACreateEvent();
			hRecvDBS = WSACreateEvent();
			if (!hRecvCS || !hRecvDBS) return FALSE;

			/* create thread to process net event */
			ghNetThread = CreateThread(NULL, 0, D2GSNetProcessor, NULL, 0, &dwThreadId);
			if (!ghNetThread)
			{
				D2GSEventLog("D2NetInitialize",
					"Can't CreateThread D2GSNetProcessor. Code: %lu", GetLastError());
				CleanupRoutineForNet();
				return FALSE;
			}

			/* add to the cleanup routine list */
			if (CleanupRoutineInsert(CleanupRoutineForNet, "D2GS Network"))
			{
				return TRUE;
			}
			else
			{
				/* do some cleanup before quiting */
				CleanupRoutineForNet();
				return FALSE;
			}

		} /* End of D2NetInitialize() */


		/*********************************************************************
		 * Purpose: to shutdown the network sub-system
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int CleanupRoutineForNet(void)
		{
			if (hStopEvent)
			{
				SetEvent(hStopEvent);
				if (ghNetThread)
				{
					WaitForSingleObject(ghNetThread, INFINITE);
					CloseHandle(ghNetThread);
					ghNetThread = NULL;
				}
				if (bConnectedToCS) CloseConnectionToD2CS();
				if (hRecvCS) WSACloseEvent(hRecvCS);
				if (hRecvDBS) WSACloseEvent(hRecvDBS);
				CloseHandle(hStopEvent);
				hStopEvent = NULL;
			}

			WSACleanup();

			/* destroy all the CriticalSection Objects */
			DeleteCriticalSection(&csNet);

			return TRUE;

		} /* End of CleanupRoutineForNet() */


		/*********************************************************************
		 * Purpose: to connect to D2CS
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		DWORD WINAPI D2GSConnectToD2xS(LPVOID lpParameter)
		{
			u_int				sock;
			struct sockaddr_in	sin;
			WSAEVENT			hConnEvent;
			WSANETWORKEVENTS	NetEvents;
			DWORD				dwWait;
			HANDLE				hEvents[4];
			DWORD				flag;
			BOOL				keepalive;
			int					bufsize, optlen;


			flag = (DWORD)lpParameter;

			if (bConnectedToCS) CloseConnectionToD2CS();
			hConnEvent = WSACreateEvent();
			if (hConnEvent == WSA_INVALID_EVENT)
			{
				D2GSEventLog("D2GSConnectToD2xS",
					"Can't WSACreateEvent. Code: %d", WSAGetLastError());
				return FALSE;
			}

			/* create new socket */
			while (WaitForSingleObject(hStopEvent, 0) != WAIT_OBJECT_0)
			{
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if (sock == INVALID_SOCKET)
				{
					D2GSEventLog("D2GSConnectToD2xS",
						"Can't create new socket. Code: %d", WSAGetLastError());
					Sleep(DEFAULT_CONNECT_INTERVAL);
					continue;
				}
				else break;
			}

			/* set socket keep alive option */
			keepalive = TRUE;
			setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)(&keepalive), sizeof(keepalive));
			bufsize = 32768;
			optlen = sizeof(bufsize);
			setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, optlen);

			/* try to coonnect to D2xS */
			while (WaitForSingleObject(hStopEvent, 0) != WAIT_OBJECT_0)
			{
				if (WSAEventSelect(sock, hConnEvent, FD_CONNECT))
				{
					D2GSEventLog("D2GSConnectToD2xS",
						"Failed in WSAEventSelect when connecting. Code: %d", WSAGetLastError());
					Sleep(DEFAULT_CONNECT_INTERVAL);
					continue;
				}

				ZeroMemory(&sin, sizeof(sin));
				sin.sin_family = PF_INET;
				if (flag == D2CSERVER)
				{
					sin.sin_addr.s_addr = d2gsconf.d2csip;
					sin.sin_port = d2gsconf.d2csport;
				}
				else
				{
					sin.sin_addr.s_addr = d2gsconf.d2dbsip;
					sin.sin_port = d2gsconf.d2dbsport;
				}
				if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) != 0)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						D2GSEventLog("D2GSConnectToD2xS",
							"Can't connect to %s. Code: %d", WSAGetLastError(),
							(flag == D2CSERVER) ? "D2CS" : "D2DBS");
						Sleep(DEFAULT_CONNECT_INTERVAL);
						continue;
					}
				}

				while (WaitForSingleObject(hStopEvent, 0) != WAIT_OBJECT_0)
				{
					hEvents[0] = hStopEvent;
					hEvents[1] = hConnEvent;
					dwWait = WSAWaitForMultipleEvents(2, hEvents, FALSE, INFINITE, FALSE);
					if (dwWait == WSA_WAIT_TIMEOUT) continue;
					else if (dwWait == WSA_WAIT_EVENT_0) break;
					else if (dwWait == (WSA_WAIT_EVENT_0 + 1))
					{
						if (WSAEnumNetworkEvents(sock, hConnEvent, &NetEvents))
						{
							D2GSEventLog("D2GSConnectToD2xS",
								"Failed in WSAEnumNetworkEvents. Code: %d", WSAGetLastError());
							break;
						}
						if (NetEvents.lNetworkEvents & FD_CONNECT)
						{
							if (NetEvents.iErrorCode[FD_CONNECT_BIT]) break;
							if (flag == D2CSERVER)
							{
								sockCS = sock;
								bConnectedToCS = TRUE;
							}
							else
							{
								sockDBS = sock;
								bConnectedToDBS = TRUE;
							}
						}
						else break;
						break;
					}
				}

				if ((flag == D2CSERVER) && bConnectedToCS)
				{
					D2GSEventLog("D2GSConnectToD2xS", "Connected to D2CS Successfully");
					WSAEventSelect(sock, hConnEvent, 0);
					D2GSSendClassToD2CS();
					D2GSSendNetData(&nsbCS);
					break;
				}
				else if ((flag != D2CSERVER) && bConnectedToDBS)
				{
					D2GSEventLog("D2GSConnectToD2xS", "Connected to D2DBS Successfully");
					WSAEventSelect(sock, hConnEvent, 0);
					D2GSSendClassToD2DBS();
					D2GSSendNetData(&nsbDBS);
					break;
				}
				else
				{
					D2GSEventLog("D2GSConnectToD2xS",
						"Failed connecting to %s, wait to retry",
						(flag == D2CSERVER) ? "D2CS" : "D2DBS");
					WaitForSingleObject(hStopEvent, d2gsconf.intervalreconnectd2cs * 100);
					continue;
				}
			}

			if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0)
			{
				D2GSEventLog("D2GSConnectToD2xS", "Cancel connecting to D2CS");
				if (sock != INVALID_SOCKET) CloseConnectionToD2CS();
			}
			WSACloseEvent(hConnEvent);
			return TRUE;

		} /* End of D2GSConnectToD2xS() */


		/*********************************************************************
		 * Purpose: to close connection to D2CS
		 * Return: NONE
		 *********************************************************************/
		void CloseConnectionToD2CS(void)
		{
			if (sockCS > 0)
			{
				shutdown(sockCS, SD_BOTH);
				closesocket(sockCS);
			}
			sockCS = INVALID_SOCKET;
			bConnectedToCS = FALSE;
			D2GSActive(FALSE);
			D2GSEndAllGames();
			NRBInitialize(&nrbCS, PACKET_PEER_RECV_FROM_D2CS);
			NSBInitialize(&nsbCS, PACKET_PEER_SEND_TO_D2CS);
			D2GSEventLog("CloseConnectionToD2CS", "Close Connection to D2CS");
			return;

		} /* End of CloseConnectionToD2CS() */


		/*********************************************************************
		 * Purpose: to close connection to D2DBS
		 * Return: NONE
		 *********************************************************************/
		void CloseConnectionToD2DBS(void)
		{
			CloseConnectionToD2CS();
			Sleep(5000);
			if (sockDBS > 0)
			{
				shutdown(sockDBS, SD_BOTH);
				closesocket(sockDBS);
			}
			sockDBS = INVALID_SOCKET;
			bConnectedToDBS = FALSE;
			NRBInitialize(&nrbDBS, PACKET_PEER_RECV_FROM_D2DBS);
			NSBInitialize(&nsbDBS, PACKET_PEER_SEND_TO_D2DBS);
			D2GSEventLog("CloseConnectionToD2DBS", "Close Connection to D2DBS");
			return;

		} /* End of CloseConnectionToD2CS() */


		/*********************************************************************
		 * Purpose: net packet processor
		 * Return: return value of the thread
		 *********************************************************************/
		DWORD WINAPI D2GSNetProcessor(LPVOID lpParameter)
		{
			DWORD		dwThreadId;
			HANDLE		hThread;
			D2GSPACKET	packet;
			int			val;
			HANDLE		hEvents[2];
			DWORD		dwWait;

		netloop:

			/* if not connected to D2DBS, try to connect */
			if (!bConnectedToDBS)
			{
				/* create thread to connect to D2DBS */
				hThread = CreateThread(NULL, 0,
					D2GSConnectToD2xS, (LPVOID)D2DBSERVER, 0, &dwThreadId);
				if (!hThread)
				{
					D2GSEventLog("D2GSNetProcessor",
						"Can't CreateThread D2GSConnectToD2xS. Code: %lu", GetLastError());
					Sleep(DEFAULT_CONNECT_INTERVAL);
					goto netloop;
				}
				hEvents[0] = hStopEvent;
				hEvents[1] = hThread;
				dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
				CloseHandle(hThread);
				if (dwWait == WAIT_OBJECT_0)
					return FALSE;
				else
				{
					WaitForSingleObject(hStopEvent, d2gsconf.intervalreconnectd2cs * 100);
					goto netloop;
				}
			}

			/* if not connected to D2CS, try to connect */
			if (!bConnectedToCS)
			{
				/* create thread to connect to D2CS */
				hThread = CreateThread(NULL, 0,
					D2GSConnectToD2xS, (LPVOID)D2CSERVER, 0, &dwThreadId);
				if (!hThread)
				{
					D2GSEventLog("D2GSNetProcessor",
						"Can't CreateThread D2GSConnectToD2xS. Code: %lu", GetLastError());
					Sleep(DEFAULT_CONNECT_INTERVAL);
					goto netloop;
				}
				hEvents[0] = hStopEvent;
				hEvents[1] = hThread;
				dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
				CloseHandle(hThread);
				if (dwWait == WAIT_OBJECT_0)
					return FALSE;
				else
				{
					WaitForSingleObject(hStopEvent, d2gsconf.intervalreconnectd2cs * 100);
					goto netloop;
				}
			}

			/* to read the packet and told other routine to deal with it */
			val = D2GSNetRecvPacket();
			if (val)
			{
				switch (val)
				{
				case D2GS_WAIT_TIMEOUT:
					break;
				case ERROR_D2GSNET_RECV_TIMEOUT:
				case D2GSERROR_BAD_PARAMETER:
					Sleep(1000);
					break;
				case ERROR_D2CS_WAITEVENT:
				case ERROR_D2CS_ENUMNETEVENT:
				case ERROR_D2CS_CONNCLOSE:
				case ERROR_D2CS_RECV:
					//			CloseConnectionToD2CS();
					//			WaitForSingleObject(hStopEvent, d2gsconf.intervalreconnectd2cs*100);
					//			break;
				case ERROR_D2DBS_WAITEVENT:
				case ERROR_D2DBS_ENUMNETEVENT:
				case ERROR_D2DBS_CONNCLOSE:
				case ERROR_D2DBS_RECV:
					CloseConnectionToD2DBS();
					break;
				}
			}
			else
			{
				// deal with the packet
				while (TRUE)
				{
					if (NRBRemovePacketOut(&nrbCS, &packet)) break;
#ifdef DEBUG
					DebugNetPacket(&packet);
#endif
					d2netstat.d2cs.recvpacket++;
					d2netstat.d2cs.recvbytes += (packet.datalen);
					D2GSHandleS2SPacket(&packet);
				}
				while (TRUE)
				{
					if (NRBRemovePacketOut(&nrbDBS, &packet)) break;
#ifdef DEBUG
					DebugNetPacket(&packet);
#endif
					d2netstat.d2dbs.recvpacket++;
					d2netstat.d2dbs.recvbytes += (packet.datalen);
					D2GSHandleS2SPacket(&packet);
				}
			}

			D2GSSendNetData(&nsbCS);
			D2GSSendNetData(&nsbDBS);

			/* loop */
			if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0) return TRUE;
			else goto netloop;

		} /* End of D2GSNetProcessor() */


		/*********************************************************************
		 * Purpose: to read packet from the connections
		 * Return: 0(success), >0 error code
		 *********************************************************************/
		int D2GSNetRecvPacket(void)
		{
			WSAEVENT			hEvents[4];
			DWORD				dwWait;
			WSANETWORKEVENTS	NetEvents;
			char				buffer[8192];
			int					val;
			int					retval;

			if (!bConnectedToCS) return ERROR_D2CS_CONNCLOSE;

			if (WSAEventSelect(sockCS, hRecvCS, FD_READ | FD_WRITE | FD_CLOSE))
			{
				D2GSEventLog("D2GSNetRecvPacket",
					"Failed in WSAEventSelect(). Code %u", WSAGetLastError());
				return ERROR_D2GSNET_RECV_TIMEOUT;
			}
			if (WSAEventSelect(sockDBS, hRecvDBS, FD_READ | FD_WRITE | FD_CLOSE))
			{
				D2GSEventLog("D2GSNetRecvPacket",
					"Failed in WSAEventSelect(). Code %u", WSAGetLastError());
				return ERROR_D2GSNET_RECV_TIMEOUT;
			}

			hEvents[0] = hStopEvent;
			hEvents[1] = hRecvCS;
			hEvents[2] = hRecvDBS;
			retval = ERROR_D2GSNET_RECV_TIMEOUT;
			dwWait = WSAWaitForMultipleEvents(3, hEvents, FALSE, 100, FALSE);
			if (dwWait == WSA_WAIT_TIMEOUT)
				return D2GS_WAIT_TIMEOUT;
			else if (dwWait == WSA_WAIT_EVENT_0 || dwWait == WAIT_IO_COMPLETION)
			{
				return ERROR_D2GSNET_RECV_TIMEOUT;
			}
			else if (dwWait == WSA_WAIT_EVENT_0 + 1)
			{
				/*************************************************************************/
				/* data from D2CS */
				if (WSAEnumNetworkEvents(sockCS, hRecvCS, &NetEvents))
				{
					D2GSEventLog("D2GSNetRecvPacket",
						"Failed in WSAEnumNetworkEvents. Code: %d", WSAGetLastError());
					return ERROR_D2CS_ENUMNETEVENT;
				}
				if (NetEvents.lNetworkEvents & FD_CLOSE)
					return ERROR_D2CS_CONNCLOSE;
				if (NetEvents.lNetworkEvents & FD_WRITE)
				{
					if (NetEvents.iErrorCode[FD_WRITE_BIT])
						return ERROR_D2CS_CONNCLOSE;
					nsbCS.writable = TRUE;
					D2GSEventLog("D2GSNetRecvPacket", "CS socket become writable");
					retval = 0;
				}
				if (NetEvents.lNetworkEvents & FD_READ)
				{
					if (NetEvents.iErrorCode[FD_READ_BIT])
						return ERROR_D2CS_CONNCLOSE;
					/* now can read the data */
					val = recv(sockCS, buffer, sizeof(buffer), 0);
					if (val < 0)
					{
						D2GSEventLog("D2GSNetRecvPacket",
							"Error in recv() with socket to D2CS. Code: %d", WSAGetLastError());
						return ERROR_D2CS_RECV;
					}
					else if (val == 0)
						return ERROR_D2CS_CONNCLOSE;
					NRBAddNewData(&nrbCS, buffer, val);
					retval = 0;
				}
				/*************************************************************************/
			}
			else if (dwWait == WSA_WAIT_EVENT_0 + 2)
			{
				/*************************************************************************/
				/* data from D2DBS */
				if (WSAEnumNetworkEvents(sockDBS, hRecvDBS, &NetEvents))
				{
					D2GSEventLog("D2GSNetRecvPacket",
						"Failed in WSAEnumNetworkEvents. Code: %d", WSAGetLastError());
					return ERROR_D2DBS_ENUMNETEVENT;
				}
				if (NetEvents.lNetworkEvents & FD_CLOSE)
					return ERROR_D2DBS_CONNCLOSE;
				if (NetEvents.lNetworkEvents & FD_WRITE)
				{
					if (NetEvents.iErrorCode[FD_WRITE_BIT])
						return ERROR_D2CS_CONNCLOSE;
					nsbCS.writable = TRUE;
					D2GSEventLog("D2GSNetRecvPacket", "DBS socket become writable");
					retval = 0;
				}
				if (NetEvents.lNetworkEvents & FD_READ)
				{
					if (NetEvents.iErrorCode[FD_READ_BIT])
						return ERROR_D2DBS_CONNCLOSE;
					/* now can read the data */
					val = recv(sockDBS, buffer, sizeof(buffer), 0);
					if (val < 0)
					{
						D2GSEventLog("D2GSNetRecvPacket",
							"Error in recv() with socket to D2DBS. Code: %d", WSAGetLastError());
						return ERROR_D2DBS_RECV;
					}
					else if (val == 0)
						return ERROR_D2DBS_CONNCLOSE;
					NRBAddNewData(&nrbDBS, buffer, val);
					retval = 0;
				}
				/*************************************************************************/
			}
			else
				return ERROR_D2CS_WAITEVENT;

			return retval;

		} /* End of D2GSNetRecvPacket() */


		/*********************************************************************
		 * Purpose: to send data to network
		 * Return: 0(success), >0 error code
		 *********************************************************************/
		int D2GSSendNetData(NETSENDBUFFER* lpnsr)
		{
			int		bytes;
			u_int	sock;
			u_char* buf;
			u_int	datalen;
			int		ret;

			//if (!(lpnsr->writable)) return -1;
			if (!(lpnsr->length)) return -1;
			if (lpnsr->peer == PACKET_PEER_SEND_TO_D2CS) sock = sockCS;
			else sock = sockDBS;

			ret = 0;
			EnterCriticalSection(&csNet);
			NSBGetData(lpnsr, &buf, &datalen);
			if (datalen)
			{
				bytes = send(sock, (char*)buf, datalen, 0);
				if (bytes <= 0)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						D2GSEventLog("D2GSSendNetData",
							"socket of %s block, %u",
							lpnsr->peer == PACKET_PEER_SEND_TO_D2CS ? "CS" : "DBS", WSAGetLastError());
						lpnsr->writable = FALSE;
					}
					else
					{
						D2GSEventLog("D2GSSendNetData", "send failed, code: %u", WSAGetLastError());
						ret = -1;
					}
				}
				else if (bytes != (int)datalen)
				{
					NSBRemoveData(lpnsr, bytes);
					//lpnsr->writable = FALSE;
					D2GSEventLog("D2GSSendNetData",
						"socket of %s patial sended",
						lpnsr->peer == PACKET_PEER_SEND_TO_D2CS ? "CS" : "DBS");
				}
				else
				{
					NSBRemoveData(lpnsr, datalen);
				}
			}
			LeaveCriticalSection(&csNet);
			return ret;

		} /* End of D2GSSendNetData() */


		/*********************************************************************
		 * Purpose: to put a packet to the send buffer
		 * Return: 0(success), >0 error code
		 *********************************************************************/
		int D2GSNetSendPacket(D2GSPACKET* lpPacket)
		{
			/*
			 * Fix me.
			 * Now just send the packet out directely, no further examination. Should
			 * check the return value, check if the socket send buffer is full. etc.
			 * Should build up a send-queue to do this.
			 * and must do some error handle, such as net down,
			 * should call CloseConnectionToD2CS()
			 */
			if (!lpPacket) return ERROR_BAD_PACKET_PTR;
			if (lpPacket->datalen <= 0) return 0;

#ifdef DEBUG
			DebugNetPacket(lpPacket);
#endif

			switch (lpPacket->peer)
			{
			case PACKET_PEER_SEND_TO_D2CS:
				if (!bConnectedToCS) return ERROR_D2CS_CONNCLOSE;
				/*if (send(sockCS, lpPacket->data, lpPacket->datalen, 0) != lpPacket->datalen)
					return ERROR_D2CS_SEND;
				else {
					d2netstat.d2cs.sendpacket ++;
					d2netstat.d2cs.sendbytes += (lpPacket->datalen);
					return 0;
				}*/
				if (NSBAppendData(&nsbCS, lpPacket) == 0)
				{
					d2netstat.d2cs.sendpacket++;
					d2netstat.d2cs.sendbytes += (lpPacket->datalen);
					D2GSSendNetData(&nsbCS);
					return 0;
				}
			case PACKET_PEER_SEND_TO_D2DBS:
				if (!bConnectedToDBS) return ERROR_D2DBS_CONNCLOSE;
				/*if (send(sockDBS, lpPacket->data, lpPacket->datalen, 0) != lpPacket->datalen)
					return ERROR_D2CS_SEND;
				else {
					d2netstat.d2dbs.sendpacket ++;
					d2netstat.d2dbs.sendbytes += (lpPacket->datalen);
					return 0;
				}*/
				if (NSBAppendData(&nsbDBS, lpPacket) == 0)
				{
					d2netstat.d2dbs.sendpacket++;
					d2netstat.d2dbs.sendbytes += (lpPacket->datalen);
					D2GSSendNetData(&nsbDBS);
					return 0;
				}
			}

			return ERROR_BAD_PACKET_PTR;

		} /* End of D2GSNetSendPacket() */


		/*=================================================================================*/


		/*********************************************************************
		 * Purpose: to initialize the NETRECVBUFFER
		 * Return: None
		 *********************************************************************/
		void NRBInitialize(NETRECVBUFFER* lpnbr, u_short peer)
		{
			if (!lpnbr) return;
			ZeroMemory(lpnbr, sizeof(NETRECVBUFFER));
			lpnbr->lpHead = (lpnbr->data);
			lpnbr->lpTail = (lpnbr->data);
			lpnbr->length = 0;
			lpnbr->rest = DEFAULT_RECV_BUFFER_LENGTH;
			lpnbr->peer = peer;

			return;

		} /* End of NRBInitialize() */


		/*********************************************************************
		 * Purpose: to add some new data to the NETRECVBUFFER
		 * Return: None
		 *********************************************************************/
		void NRBAddNewData(NETRECVBUFFER* lpnbr, void* lpdata, u_int datalen)
		{
			u_int		bmove;

			if (!lpnbr || !lpdata) return;
			if (lpnbr->length > DEFAULT_RECV_BUFFER_LENGTH) return;

			if (lpnbr->rest < datalen)
			{
				/* the rest of the buffer not enough, compress it */
				bmove = (lpnbr->lpHead) - (lpnbr->data);
				if (bmove)
				{
					if (lpnbr->length) CopyMemory(lpnbr->data, lpnbr->lpHead, lpnbr->length);
					lpnbr->lpHead = (lpnbr->data);
					lpnbr->lpTail = (lpnbr->data) + (lpnbr->length);
					lpnbr->rest = DEFAULT_RECV_BUFFER_LENGTH - (lpnbr->length);
				}
			}

			/* after data moving */
			if (lpnbr->rest >= datalen)
			{
				CopyMemory(lpnbr->lpTail, lpdata, datalen);
				lpnbr->lpTail += datalen;
				lpnbr->rest -= datalen;
				lpnbr->length += datalen;
			}
			else
			{
				/* if not enough rest room, drop the whole packet, instead of just
					dropping some bytes */
					/*
					CopyMemory(lpnbr->lpTail, lpdata, lpnbr->rest);
					bmove = datalen - lpnbr->rest;
					lpnbr->lpTail += (lpnbr->rest);
					lpnbr->rest   =  0;
					lpnbr->length =  DEFAULT_RECV_BUFFER_LENGTH;
					*/
				D2GSEventLog("NBRAddNewData",
					"!!! Receive buffer for '%s' overflow, lost %u bytes",
					(lpnbr->peer == PACKET_PEER_RECV_FROM_D2CS) ? "D2CS" : "D2DBS", datalen);
			}

			return;

		} /* End of NBRAddNewData() */


		/*********************************************************************
		 * Purpose: to take a D2GSPACKET out of the NETRECVBUFFER
		 * Return: 0(success), other(failed)
		 *********************************************************************/
		int NRBRemovePacketOut(NETRECVBUFFER* lpnbr, D2GSPACKET* lpPkt)
		{
			t_d2cs_d2gs_generic* ph;
			u_int				bytes;

			if (!lpnbr || !lpPkt) return D2GSERROR_BAD_PARAMETER;
			if (lpnbr->length < sizeof(t_d2cs_d2gs_generic)) return ERROR_NOT_INTEGRITY_PACKET;

			ph = (t_d2cs_d2gs_generic*)(lpnbr->lpHead);
			bytes = (u_int)(bn_ntohs(ph->h.size));
			if (bytes == 0) return ERROR_NOT_INTEGRITY_PACKET;
			if ((lpnbr->length) >= bytes)
			{
				CopyMemory(lpPkt->data, lpnbr->lpHead, bytes);
				lpPkt->datalen = bytes;
				lpPkt->peer = lpnbr->peer;
				lpnbr->lpHead += bytes;
				lpnbr->length -= bytes;
				return 0;
			}
			else
				return ERROR_NOT_INTEGRITY_PACKET;

		} /* End of NRBRemovePacketOut() */


		/*=================================================================================*/


		/*********************************************************************
		 * Purpose: to initialize the NETSENDBUFFER
		 * Return: None
		 *********************************************************************/
		void NSBInitialize(NETSENDBUFFER* lpnsr, u_short peer)
		{
			if (!lpnsr) return;
			ZeroMemory(lpnsr, sizeof(NETSENDBUFFER));
			lpnsr->lpHead = (lpnsr->data);
			lpnsr->lpTail = (lpnsr->data);
			lpnsr->length = 0;
			lpnsr->rest = DEFAULT_SEND_BUFFER_LENGTH;
			lpnsr->peer = peer;
			lpnsr->writable = TRUE;

			return;

		} /* End of NSBInitialize() */


		/*********************************************************************
		 * Purpose: to add some new data to the NETSENDBUFFER
		 * Return: 0(success), other(failed)
		 *********************************************************************/
		int NSBAppendData(NETSENDBUFFER* lpnsr, D2GSPACKET* lpPkt)
		{
			u_int		bmove;
			u_char* lpdata;
			u_int		datalen;
			int			ret;

			if (!lpnsr || !lpPkt) return -1;
			if (lpnsr->length > DEFAULT_SEND_BUFFER_LENGTH) return -1;

			EnterCriticalSection(&csNet);
			lpdata = lpPkt->data;
			datalen = lpPkt->datalen;
			if (lpnsr->rest < datalen)
			{
				/* the rest of the buffer not enough, compress it */
				bmove = (lpnsr->lpHead) - (lpnsr->data);
				if (bmove)
				{
					if (lpnsr->length) CopyMemory(lpnsr->data, lpnsr->lpHead, lpnsr->length);
					lpnsr->lpHead = (lpnsr->data);
					lpnsr->lpTail = (lpnsr->data) + (lpnsr->length);
					lpnsr->rest = DEFAULT_SEND_BUFFER_LENGTH - (lpnsr->length);
				}
			}

			/* after data moving */
			if (lpnsr->rest >= datalen)
			{
				CopyMemory(lpnsr->lpTail, lpdata, datalen);
				lpnsr->lpTail += datalen;
				lpnsr->rest -= datalen;
				lpnsr->length += datalen;
				ret = 0;
			}
			else
			{
				/* if not enough rest room, drop the whole packet, instead of some
					dropping some bytes */
					/*
					CopyMemory(lpnsr->lpTail, lpdata, lpnsr->rest);
					bmove = datalen - lpnsr->rest;
					lpnsr->lpTail += (lpnsr->rest);
					lpnsr->rest   =  0;
					lpnsr->length =  DEFAULT_SEND_BUFFER_LENGTH;
					*/
				D2GSEventLog("NSBAddNewData",
					"!!! Send buffer for '%s' overflow",
					(lpnsr->peer == PACKET_PEER_RECV_FROM_D2CS) ? "D2CS" : "D2DBS");
				ret = -1;
			}
			LeaveCriticalSection(&csNet);

			return ret;

		} /* End of NSBAppendData() */


		/*********************************************************************
		 * Purpose: to take get the pending data to for sending
		 * Return: 0(success), other(failed)
		 *********************************************************************/
		int NSBGetData(NETSENDBUFFER* lpnsr, u_char** lppdata, u_int* datalen)
		{
			if (!lpnsr) return D2GSERROR_BAD_PARAMETER;

			*lppdata = lpnsr->lpHead;
			*datalen = lpnsr->length;
			return 0;

		} /* End of NSBGetData() */


		/*********************************************************************
		 * Purpose: to remove some data from the buffer
		 * Return: 0(success), other(failed)
		 *********************************************************************/
		int NSBRemoveData(NETSENDBUFFER* lpnsr, u_int datalen)
		{
			if (!lpnsr) return D2GSERROR_BAD_PARAMETER;
			if (datalen > (lpnsr->length)) return D2GSERROR_BAD_PARAMETER;

			EnterCriticalSection(&csNet);
			lpnsr->lpHead += datalen;
			lpnsr->length -= datalen;
			LeaveCriticalSection(&csNet);
			return 0;

		} /* End of NSBRemoveData() */


		/*=================================================================================*/


		/*********************************************************************
		 * Purpose: to get ipaddr and port of a socket
		 * Return: 0(success), other(failed)
		 *********************************************************************/
		int D2GSGetSockName(int server, DWORD* ipaddr, DWORD* port)
		{
			u_int				sock;
			struct sockaddr_in	name;
			int					namelen;

			if (server == D2CSERVER) sock = sockCS;
			else sock = sockDBS;
			*ipaddr = *port = 0;

			namelen = sizeof(name);
			if (getsockname(sock, (struct sockaddr*)&name, &namelen))
			{
				*ipaddr = 0;
				*port = 0;
				return -1;
			}
			else
			{
				*ipaddr = ntohl(name.sin_addr.s_addr);
				*port = (DWORD)(ntohs(name.sin_port));
				return 0;
			}

		} /* End of D2GSGetSockName() */


		/*********************************************************************
		 * Purpose: to get status of current connection
		 * Return: status
		 *********************************************************************/
		int D2GSGetConnectionStatus(void)
		{
			int		status;

			status = 0;
			if (bConnectedToCS)  status |= D2CSERVER;
			if (bConnectedToDBS) status |= D2DBSERVER;
			return status;

		} /* End of D2GSGetConnectionStatus() */


		/*********************************************************************
		 * Purpose: to calculate the net statistic
		 * Return: none
		 *********************************************************************/
		void D2GSCalculateNetStatistic(void)
		{
			DWORD		tick, ticknow;
			u_int		rbytes, sbytes;
			double		rrate, srate;

			/* D2CS */
			tick = d2netstat.d2cs.tick;
			rbytes = d2netstat.d2cs.recvbytes - d2netstat.d2cs.orgrecvbytes;
			sbytes = d2netstat.d2cs.sendbytes - d2netstat.d2cs.orgsendbytes;
			d2netstat.d2cs.orgrecvbytes = d2netstat.d2cs.recvbytes;
			d2netstat.d2cs.orgsendbytes = d2netstat.d2cs.sendbytes;
			ticknow = GetTickCount();
			tick = ticknow - tick;
			if (tick)
			{
				rrate = (double)rbytes / (double)tick;
				srate = (double)sbytes / (double)tick;
				d2netstat.d2cs.recvrate = rrate;
				d2netstat.d2cs.sendrate = srate;
				if (d2netstat.d2cs.peakrecvrate < rrate) d2netstat.d2cs.peakrecvrate = rrate;
				if (d2netstat.d2cs.peaksendrate < srate) d2netstat.d2cs.peaksendrate = srate;
				d2netstat.d2cs.tick = ticknow;
			}

			/* D2DBS */
			tick = d2netstat.d2dbs.tick;
			rbytes = d2netstat.d2dbs.recvbytes - d2netstat.d2dbs.orgrecvbytes;
			sbytes = d2netstat.d2dbs.sendbytes - d2netstat.d2dbs.orgsendbytes;
			d2netstat.d2dbs.orgrecvbytes = d2netstat.d2dbs.recvbytes;
			d2netstat.d2dbs.orgsendbytes = d2netstat.d2dbs.sendbytes;
			ticknow = GetTickCount();
			tick = ticknow - tick;
			if (tick)
			{
				rrate = (double)rbytes / (double)tick;
				srate = (double)sbytes / (double)tick;
				d2netstat.d2dbs.recvrate = rrate;
				d2netstat.d2dbs.sendrate = srate;
				if (d2netstat.d2dbs.peakrecvrate < rrate) d2netstat.d2dbs.peakrecvrate = rrate;
				if (d2netstat.d2dbs.peaksendrate < srate) d2netstat.d2dbs.peaksendrate = srate;
				d2netstat.d2dbs.tick = ticknow;
			}

			return;

		} /* End of D2GSCalculateNetStatistic() */


		/*********************************************************************
		 * Purpose: to get the net statistic
		 * Return: none
		 *********************************************************************/
		void D2GSGetNetStatistic(D2GSNETSTATISTIC* pnetstat)
		{
			if (!pnetstat) return;
			CopyMemory(pnetstat, &d2netstat, sizeof(d2netstat));
			return;

		} /* End of D2GSGetNetStatistic() */

	}

}