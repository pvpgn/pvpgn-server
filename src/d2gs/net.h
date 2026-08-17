#ifndef INCLUDED_NET_H
#define INCLUDED_NET_H


namespace pvpgn
{

	namespace d2gs
	{

		/* structure */
#define DEFAULT_RECV_BUFFER_LENGTH			65536
		typedef struct
		{
			u_short	peer;		/* descripion of the buffer, recv from whom? */
			u_short	reserved;
			u_char* lpHead;	/* pointer to the first byte of the valid data */
			u_char* lpTail;	/* pointer to the next byte of the last legal data */
			u_int	length;		/* legal data length in bytes */
			u_int	rest;		/* the rest avalible bytes to the end of the buffer */
			u_char	data[DEFAULT_RECV_BUFFER_LENGTH];
		} NETRECVBUFFER, * PNETRECVBUFFER, * LPNETRECVBUFFER;

#define DEFAULT_SEND_BUFFER_LENGTH			1024*1024
		typedef struct
		{
			u_short	peer;		/* descripion of the buffer, recv from whom? */
			u_short	reserved;
			u_char* lpHead;	/* pointer to the first byte of the valid data */
			u_char* lpTail;	/* pointer to the next byte of the last legal data */
			u_int	length;		/* legal data length in bytes */
			u_int	rest;		/* the rest avalible bytes to the end of the buffer */
			u_int	writable;	/* if the socket is writable */
			u_char	data[DEFAULT_SEND_BUFFER_LENGTH];
		} NETSENDBUFFER, * PNETSENDBUFFER, * LPNETSENDBUFFER;


		typedef struct
		{
			u_int	tick;
			u_int	recvpacket;
			u_int	sendpacket;
			u_int	recvbytes;
			u_int	sendbytes;
			u_int	orgrecvbytes;
			u_int	orgsendbytes;
			double	recvrate;
			double	sendrate;
			double	peakrecvrate;
			double	peaksendrate;
		} NETSTATISTIC, * PNETSTATISTIC, * LPNETSTATISTIC;


		typedef struct
		{
			NETSTATISTIC	d2cs;
			NETSTATISTIC	d2dbs;
		} D2GSNETSTATISTIC, * PD2NETSTATISTIC, * LPD2NETSTATISTIC;


		/* const */
#define DEFAULT_CONNECT_INTERVAL		1000
#define DEFAULT_CONNECT_INTERVAL2		30000
#define DEFAULT_CONNECT_TIMEOUT			400
#define DEFAULT_SHUTDOWN_TIMEOUT		5000
#define DEFAULT_NET_RECV_TIMEOUT		30


/* return by D2GSNetRecvPacket or D2GSNetSendPacket */
#define ERROR_D2GSNET_RECV_TIMEOUT		0x01
#define ERROR_BAD_PACKET_PTR			0x02
#define ERROR_D2CS_WAITEVENT			0x11
#define ERROR_D2CS_ENUMNETEVENT			0x12
#define ERROR_D2CS_CONNCLOSE			0x13
#define ERROR_D2CS_RECV					0x14
#define ERROR_D2CS_SEND					0x15
#define ERROR_D2DBS_WAITEVENT			0x31
#define ERROR_D2DBS_ENUMNETEVENT		0x32
#define ERROR_D2DBS_CONNCLOSE			0x33
#define ERROR_D2DBS_RECV				0x34
#define ERROR_D2DBS_SEND				0x35

#define ERROR_NOT_INTEGRITY_PACKET		0x50

#define D2GS_WAIT_TIMEOUT				0xff

/* functions */
		int   D2GSNetInitialize(void);
		int   CleanupRoutineForNet(void);
		DWORD WINAPI D2GSConnectToD2xS(LPVOID lpParameter);
		void  CloseConnectionToD2CS(void);
		void  CloseConnectionToD2DBS(void);
		DWORD WINAPI D2GSNetProcessor(LPVOID lpParameter);
		int   D2GSNetRecvPacket(void);
		int   D2GSSendNetData(NETSENDBUFFER* lpnsr);
		int   D2GSNetSendPacket(D2GSPACKET* lpPacket);
		int   D2GSGetSockName(int server, DWORD* ipaddr, DWORD* port);
		int  D2GSGetConnectionStatus(void);
		void D2GSCalculateNetStatistic(void);
		void D2GSGetNetStatistic(D2GSNETSTATISTIC* pnetstat);

		/* NETRECVBUFFER operator */
		void NRBInitialize(NETRECVBUFFER* lpnrb, u_short peer);
		void NRBAddNewData(NETRECVBUFFER* lpnrb, void* lpdata, u_int datalen);
		int  NRBRemovePacketOut(NETRECVBUFFER* lpnrb, D2GSPACKET* lpPkt);
		void NSBInitialize(NETSENDBUFFER* lpnsr, u_short peer);
		int  NSBAppendData(NETSENDBUFFER* lpnsr, D2GSPACKET* lpPkt);
		int  NSBGetData(NETSENDBUFFER* lpnsr, u_char** lppdata, u_int* datalen);
		int  NSBRemoveData(NETSENDBUFFER* lpnsr, u_int datalen);

	}

}


#endif /* INCLUDED_NET_H */