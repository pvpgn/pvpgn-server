#include <winsock2.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <stdlib.h>
#include "d2gs.h"
#include "config.h"
#include "eventlog.h"
#include "vars.h"


namespace pvpgn
{

	namespace d2gs
	{

		char gSomeBuffer[2048] = { 0 };

		/*********************************************************************
		 * Purpose: to read configurations to the D2GSCONFIGS structure
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		typedef struct ConfigMap
		{
			const char* KeyName;
			DWORD		Type; // 0-9
			const char* DefaultValue;
			void* ValuePtr;
		}ConfigMap;

		static ConfigMap gConfigMap[] =
		{
			{REGKEY_D2CSIP					, 04, "192.168.1.1"	, d2gsconf.d2csipstr},
			{REGKEY_D2CSIP					, 10, "192.168.1.1"	, &d2gsconf.d2csip},
			{REGKEY_D2CSPORT				, 9, "6113"			, &d2gsconf.d2csport},
			{REGKEY_D2DBSIP					, 04, "192.168.1.1" , d2gsconf.d2dbsipstr},
			{REGKEY_D2DBSIP					, 10, "192.168.1.1" , &d2gsconf.d2dbsip},
			{REGKEY_D2DBSPORT				, 9, "6114"			, &d2gsconf.d2dbsport},
			{REGKEY_ENABLENTMODE			, 01, "1"			, &d2gsconf.enablentmode},
			{REGKEY_ENABLEGEPATCH			, 01, "1"			, &d2gsconf.enablegepatch},
			{REGKEY_ENABLEPRECACHEMODE		, 01, "1"			, &d2gsconf.enableprecachemode},
			{REGKEY_ENABLEGELOG				, 01, "0"			, &d2gsconf.enablegelog},
			{REGKEY_ENABLEGEMSG				, 01, "0"			, &d2gsconf.enablegemsg},
			{REGKEY_ENABLEGSLOG				, 01, "1"			, &d2gsconf.enablegslog},
			{REGKEY_DEBUGNETPACKET			, 01, "0"			, &d2gsconf.debugnetpacket},
			{REGKEY_DEBUGEVENTCALLBACK		, 01, "0"			, &d2gsconf.debugeventcallback},
			{REGKEY_IDLESLEEP				, 03, "1"			, &d2gsconf.idlesleep},
			{REGKEY_BUSYSLEEP				, 03, "1"			, &d2gsconf.busysleep},
			{REGKEY_MAXGAMES				, 03, "5"			, &d2gsconf.gsmaxgames},
			{REGKEY_MAXPREFERUSERS			, 03, "0"			, &d2gsconf.maxpreferusers},
			{REGKEY_MAXGAMELIFE				, 03, "36000"		, &d2gsconf.maxgamelife},
			{REGKEY_MOTD					, 04, "No commercial Purpose!", &d2gsconf.motd},
			{REGKEY_CHARPENDINGTIMEOUT		, 03, "200"			, &d2gsconf.charpendingtimeout},
			{REGKEY_INTERVALRECONNECTD2CS	, 03, "50"			, &d2gsconf.intervalreconnectd2cs},
			{REGKEY_D2CSSECRECT				, 04, gSomeBuffer	, d2gsconf.d2cssecrect},
			{REGKEY_MULTICPUMASK			, 03, "1"			, &d2gsconf.multicpumask},
			{REGKEY_MAX_PACKET_PER_SECOND   , 03, "300"			, &d2gsconf.maxpacketpersecond},
			{REGKEY_SERVER_CONF_FILE		, 04, "D2Server.ini", d2gsconf.serverconffile},
			{REGKEY_ADMINPWD				, 04, "9e75a42100e1b9e0b5d3873045084fae699adcb0", d2gsconf.adminpwd},
			{REGKEY_ADMINPORT				, 9, "8888"			, &d2gsconf.adminport},
			{REGKEY_ADMINTIMEOUT			, 03, "600"			, &d2gsconf.admintimeout},
			{REGKEY_AUTOUPDATE				, 01, "0"			, &gAutoUpdate.AutoUpdate},
			{REGKEY_AUTOUPDATE_TIMEOUT		, 03, "30000"		, &gAutoUpdate.AutoUpdateTimeout},
			{REGKEY_AUTOUPDATEVER			, 03, "0"			, &gAutoUpdate.AutoUpdateVer},
			{REGKEY_AUTOUPDATE_URL			, 04, gSomeBuffer	, gAutoUpdate.AutoUpdateUrl},
			{0,0,0,0}
		};

		int D2GSLoadConfigFromReg(HKEY key, LPCSTR lpSubKey, ConfigMap* confs)
		{
			HKEY		hKey;
			BOOL		result;
			u_long		ipaddr;
			DWORD		dwval;
			ConfigMap* item = 0;
			char		strbuf[256];

			result = TRUE;
			if (!RegkeyOpen(key, lpSubKey, &hKey, KEY_READ))
			{
				D2GSEventLog("D2GSReadConfig", "Can't open registry key '\\\\HKEY_LOCAL_MACHINE\\%s'", REGKEY_ROOT);
				return FALSE;
			}

			if (confs == 0)
			{
				goto tocloseregkey;
			}

			while (confs->KeyName != 0)
			{
				switch (confs->Type - 1)
				{
					// 1,3
				case 0:
				case 2:
					if (!RegkeyReadDWORD(hKey, confs->KeyName, &dwval))
					{
						D2GSEventLog("D2GSLoadRegConfig", "Can't read key '%s', set default value %s", confs->KeyName, confs->DefaultValue);
						*((DWORD*)confs->ValuePtr) = atoi(confs->DefaultValue);
					}
					else
					{
						*((DWORD*)confs->ValuePtr) = dwval;
					}
					break;
					// 2,5-8
				case 1:
				case 4:
				case 5:
				case 6:
				case 7:
					D2GSEventLog("D2GSLoadRegConfigs", "Unknow conf_type for %s", confs->KeyName);
					break;
				case 3:
					if (!RegkeyReadString(hKey, confs->KeyName, strbuf, sizeof(strbuf)))
					{
						D2GSEventLog("D2GSLoadRegConfigs", "Can't read key '%s', set default value %s", confs->KeyName, confs->DefaultValue);
						strncpy((char*)strbuf, confs->DefaultValue, 0xFF);
						strbuf[0xFF] = 0;
					}
					strcpy((char*)confs->ValuePtr, strbuf);
					break;
				case 8:
					if (!RegkeyReadDWORD(hKey, confs->KeyName, &dwval))
					{
						D2GSEventLog("D2GSLoadRegConfigs", "Can't read key '%s', set default value %s", confs->KeyName, confs->DefaultValue);
						*((WORD*)confs->ValuePtr) = (WORD)atoi(confs->DefaultValue);
					}
					else
					{
						*((WORD*)confs->ValuePtr) = (WORD)dwval;
					}
					break;
				case 9:
					if (!RegkeyReadString(hKey, confs->KeyName, strbuf, sizeof(strbuf)))
					{
						D2GSEventLog("D2GSLoadRegConfigs", "Can't read key '%s'", confs->DefaultValue);
						strncpy((char*)strbuf, confs->DefaultValue, 0xFF);
						strbuf[0xFF] = 0;
					}
					ipaddr = inet_addr(strbuf);
					if (ipaddr == INADDR_NONE)
					{
						D2GSEventLog("D2GSReadConfig", "Invalid ip address '%s'", strbuf);
						goto tocloseregkey;
					}
					*(DWORD*)(confs->ValuePtr) = ipaddr;
					break;
				default:
					D2GSEventLog("D2GSLoadRegConfigs", "Unknow conf_type for %s", confs->KeyName);
					break;
				}
				confs += 1;
			}
		tocloseregkey:
			RegkeyClose(hKey);
			return result;
		}

		int D2GSReadConfig(void)
		{
			d2gsconf.enablegslog = TRUE;
			if (!D2GSLoadConfigFromReg(HKEY_LOCAL_MACHINE, REGKEY_ROOT, gConfigMap))
			{
				D2GSEventLog("D2GSReadConfig", "failed loading configurations from registry");
				return 0;
			}

			d2gsconf.d2dbsport = htons(d2gsconf.d2dbsport);
			d2gsconf.d2csport = htons(d2gsconf.d2csport);
			d2gsconf.adminport = htons(d2gsconf.adminport);
			d2gsconf.gemaxgames = 2 * d2gsconf.gsmaxgames + 0xc8;
			d2gsconf.curgsmaxgames = 0;
			return TRUE;
		} /* End of D2GSReadConfig() */


