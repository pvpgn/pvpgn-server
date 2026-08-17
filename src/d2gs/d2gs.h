/*
 * d2gs.h: declaration of structures and constants
 *
 * 2001-08-20 faster
 *   modify
 */

#ifndef INCLUDED_D2GS_H
#define INCLUDED_D2GS_H


#include <windows.h>


		/* constants */
#define D2CSERVER					0x01
#define D2DBSERVER					0x02
#define D2GSERVER					0x04

/* version */
#define D2GS_VERSION				0x01133000
#define VERNUM						"1.13c"
#define BUILDDATE					__DATE__ " " __TIME__
#define D2GS_VERSION_STRING			D2COLOR_GOLDEN "D2GS Version " VERNUM ", build on " BUILDDATE
#define D2GS_VERSION_STRING_ONLY	"D2GS Version " VERNUM ", build on " BUILDDATE

#define D2GSERVER_MUTEX_NAME			"DIABLO_II_CLOSE_GAME_SERVER"
#define D2GS_STOP_EVENT_NAME			"D2GSERVER_WHO_STOP_ME"
#define DEFAULT_D2CS_PORT				6112
#define DEFAULT_D2DBS_PORT				6113
#define DEFAULT_ADMIN_PORT				8888
#define DEFAULT_ADMIN_TIMEOUT			300
#define DEFAULT_MAX_GAMES				100
#define DEFAULT_NT_MODE					0
#define DEFAULT_PRECACHE_MODE			0
#define DEFAULT_GE_PATCH				1
#define DEFAULT_GE_LOG					0
#define DEFAULT_GE_MSG					0
#define DEFAULT_DEBUGNETPACKET			0
#define DEFAULT_DEBUGEVENTCALLBACK		0
#define DEFAULT_CHARPENDINGTIMEOUT		200
#define DEFAULT_INTERVALRECONNECTD2CS	50
#define DEFAULT_MAXGAMELIFE				3600
#define DEFAULT_GS_SHUTDOWN_INTERVAL	15
#define DEFAULT_GS_SHUTDOWN_DELAY		300
#define DEFAULT_MULTICPUMASK			0

#define MAX_LINE_LEN				1024
#define	DEBUG_DUMPSIZE				0x40

/* string lengtn */
#define MAX_GAMENAME_LEN			16
#define MAX_GAMEPASS_LEN			32
#define MAX_GAMEDESC_LEN			32
#define MAX_ACCTNAME_LEN			16
#define MAX_CHARNAME_LEN			16
#define MAX_REALMNAME_LEN			32
#define MAX_REALMIPNAME_LEN			16

#define MAX_CHAR_IN_GAME			8
#define TIMER_TICK_IN_MS			100
#define GET_DATA_TIMEOUT			1000
#define SEND_MOTD_INTERVAL			2000

/* some error code */
#define D2GSERROR_NOT_ENOUGH_MEMORY		0x80
#define D2GSERROR_BAD_PARAMETER			0x81
#define D2GSERROR_GAME_IS_FULL			0x82
#define D2GSERROR_CHAR_ALREADY_IN_GAME	0x83


/* utilities */
#define YESNO(n) (n)?"Yes":"No"
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define NELEMS(array) (sizeof(array)/sizeof(array[0]))


namespace pvpgn
{

	namespace d2gs
	{

		typedef struct RAW_D2GSCONFIGS
		{
			char		d2csipstr[16];
			char		d2dbsipstr[16];
			DWORD		d2csip;		/* in network order */
			DWORD		d2dbsip;	/* in network order */
			u_short		d2csport;	/* in network order */
			u_short		d2dbsport;	/* in network order */
			BOOL		enablentmode;
			BOOL		enableprecachemode;
			BOOL		enablegepatch;
			BOOL		enablegelog;
			BOOL		enablegemsg;
			BOOL		enablegslog;
			BOOL		debugnetpacket;
			BOOL		debugeventcallback;
			DWORD		checksum;
			DWORD		gemaxgames;
			DWORD		gsmaxgames;
			DWORD		curgsmaxgames;
			DWORD		idlesleep;
			DWORD		busysleep;
			DWORD		charpendingtimeout;
			DWORD		intervalreconnectd2cs;
			DWORD		admintimeout;
			DWORD		maxgamelife;
			DWORD		gsshutdowninterval;
			DWORD		multicpumask;
			DWORD		maxpreferusers;
			DWORD		maxpacketpersecond;
			char		adminpwd[64];
			u_short		adminport;
			u_short		padding;
			char		d2cssecrect[32];
			char		motd[256];
			char		serverconffile[260];
		} D2GSCONFIGS, * PD2GSCONFIGS;


#define PACKET_PEER_RECV_FROM_D2CS		0x01
#define PACKET_PEER_SEND_TO_D2CS		0x02
#define PACKET_PEER_RECV_FROM_D2DBS		0x03
#define PACKET_PEER_SEND_TO_D2DBS		0x04
		typedef struct RAW_D2GSPACKET
		{
			u_short		peer;		/* where packet from or to? D2CS or D2DBS */
			u_short		datalen;	/* valid date length */
			u_char		data[16384];
		} D2GSPACKET, * PD2GSPACKET;


		typedef struct AutoUpdateSetting
		{
			BOOL		AutoUpdate;
			DWORD		AutoUpdateTimeout;
			DWORD		AutoUpdateVer;
			char		AutoUpdateUrl[260];
		}AutoUpdateSetting;


		/* function */
		void D2GSShutdown(unsigned int exitcode);
		void D2GSShutdownTimer(void);
		void D2GSBeforeShutdown(DWORD status, DWORD seconds);

	}

}


#endif /* INCLUDED_D2GS_H */