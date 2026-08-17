#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H


namespace pvpgn
{

	namespace d2gs
	{

		/* registry key names */
#define REGKEY_ROOT						"Software\\D2Server\\D2GS"
#define REGKEY_D2CSIP					"D2CSIP"
#define REGKEY_D2CSPORT					"D2CSPort"
#define REGKEY_D2DBSIP					"D2DBSIP"
#define REGKEY_D2DBSPORT				"D2DBSPort"
#define REGKEY_MAXGAMES					"MaxGames"
#define REGKEY_MAXPREFERUSERS			"MaxPreferUsers"
#define REGKEY_ENABLENTMODE				"EnableNTMode"
#define REGKEY_ENABLEGEPATCH			"EnableGEPatch"
#define REGKEY_ENABLEPRECACHEMODE		"EnablePreCacheMode"
#define REGKEY_ENABLEGELOG				"EnableGELog"
#define REGKEY_ENABLEGEMSG				"EnableGEMsg"
#define REGKEY_ENABLEGSLOG				"EnableGSLog"
#define REGKEY_DEBUGNETPACKET			"DebugNetPacket"
#define REGKEY_DEBUGEVENTCALLBACK		"DebugEventCallback"
#define REGKEY_IDLESLEEP				"IdleSleep"
#define REGKEY_BUSYSLEEP				"BusySleep"
#define REGKEY_CHARPENDINGTIMEOUT		"CharPendingTimeout"
#define REGKEY_INTERVALRECONNECTD2CS	"IntervalReconnectD2CS"
#define REGKEY_ADMINPWD					"AdminPassword"
#define REGKEY_ADMINPORT				"AdminPort"
#define REGKEY_ADMINTIMEOUT				"AdminTimeout"
#define REGKEY_MAXGAMELIFE				"MaxGameLife"
#define REGKEY_D2CSSECRECT				"D2CSSecrect"
#define REGKEY_GSSHUTDOWNINTERVAL		"GSShutdownInterval"
#define REGKEY_MOTD						"MOTD"
#define REGKEY_MULTICPUMASK				"MultiCPUMask"
#define REGKEY_MAX_PACKET_PER_SECOND	"MaxPacketPerSecond"
#define REGKEY_SERVER_CONF_FILE			"ServerConfFile"
#define REGKEY_AUTOUPDATE				"AutoUpdate"
#define REGKEY_AUTOUPDATEVER			"AutoUpdateVer"
#define REGKEY_AUTOUPDATE_TIMEOUT		"AutoUpdateTimeout"
#define REGKEY_AUTOUPDATE_URL			"AutoUpdateUrl"


/* return value used by the functions */
#define REGKEY_ERROR				0x00
#define REGKEY_SUCCESS				0x01
#define REGKEY_CREATED_NEW_KEY		0x11
#define REGKEY_OPENED_EXISTING_KEY	0x12


/* functions */
		int  D2GSReadConfig(void);
		int  RegkeyOpen(HKEY hKeyRoot, LPCTSTR lpSubKey, PHKEY hKey, REGSAM sam);
		void RegkeyClose(HKEY hKey);
		int  RegkeyReadString(HKEY hKey, LPCTSTR name, char* buf, DWORD buflen);
		int  RegkeyReadDWORD(HKEY hKey, LPCTSTR name, DWORD* val);
		int  RegkeyWriteString(HKEY hKey, LPCTSTR name, LPCSTR buf);
		int  RegkeyWriteDWORD(HKEY hKey, LPCTSTR name, DWORD val);
		/* setting value in the registry */
		int  D2GSSetConfigDWORD(LPCSTR keyname, DWORD dwVal);
		int  D2GSSetConfigString(LPCSTR keyname, LPCSTR str);
		int  D2GSSetMaxGameLife(DWORD maxgamelife);
		int  D2GSSetAdminPassword(LPCSTR password);
		int D2GSSetMaxGames(DWORD maxgames);
		DWORD D2GSSetMultiCpuMask(int mask);
		DWORD D2GSSetEnableGSLog(int enable);
		int D2GSSetMaxPreferUsers(DWORD maxusers);
		DWORD D2GSGetShutdownStatus();

	}

}


#endif /* INCLUDED_CONFIG_H */