		/*********************************************************************
		 * Purpose: to open an existing registry key
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int RegkeyOpen(HKEY hKeyRoot, LPCTSTR lpSubKey, PHKEY hKey, REGSAM sam)
		{
			if (RegOpenKeyEx(hKeyRoot, lpSubKey, 0, sam, hKey) == ERROR_SUCCESS)
				return TRUE;
			else
				return FALSE;

		} /* End of RegkeyOpen() */


		/*********************************************************************
		 * Purpose: to close an opened registry key
		 * Return: none
		 *********************************************************************/
		void RegkeyClose(HKEY hKey)
		{
			if (hKey)
				RegCloseKey(hKey);
			return;

		} /* End of RegkeyClose() */


		/*********************************************************************
		 * Purpose: to read a string key to the buffer
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int RegkeyReadString(HKEY hKey, LPCTSTR name, char* buf, DWORD buflen)
		{
			DWORD	dwType, dwLen;
			LONG	lReturn;

			if ((!hKey) || (!name))
				return FALSE;
			dwLen = buflen;
			ZeroMemory(buf, buflen);
			lReturn = RegQueryValueEx(hKey, name, NULL, &dwType, (LPBYTE)buf, &dwLen);
			if (lReturn == ERROR_SUCCESS)
			{
				*(buf + buflen - 1) = 0;
				return TRUE;
			}
			else
				return FALSE;

		} /* End of RegkeyReadString() */


		/*********************************************************************
		 * Purpose: to read a dword key to the variable
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int RegkeyReadDWORD(HKEY hKey, LPCTSTR name, DWORD* val)
		{
			DWORD	dwType, dwLen, dwVal;
			LONG	lReturn;

			if ((!hKey) || (!name))
				return FALSE;
			dwLen = sizeof(dwVal);
			lReturn = RegQueryValueEx(hKey, name, NULL, &dwType, (LPBYTE)&dwVal, &dwLen);
			if (lReturn == ERROR_SUCCESS)
			{
				*val = dwVal;
				return TRUE;
			}
			else
				return FALSE;

		} /* End of RegkeyReadDWORD() */


		/*********************************************************************
		 * Purpose: to write a string key from the buffer
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int RegkeyWriteString(HKEY hKey, LPCTSTR name, LPCSTR buf)
		{
			DWORD	dwLen;
			LONG	lReturn;

			if ((!hKey) || (!name))	return FALSE;
			dwLen = strlen(buf) + 1;
			lReturn = RegSetValueEx(hKey, name, 0L, REG_SZ, (const BYTE*)buf, dwLen);
			if (lReturn == ERROR_SUCCESS)
				return TRUE;
			else
				return FALSE;

		} /* End of RegkeyWriteString() */


		/*********************************************************************
		 * Purpose: to write a dword key from the variable
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int RegkeyWriteDWORD(HKEY hKey, LPCTSTR name, DWORD val)
		{
			DWORD	dwVal, dwLen;
			LONG	lReturn;

			if ((!hKey) || (!name))	return FALSE;
			dwVal = val;
			dwLen = sizeof(dwVal);
			lReturn = RegSetValueEx(hKey, name, 0L, REG_DWORD, (const BYTE*)&dwVal, dwLen);
			if (lReturn == ERROR_SUCCESS)
				return TRUE;
			else
				return FALSE;

		} /* End of RegkeyWriteDWORD() */


		/*********************************************************************
		 * Purpose: to set the string value to the registry
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetConfigDWORD(LPCSTR keyname, DWORD dwVal)
		{
			HKEY		hKey;

			if (!RegkeyOpen(HKEY_LOCAL_MACHINE, REGKEY_ROOT, &hKey, KEY_SET_VALUE))
				return FALSE;
			if (!RegkeyWriteDWORD(hKey, keyname, dwVal))
			{
				RegkeyClose(hKey);
				return FALSE;
			}
			else
			{
				RegkeyClose(hKey);
				return TRUE;
			}

		} /* End of D2GSSetConfigDWORD() */


		/*********************************************************************
		 * Purpose: to set the string value to the registry
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetConfigString(LPCSTR keyname, LPCSTR str)
		{
			HKEY		hKey;

			if (!RegkeyOpen(HKEY_LOCAL_MACHINE, REGKEY_ROOT, &hKey, KEY_SET_VALUE))
				return FALSE;
			if (!RegkeyWriteString(hKey, keyname, str))
			{
				RegkeyClose(hKey);
				return FALSE;
			}
			else
			{
				RegkeyClose(hKey);
				return TRUE;
			}

		} /* End of D2GSSetConfigString() */


		/*********************************************************************
		 * Purpose: to set the MaxGameLife
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetMaxGameLife(DWORD maxgamelife)
		{
			d2gsconf.maxgamelife = maxgamelife;
			return D2GSSetConfigDWORD(REGKEY_MAXGAMELIFE, maxgamelife);

		} /* End of D2GSSetMaxGameLife() */


		/*********************************************************************
		 * Purpose: to set the AdminPadssword
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetAdminPassword(LPCSTR password)
		{
			return D2GSSetConfigString(REGKEY_ADMINPWD, password);

		} /* End of D2GSSetAdminPassword() */


		/*********************************************************************
		 * Purpose: to set the MaxGames
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetMaxGames(DWORD maxgames)
		{
			d2gsconf.gsmaxgames = maxgames;
			return D2GSSetConfigDWORD(REGKEY_MAXGAMES, maxgames);

		} /* End of D2GSSetMaxGameLife() */

		/*********************************************************************
		 * Purpose: to set the Max Prefer Users
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSSetMaxPreferUsers(DWORD maxusers)
		{
			d2gsconf.maxpreferusers = maxusers;
			return D2GSSetConfigDWORD(REGKEY_MAXPREFERUSERS, maxusers);

		} /* End of D2GSSetMaxPreferUsers() */

		/*********************************************************************
		 * Purpose: set multi cpu mask
		 * Return: NONE
		 *********************************************************************/
		DWORD D2GSSetMultiCpuMask(int mask)
		{
			DWORD ret = D2GSSetConfigDWORD(REGKEY_MULTICPUMASK, mask);
			if (ret)
			{
				d2gsconf.multicpumask = mask;
			}
			return ret;
		}

		/*********************************************************************
		 * Purpose: set enable gs log
		 * Return: NONE
		 *********************************************************************/
		DWORD D2GSSetEnableGSLog(int enable)
		{
			DWORD ret = D2GSSetConfigDWORD(REGKEY_ENABLEGSLOG, enable);
			if (ret)
			{
				d2gsconf.enablegslog = enable;
			}
			return ret;
		}

		/*********************************************************************
		 * Purpose: set autoupdate
		 * Return: NONE
		 *********************************************************************/
		void D2GSSetAutoUpdate(int enable)
		{
			gAutoUpdate.AutoUpdate = enable;
			D2GSSetConfigDWORD(REGKEY_AUTOUPDATE, enable);
		}

		/*********************************************************************
		 * Purpose: set autoupdatever
		 * Return: NONE
		 *********************************************************************/
		void D2GSSetAutoUpdateVer(int ver)
		{
			gAutoUpdate.AutoUpdateVer = ver;
			D2GSSetConfigDWORD(REGKEY_AUTOUPDATEVER, ver);
		}

		/*********************************************************************
		 * Purpose: set autoupdatetimeout
		 * Return: NONE
		 *********************************************************************/
		void D2GSSetAutoUpdateTimeout(int timeout)
		{
			gAutoUpdate.AutoUpdateTimeout = timeout;
			D2GSSetConfigDWORD(REGKEY_AUTOUPDATE_TIMEOUT, timeout);
		}

		/*********************************************************************
		 * Purpose: set autoupdateurl
		 * Return: NONE
		 *********************************************************************/
		void D2GSSetAutoUpdateUrl(const char* url)
		{
			strcpy(gAutoUpdate.AutoUpdateUrl, url);
			D2GSSetConfigString(REGKEY_AUTOUPDATE_URL, url);
		}

	}

}