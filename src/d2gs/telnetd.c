#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "psapi.h"
#include <d2server.h>
#include "d2gs.h"
#include "eventlog.h"
#include "config.h"
#include "net.h"
#include "d2gamelist.h"
#include "handle_s2s.h"
#include "vars.h"
#include "charlist.h"
#include "telnetd.h"
#include "bnethash.h"
#include "utils.h"


extern void D2GSSetD2CSMaxGameNumber(DWORD maxgamenum);


/* command table */
static ADMINCOMMAND	admincmdtbl[] = {
	{"help", 	FALSE,	admin_help,				"",			"Show this message."},
	{"status",	FALSE,	admin_getstatus,		"",			"Get status of this GS."},
	{"gl",		FALSE,	admin_show_game_list,	"",			"Show active game list."},
	{"cl",		FALSE,	admin_show_char_in_game,"GameId",	"Show char list in a game."},
	{"char",	FALSE,	admin_getcharinfo,		"CharName", "Get char information on this GS."},
	{"kick",	FALSE,	admin_kick_user,		"CharName",	"Kick an user out of the game."},
	{"msg",		FALSE,	admin_msg,				"MsgType Target Message", "Send message"},
	{"disable",	FALSE,	admin_disablegame,		"GameId",	"Deny joining the specified game."},
	{"enable",	FALSE,	admin_enablegame,		"GameId",	"Allow joining the specified game."},
	{"maxgame",	FALSE,	admin_setmaxgame,		"MAXNUM",	"Change maximum game number."},
	{"maxlife",	FALSE,	admin_setmaxgamelife,	"MaxLife",	"To set the maximum game life in seconds."},
	{"maxuser", FALSE,	admin_setmaxpreferusers,"MaxPreferUsers", "To set the maximum prefer users in this gs."},
	{"uptime",	FALSE,	admin_uptime,			"",			"When the system startup."},
	{"version", FALSE,	admin_getversion,		"",			"To show version informations."},
	{"ver",		FALSE,	admin_getversion,		"",			"To show version informations."},
	{"passwd",	FALSE,	admin_chgpasswd,		"",			"To change the login password."},
	{"gslog",	FALSE,	admin_enablegslog,		"",			"To enable or disable GS log to file."},
	{"restart",	FALSE,	admin_restart,			"[seconds]","To restart this GS."},
	{"shutdown",FALSE,	admin_shutdown,			"[seconds]","To shutdown this GS."},
	{"setmotd",	FALSE,	admin_setmotd,			"MOTD",		"To set the Message of The Day"},
	{"setcpumask",	FALSE,	admin_setcpumask,	"CPUMASK",	"To set process CPU mask, used in SMP machine"},
	{"reload",	FALSE,	admin_reloadconf,		"",			"To reload d2gs config file"},
	{"we",		FALSE,	admin_showwe,			""			"Show world event setting and status"},
	//{"au",	FALSE,	admin_setautoupdate,	"[enable|disable|timeout|ver|url] [value].","To show/control autoupdate status."},
	{"exit",	FALSE,	NULL,					"",			"Exit this administration console."},
	{NULL, TRUE, NULL, NULL}
};


static HANDLE	hStopEvent;
static HANDLE	hAdminThread;
static long		uptime;


/*********************************************************************
 * Purpose: to initialize the admin console
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int D2GSAdminInitialize(void)
{
	DWORD		dwThreadId;

	/* system start time */
	uptime = time(NULL);

	/* create stop event */
	hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hStopEvent) {
		D2GSEventLog("D2GSAdminInitialize",
			"Failed in creating event object. Code: %lu", GetLastError());
		return FALSE;
	}

	/* create thread to process net event */
	hAdminThread = CreateThread(NULL, 0, admin_service, NULL, 0, &dwThreadId);
	if (!hAdminThread) {
		D2GSEventLog("D2GSAdminInitialize",
			"Can't CreateThread admin_service. Code: %lu", GetLastError());
		CleanupRoutineForAdmin();
		return FALSE;
	}
	CloseHandle(hAdminThread);
	hAdminThread = NULL;

	/* add to the cleanup routine list */
	if (CleanupRoutineInsert(CleanupRoutineForAdmin, "Administrator Console")) {
		return TRUE;
	} else {
		/* do some cleanup before quiting */
		CleanupRoutineForAdmin();
		return FALSE;
	}

} /* End of D2GSAdminInitialize() */


/*********************************************************************
 * Purpose: to shutdown the admin server
 * Return: TRUE(success) or FALSE(failed)
 *********************************************************************/
int CleanupRoutineForAdmin(void)
{
	if (hStopEvent) {
		SetEvent(hStopEvent);
		if (hAdminThread) {
			WaitForSingleObject(hAdminThread, INFINITE);
			CloseHandle(hAdminThread);
			hAdminThread = NULL;
		}
		Sleep(1000);
		CloseHandle(hStopEvent);
		hStopEvent = NULL;
	}

	return TRUE;

} /* End of CleanupRoutineForAdmin() */


/**********************************************************
 * Function: admin_service
 * Purpose: administration service thread
 * Return: None
 ************************************************************/
DWORD WINAPI admin_service(LPVOID lpParam)
{
	unsigned int		mainsock, ns;
	struct sockaddr_in	server, client;
	int					clen;
	char				ipstr[16];
	int					val;
	struct in_addr		temp;
	struct timeval		tv;
	DWORD				ipaddr;
	HANDLE				hThread;
	DWORD				dwThreadId;

	/* create our new socket for administraion */
	if ((mainsock=socket(AF_INET,SOCK_STREAM,0))<0)	{
		D2GSEventLog("admin_service",
			"failed create admin socket. code: %d", WSAGetLastError());
		return -1;
	}
	val = 1;
	clen = sizeof(val);
	setsockopt(mainsock, SOL_SOCKET, SO_REUSEADDR, (char *)&val, clen);
	ZeroMemory(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = d2gsconf.adminport;
	if (bind(mainsock, (struct sockaddr *)&server, sizeof(server))<0) {
		closesocket(mainsock);
		mainsock = -1;
		D2GSEventLog("admin_service",
			"failed bind admin socket, port %u. code: %d", d2gsconf.adminport, WSAGetLastError());
		return -1;
	}
	if (listen(mainsock,1)==-1) {
		closesocket(mainsock);
		mainsock = -1;
		D2GSEventLog("admin_service",
			"failed listen admin socket, port %u. code: %d", d2gsconf.adminport, WSAGetLastError());
		return -1;
	}

	/* admin service ok, loop for service request */
	while(1)
	{
		memset(&client, 0, sizeof(client));
		clen = sizeof(client);
		ns = accept(mainsock, (struct sockaddr *)&client, &clen);
		if (ns<0) {
			D2GSEventLog("admin_service",
				"failed accept for admin socket. code: %d", WSAGetLastError());
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);
			continue;
		}
		temp.s_addr = client.sin_addr.s_addr;
		ipaddr = ntohl(temp.s_addr);
		ZeroMemory(ipstr, sizeof(ipstr));
		strncpy(ipstr, inet_ntoa(temp), sizeof(ipstr)-1);

		if (ipaddr==0) {
			closesocket(ns);
			continue;
		}

		/* check if user comes from authorized IP address? */
		/*
		if (((ipaddr && 0xffff0000) != 0xa66f0000) && ipaddr != 0x7f000001) {
			SENDSTR(ns, "Access deny");
			closesocket(ns);
			continue;
		}
		*/

		/* It is ok now, create new thread to server it */
		D2GSEventLog("admin_service", "New admin request from %s(%u)", ipstr, ns);
		hThread = CreateThread(NULL, 0, admin_thread, &ns, 0, &dwThreadId);
		if (!hThread) {
			SENDSTR(ns, "System error");
			closesocket(ns);
			continue;
		} 
		else
			CloseHandle(hThread);
	} /* End of while(1) */

} /* End of admin_service() */


/**********************************************************
 * Function: admin_thread
 * Purpose: administration service thread
 * Return: None
 ************************************************************/
DWORD WINAPI admin_thread(LPVOID lpParam)
{
	unsigned int		ns;
	unsigned char		buf[1024], recvbuf[2048];
	unsigned char		cmd[4][256];
	int					count, i;

	/* get the socket description */
	if (!lpParam) return -1;
	else
		ns = *((int *)lpParam);

	/* telnet option specifications */
	sprintf(buf, "%c%c%c%c%c%c", 
			TC_IAC, TC_WILL, TC_NOGA,	/* enable Go-Ahead option */
			TC_IAC, TC_WILL, TC_ECHO);	/* enable Echo */
	SENDSTR(ns, buf);
	/* show logo info */
	admin_logo(ns);

	/* checking out password */
	count = 3;
	while(count>0)
	{
		strcpy(buf, "Password: ");
		SENDSTR(ns, buf);
		if (get_cmd_line(ns, recvbuf, 0)<0)	{
			count = 0;
			break;
		}
		i = admin_check_pass(recvbuf);
		if (i<0) {
			strcpy(buf, "Sorry!\r\n");
			SENDSTR(ns, buf);
			count--;
		} else if (i==0) {
			SENDSTR(ns, "NULL password! For security, please set a password!!\r\n");
			break;
		} else
			break;
		if (admin_to_stop()) {
			closesocket(ns);
			return 0;
		}
	}
	if (admin_to_stop()) {
		closesocket(ns);
		return 0;
	}
	if (count==0) {
		/* retry for 3 times */
		closesocket(ns);
		return 0;
	}
	strcpy(buf, "\r\n\r\n~~~ Welcome to Diablo II Close Game Server Administration Console ~~~\r\n\r\n");
	SENDSTR(ns, buf);

	/* if no password, tell to set one */
	if (i==0) {
		strcpy(buf, "No password set!! For security, go to set one!\r\n\r\n");
		SENDSTR(ns, buf);
	}

	/* entering loop wating for command */
	while(1)
	{
		strcpy(buf, "D2GS> ");
		SENDSTR(ns, buf);
		if ((count=get_cmd_line(ns, recvbuf, 1))<0) {
			strcpy(buf, "\r\n\r\nTimeout!!\r\n\r\n");
			SENDSTR(ns, buf);
			break;
		} 
		if (admin_to_stop()) break;
		if (count==0) continue;	// an empty line
		recvbuf[255] = 0;
		if (admin_analyse_cmd(recvbuf, (u_char *)cmd, sizeof(cmd)/sizeof(cmd[0]), sizeof(cmd[0]))==0) {
			sprintf(buf, "'%s'\r\n", recvbuf);
			strcat(buf, "Unknown command, please type \"help\" to get help.\r\n");
			SENDSTR(ns, buf);
			continue;
		}

		/* to find which command it is */
		i = 0;
		while(admincmdtbl[i].keyword != NULL)
		{
			if (strcmp(cmd[0], admincmdtbl[i].keyword) != 0) {
				i++;
				continue;
			}
			if (admincmdtbl[i].adminfunc == NULL) {
				/* quit admin this session */
				strcpy(buf, "\r\nBye!\r\n");
				SENDSTR(ns, buf);
				i = 99999;
			} else {
				admincmdtbl[i].adminfunc(ns, cmd[1]);
			}
			break;
		}
		if (i == 99999)
			break;
		else if (admincmdtbl[i].keyword == NULL) {
			sprintf(buf, "'%s'\r\n", recvbuf);
			strcat(buf, "Unknown command, please type \"help\" to get help.\r\n");
			SENDSTR(ns, buf);
		}

	} /* loop waiting for commnad */

	/* end of this service, close */
	closesocket(ns);
	return 0;

} /* End of admin_thread() */


/**********************************************************
 * Function: admin_analyse_cmd
 * Purpose: to analyse the command line
 * Return:  0(failed), >0(success)
 ************************************************************/
int admin_analyse_cmd(u_char *buf, u_char *cmd, int x, int y)
{
	u_char		*ptr, *pstr;

	ptr = buf;
	while(*ptr==' ' || *ptr=='\t') ptr++;
	pstr = ptr;
	while(*ptr!=' ' && *ptr!='\t' && *ptr!='\0') ptr++;
	*ptr = '\0';
	strcpy(cmd, pstr);
	strcpy(cmd+y, ptr+1);

	return 1;

} /* End of admin_analyse_cmd() */


/**********************************************************
 * Function: admin_to_stop
 * Purpose: to check if the hStopEvent is set
 * Return:  0(not set), >0(set)
 ************************************************************/
int admin_to_stop(void)
{
	if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0)
		return 1;
	else
		return 0;

} /* End of admin_to_stop() */


 /**********************************************************
 * Function: get_cmd_line
 * Purpose: to get a command line from admin client
 * Param:   ns - socket desc
 *          buf - command line buffer
 *          flag - 0(no echo), 1(echo back)
 * Return:  <=0  failed
 *          >0 success, it is the char number received
 ************************************************************/
int get_cmd_line(unsigned int ns, unsigned char *buf, int flag)
{
	unsigned char	mybuf[256], *ptr, *p, tmp[16];
	fd_set			rd;
	int				count, bytes, i;
	struct timeval	tv;
	DWORD			timeoutcount;

	ptr = buf;
	count = 0;
	timeoutcount = 0;
	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(ns, &rd);
		tv.tv_sec = ADMIN_SESSION_TIMEOUT_UNIT;
		tv.tv_usec = 0;
		i = select(ns+1, &rd, NULL, NULL, &tv);
		if (i<0) return -1;
		if (i==0) {
			timeoutcount++;
			if (timeoutcount>=d2gsconf.admintimeout) return -1;
			continue;
		}
		if (admin_to_stop()) return -1;
		if ((bytes=recv(ns, mybuf, sizeof(mybuf), 0))<=0)
			return -1;
		timeoutcount = 0;
		*(mybuf+bytes) = '\0';
		p = mybuf;
		while(p<(mybuf+bytes))
		{
			if ((*p=='\r')||(*p=='\n')||(*p=='\0'))	{
				*ptr = '\0';
				strcpy(mybuf, "\r\n");
				if (flag) SENDSTR(ns, mybuf);
				return count;
			} else if (*p==TC_IAC) {
				p += 3;
			} else if ((*p==8)||(*p==127)) {		/* backspace */
				if (count>0) {
					sprintf(tmp, "%c %c", 8, 8);
					if (flag)
						SENDSTR(ns, tmp);
					*(--ptr) = '\0';
					count--;
				}
				p++;
			} else if ((*p<' ')||(*p>126)) {
				p++;
			} else {
				if (flag) SENDCHAR(ns, p);
				if (count<255) {
					*(ptr++) = *(p++);
					count++;
				}
			}
		}
	} /* while(1) */

} /* End of get_cmd_line() */


/**********************************************************
 * Function: admin_getchar
 * Purpose: to get a character from the client
 * Param:   ns - socket desc
 * Return:  <=0  failed
 *          >0 success, the character
 ************************************************************/
int admin_getchar(unsigned int ns)
{
	unsigned char	buf[8], *p;
	fd_set			rd;
	int				i;
	struct timeval	tv;
	DWORD			timeoutcount;

	timeoutcount = 0;
	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(ns, &rd);
		tv.tv_sec = ADMIN_SESSION_TIMEOUT_UNIT;
		tv.tv_usec = 0;
		i = select(ns+1, &rd, NULL, NULL, &tv);
		if (i<0) return -1;
		if (i==0) {
			timeoutcount++;
			if (timeoutcount>=d2gsconf.admintimeout) return -1;
			continue;
		}
		if (admin_to_stop()) return -1;
		if (recv(ns, buf, 1, 0)!=1)
			return -1;
		p = buf;
		if ((*p=='\r')||(*p=='\n')) {
			i = 13;		/* the return value */
			break;
		} else if (*p==27) {
			i = 27;
			break;
		} else if ((*p==8)||(*p==127)) {	/* backspace */
			i = 8;
			break;
		} else if ((*p>=' ')&&(*p<=126)) {
			i = *p;
			break;
		}

	} /* while(1) */

	return i;

} /* End of admin_getchar() */


/**********************************************************
 * Function: admin_check_pass
 * Purpose: to check the passowrd inputed
 * Param:   pass - user input password
 * Return:  < 0  failed
 *          = 0  success, and the password is null, need to be change
 *          > 0  success
 ************************************************************/
int admin_check_pass(unsigned char *pass)
{
	t_hash		hash;
	char const	*phash;

	if (!pass) return -1;

	if ((pass[0]=='\0') && (d2gsconf.adminpwd[0]=='\0'))
		return 0;
	if (bnet_hash(&hash, strlen(pass), pass) != 0)
		return -1;
	phash = hash_get_str(hash);
	if (phash==NULL)
		return -1;
	if (strcmp(phash, d2gsconf.adminpwd)==0)
		return 1;
	else
		return -1;

} /* End of admin_check_pass() */


/**********************************************************
 * Function: admin_logo
 * Purpose: to show logo infomation
 * Return: None
 ************************************************************/
void admin_logo(unsigned int ns)
{
	unsigned char	buf[512];

	sprintf(buf, "%s%s%s%s",
			"\r\nDiablo II Close Game Server Administration Console\r\n",
			"Win32 Version ", D2GS_VERSION_STRING_ONLY, "\r\n\r\n");
	SENDSTR(ns, buf);

} /* End of admin_logo() */


/**********************************************************
 * Function: admin_help
 * Purpose: administration service help manual
 * Return: None
 ************************************************************/
void admin_help(unsigned int ns, u_char *param)
{
	unsigned char	buf[256];
	int				i;

	memset(buf, 0, sizeof(buf));
	strcat(buf, "\r\nCommands help£º\r\n\r\n");
	SENDSTR(ns, buf);
	i = 0;
	while(admincmdtbl[i].keyword != NULL)
	{
		if (!admincmdtbl[i].disable)
		{
			sprintf(buf, "%s %s\r\n  %s\r\n",
				admincmdtbl[i].keyword, admincmdtbl[i].param, admincmdtbl[i].annotation);
			SENDSTR(ns, buf);
		}
		i++;
	}
	strcpy(buf, "\r\n");
	SENDSTR(ns, buf);
	return;

} /* End of admin_help() */


/**********************************************************
 * Function: admin_chgpasswd
 * Return: None
 ************************************************************/
void admin_chgpasswd(unsigned int ns, u_char *param)
{
	unsigned char	buf[256];
	unsigned char	newpass1[32], newpass2[32];
	int				count;
	t_hash			hash;
	char const		*ptr;

	/* check old password */
	SENDSTR(ns, "Changing login password:\r\n");
	SENDSTR(ns, "current password: ");
	if (get_cmd_line(ns, buf, 0)<0)
		return;
	if (admin_check_pass(buf)<=0) {
		SENDSTR(ns, "\r\nUnauthorized operation!\r\n");
		return;
	}

	/* input new password */
	for(count=3; count>0; count--)
	{
		SENDSTR(ns, "\r\nnew password: ");
		if (get_cmd_line(ns, buf, 0)<0)
			return;
		memset(newpass1, 0, sizeof(newpass1));
		strncpy(newpass1, buf, sizeof(newpass1)-1);
		SENDSTR(ns, "\r\nconfirm password: ");
		if (get_cmd_line(ns, buf, 0)<0)
			return;
		memset(newpass2, 0, sizeof(newpass2));
		strncpy(newpass2, buf, sizeof(newpass2)-1);
		if (strcmp(newpass1, newpass2)==0) {
			if (*newpass1=='\0') {
				SENDSTR(ns, "\r\nSorry, password can't be null!\r\n");
				continue;
			} else
				break;	/* can change the password now */
		} else {
			SENDSTR(ns, "\r\nPassword mismatched!\r\n");
			continue;
		}
	}
	if (count==0) {
		SENDSTR(ns, "\r\nRetry for too many times, sorry!\r\n\r\n");
		return;
	}

	/* save the new password */
	SENDSTR(ns, "\r\n");
	if (bnet_hash(&hash, strlen(newpass1), newpass1)!=0) {
		SENDSTR(ns, "Internal error!\r\n");
		return;
	}
	ptr = hash_get_str(hash);
	if (ptr==NULL) {
		SENDSTR(ns, "Internal error2!\r\n");
		return;
	}
	if (D2GSSetConfigString(REGKEY_ADMINPWD, ptr)) {
		SENDSTR(ns, "Password change successfully!\r\n");
		strcpy(d2gsconf.adminpwd, ptr);
		D2GSEventLog("admin_chgpasswd", "Password changed by admin %u", ns);
	}
	else
		SENDSTR(ns, "Failed saving new password!\r\n");
	return;

} /* End of admin_chgpasswd() */


/**********************************************************
 * Function: admin_show_game_list
 * Return: None
 ************************************************************/
void admin_show_game_list(unsigned int ns, u_char *param)
{
	D2GSShowGameList(ns);
	return;

} /* End of admin_show_game_list() */


/**********************************************************
 * Function: admin_show_char_in_game
 * Return: None
 ************************************************************/
void admin_show_char_in_game(unsigned int ns, u_char *param)
{
	WORD	GameId;

	if (!param) return;
	GameId = (WORD)atoi(param);
	if (GameId==0) {
		SENDSTR(ns, "Invalid game id\r\n\r\n");
		return;
	}
	D2GSShowCharInGame(ns, GameId);
	return;

} /* End of admin_show_char_in_game() */


void admin_setcpumask(unsigned int ns, u_char* param)
{
	DWORD mask = 0;
	unsigned char buf[256];
	if (!param) return;
	mask = atoi(param);
	if (SetThreadAffinityMask(hGEThread, mask) == 0)
	{
		sprintf(buf, "Failed setting CPUMASK, code: %lx\r\n", GetLastError());
		SENDSTR(ns, buf);
		return;
	}
	if (D2GSSetMultiCpuMask(mask) == 0)
	{
		SENDSTR(ns, "Can't write CPUMASK to registry\r\n");
		return;
	}
	sprintf(buf, "Set CPUMASK to 0x%lx.\r\n\r\n", mask);
	SENDSTR(ns, buf);
	return;
} /* End of admin_setcpumask() */


/**********************************************************
 * Function: admin_restart
 * Return: None
 ************************************************************/
void admin_restart(unsigned int ns, u_char *param)
{
	unsigned int	delay;

	if (!param) return;

	if (stricmp(param, "force") == 0)
	{
		SENDSTR(ns, "Force restarting Diablo II Close Game Server!\r\n");
		d2gsconf.enablegslog = TRUE;
		D2GSEventLog("admin_restart", "force restart d2gs by admin %u", ns);
		d2gsconf.enablegslog = FALSE;
		D2GSBeforeShutdown(1, 0);
		return;
	}
	else
	{
		delay = atoi(param);
		if (delay == 0)
		{
			delay = DEFAULT_GS_SHUTDOWN_DELAY;
		}
	}

	SENDSTR(ns, "Restart Diablo II Close Game Server in progress!");
	d2gsconf.enablegslog = TRUE;
	D2GSEventLog("admin_shutdown", "shutdown d2gs by admin %u in %lu second", ns, delay);
	d2gsconf.enablegslog = FALSE;
	D2GSBeforeShutdown(1, delay);

} /* End of admin_restart() */


/**********************************************************
 * Function: admin_shutdown
 * Return: None
 ************************************************************/
void admin_shutdown(unsigned int ns, u_char *param)
{
	unsigned int	delay;

	if (!param) return;

	if (stricmp(param, "force") == 0)
	{
		SENDSTR(ns, "Force shutdown Diablo II Close Game Server!\r\n");
		d2gsconf.enablegslog = TRUE;
		D2GSEventLog("admin_shutdown", "force shutdown d2gs by admin %u", ns);
		d2gsconf.enablegslog = FALSE;
		D2GSBeforeShutdown(2, 0);
		return;
	}

	delay = atoi(param);
	if (delay == 0)
	{
		delay = DEFAULT_GS_SHUTDOWN_DELAY;
	}

	SENDSTR(ns, "Shutdown Diablo II Close Game Server in progress!\r\n");
	d2gsconf.enablegslog = TRUE;
	D2GSEventLog("admin_shutdown", "shutdown d2gs by admin %u in %lu seconds", ns, delay);
	d2gsconf.enablegslog = FALSE;
	D2GSBeforeShutdown(2, delay);
} /* End of admin_shutdown() */


/**********************************************************
 * Function: admin_reloadconf
 * Return: None
 ************************************************************/
void admin_reloadconf(unsigned int ns, u_char* param)
{
	D2GEReloadConfig();
	SENDSTR(ns, "D2GE config file reloaded.\r\n");
	return;

} /* End of admin_reloadconf() */


/**********************************************************
 * Function: admin_uptime
 * Return: None
 ************************************************************/
void admin_uptime(unsigned int ns, u_char *param)
{
	char			buf[256];
	long			now, interval;
	struct tm		*tm;

	now = time(NULL);
	interval = now-uptime;
	tm = localtime(&uptime);
	strftime(buf, sizeof(buf), "The game server started at %m-%d %H:%M:%S\r\n", tm);
	SENDSTR(ns, buf);
	tm = gmtime(&interval);
	//strftime(buf, sizeof(buf), "uptime %d days %H hours %M minutes %S seconds\r\n", tm);
	_snprintf(buf, sizeof(buf), "uptime %d days %d hours %d minutes %d seconds\r\n",
			tm->tm_yday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	SENDSTR(ns, buf);
	tm = localtime(&now);
	strftime(buf, sizeof(buf), "Now it is %m-%d %H:%M:%S\r\n", tm);
	SENDSTR(ns, buf);
	SENDSTR(ns, "\r\n");
	return;

} /* End of admin_uptime() */


/**********************************************************
 * Function: admin_setmaxgame
 * Return: None
 ************************************************************/
void admin_setmaxgame(unsigned int ns, u_char *param)
{
	unsigned char	buf[256];
	DWORD			maxgamenum;
	DWORD shutdownStatus = 0;

	if (!param) return;
	maxgamenum = (DWORD)atoi(param);
	shutdownStatus = D2GSGetShutdownStatus();
	if (shutdownStatus != 0 && maxgamenum != 0)
	{
		switch (shutdownStatus - 1)
		{
		case 0:
			SENDSTR(ns, "The GS is in restarting progress!\r\n");
			break;
		case 1:
			SENDSTR(ns, "The GS is in shutting down progress!\r\n");
			break;
		case 2:
			SENDSTR(ns, "The GS is in D2CS restarting progress!\r\n");
			break;
		case 3:
			SENDSTR(ns, "The GS is in D2CS shutting down progres!\r\n");
			break;
		default:
			break;
		}
		return;
	}
	if (maxgamenum>d2gsconf.gemaxgames) {
		sprintf(buf, "Maximum game number must be in range 0-%lu\r\n", d2gsconf.gemaxgames);
		SENDSTR(ns, buf);
	} else {
		D2GSSetD2CSMaxGameNumber(maxgamenum);
		sprintf(buf, "Set maximum game number to %lu\r\n", maxgamenum);
		SENDSTR(ns, buf);
		if (D2GSSetMaxGames(maxgamenum))
		{
			sprintf(buf, "MaxGame set to registry\r\n");
			D2GSEventLog("admin_setmaxgame",
				"Change max game number to %lu by admin %u", maxgamenum, ns);
			SENDSTR(ns, buf);
			D2GSSetD2CSMaxGameNumber(maxgamenum);
		}
		else
			sprintf(buf, "Set registry failed\r\n");
		D2GSEventLog("admin_setmaxgame",
				"Change max game number to %lu by admin %u", maxgamenum, ns);
		SENDSTR(ns, buf);
	}
	SENDSTR(ns, "\r\n");
	return;

} /* End of admin_setmaxgame() */


/**********************************************************
 * Function: admin_disablegame
 * Return: None
 ************************************************************/
void admin_disablegame(unsigned int ns, u_char *param)
{
	WORD	GameId;

	if (!param) return;
	GameId = (WORD)atoi(param);
	if (GameId==0) {
		SENDSTR(ns, "Invalid game id\r\n\r\n");
		return;
	}
	D2GSDisableGameByGameId(ns, GameId);
	return;

} /* End of admin_disablegame() */


/**********************************************************
 * Function: admin_enablegame
 * Return: None
 ************************************************************/
void admin_enablegame(unsigned int ns, u_char *param)
{
	WORD	GameId;

	if (!param) return;
	GameId = (WORD)atoi(param);
	if (GameId==0) {
		SENDSTR(ns, "Invalid game id\r\n\r\n");
		return;
	}
	D2GSEnableGameByGameId(ns, GameId);
	return;

} /* End of admin_enablegame() */


/**********************************************************
 * Function: admin_enablegslog
 * Return: None
 ************************************************************/
void admin_enablegslog(unsigned int ns, u_char* param)
{
	BOOL Enable;

	if (!param) return;

	Enable = (WORD)atoi(param);

	if (D2GSSetEnableGSLog(Enable))
	{
		SENDSTR(ns, "Done.\r\n\r\n");
		return;
	}
	SENDSTR(ns, "Can't write ENABLEGSLOG to registry\r\n");
	return;

} /* End of admin_enablegslog() */


/**********************************************************
 * Function: admin_setmaxgamelife
 * Return: None
 ************************************************************/
void admin_setmaxgamelife(unsigned int ns, u_char *param)
{
	DWORD	gamelife;

	if (!param) return;
	gamelife = (DWORD)atoi(param);
	if (gamelife==0) {
		SENDSTR(ns, "Invalid value\r\n\r\n");
		return;
	}
	if (D2GSSetMaxGameLife(gamelife))
		SENDSTR(ns, "Done.\r\n\r\n");
	else
		SENDSTR(ns, "Can't write MAXGAMELIFE to registry\r\n");
	return;

} /* End of admin_setmaxgamelife() */


/**********************************************************
 * Function: admin_setmaxpreferuses
 * Return: None
 ************************************************************/
void admin_setmaxpreferusers(unsigned int ns, u_char* param)
{
	DWORD	maxusers;

	if (!param) return;
	maxusers = (DWORD)atoi(param);
	if (maxusers == 0)
	{
		SENDSTR(ns, "Invalid value\r\n\r\n");
		return;
	}
	if (D2GSSetMaxPreferUsers(maxusers))
		SENDSTR(ns, "Done.\r\n\r\n");
	else
		SENDSTR(ns, "Can't write MAXPREFERUSER to registry\r\n");
	return;

} /* End of admin_setmaxpreferusers() */


/**********************************************************
 * Function: admin_version
 * Return: None
 ************************************************************/
void admin_getversion(unsigned int ns, u_char *param)
{
	unsigned char	buf[1024];

	sprintf(buf, "Checksum: 0x%08X\r\n", d2gsconf.checksum);
	SENDSTR(ns, buf);
	sprintf(buf, "D2GS Version:  %s\r\n", D2GS_VERSION_STRING_ONLY);
	SENDSTR(ns, buf);
	sprintf(buf, "D2GE Version: %s\r\n", gD2GEVersionString);
	SENDSTR(ns, buf);
	sprintf(buf, "Busy sleep time:  %lu\r\n", d2gsconf.busysleep);
	SENDSTR(ns, buf);
	sprintf(buf, "Idle sleep time:  %lu\r\n", d2gsconf.idlesleep);
	SENDSTR(ns, buf);
	sprintf(buf, "NT mode:  %s\r\n", d2gsconf.enablentmode ? "Enable" : "Disable");
	SENDSTR(ns, buf);
	sprintf(buf, "GS:  %s\r\n", d2gsconf.enablegslog ? "Enable" : "Disable");
	SENDSTR(ns, buf);
	return;

} /* End of admin_version() */


/**********************************************************
 * Function: admin_getchar
 * Return: None
 ************************************************************/
void admin_getcharinfo(unsigned int ns, u_char *param)
{
	D2CHARINFO		*pCharInfo;

	if (!param || *param=='\0') {
		SENDSTR(ns, "Invalid char name\r\n");
		return;
	}
	pCharInfo = charlist_getdata(param, CHARLIST_GET_CHARINFO);
	if (!pCharInfo) {
		SENDSTR(ns, "char not found\r\n");
		return;
	}
	D2GSShowCharInGame(ns, pCharInfo->lpGameInfo->GameId);
	return;

} /* End of admin_getcharinfo() */


/**********************************************************
 * Function: admin_kick_user
 * Return: None
 ************************************************************/
void admin_kick_user(unsigned int ns, u_char *param)
{
	D2CHARINFO		*pCharInfo;
	unsigned char	buf[64];

	if (!param || *param=='\0') {
		SENDSTR(ns, "Invalid char name\r\n");
		return;
	}
	pCharInfo = charlist_getdata(param, CHARLIST_GET_CHARINFO);
	if (!pCharInfo) {
		SENDSTR(ns, "char not found\r\n");
		return;
	}
	D2GSSendClientChatMessage(pCharInfo->ClientId, CHAT_MESSAGE_TYPE_SYS_MESSAGE,
			D2COLOR_ID_RED, NULL, "You were kicked out of the game by the administrator.");
	Sleep(1000);
	D2GSRemoveClientFromGame(pCharInfo->ClientId);
	sprintf(buf, "Char %s was kicked out\r\n", param);
	SENDSTR(ns, buf);
	return;

} /* End of admin_kick_user() */


/**********************************************************
 * Function: admin_getstatus
 * Return: None
 ************************************************************/
typedef BOOL (WINAPI * GetProcessMemoryInfoFunc)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD);
void admin_getstatus(unsigned int ns, u_char *param)
{
	GetProcessMemoryInfoFunc	GetProcMem;
	HANDLE						hPsapi;
	HANDLE						hProcess;
	PROCESS_MEMORY_COUNTERS		psmem;
	unsigned char		buf[256];
	DWORD				gamenum, usernum;
	int					status;
	FILETIME			ct, et, kt0, ut0, kt, ut;
	LARGE_INTEGER		start, end, freq;
	LONGLONG			*p0, *p;
	D2GSNETSTATISTIC	netstat;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	if (!hProcess || (hProcess==INVALID_HANDLE_VALUE)) {
		SENDSTR(ns, "Can't get current process handle\r\n\r\n");
		return;
	}

	sprintf(buf, "Setting maximum game: %lu\r\n", d2gsconf.gsmaxgames);
	SENDSTR(ns, buf);
	sprintf(buf, "Current maximum game: %lu\r\n", d2gsconf.curgsmaxgames);
	SENDSTR(ns, buf);
	D2GSGetCurrentGameStatistic(&gamenum, &usernum);
	sprintf(buf, "Current running game: %lu\r\n", gamenum);
	SENDSTR(ns, buf);
	sprintf(buf, "Current users in game: %lu\r\n", usernum);
	SENDSTR(ns, buf);
	sprintf(buf, "Maximum prefer users: %lu\r\n", d2gsconf.maxpreferusers);
	SENDSTR(ns, buf);
	sprintf(buf, "Maximum game life: %lu seconds\r\n", d2gsconf.maxgamelife);
	SENDSTR(ns, buf);

	status = D2GSGetConnectionStatus();
	sprintf(buf, "Connetion to D2CS:  %s\r\n",
		status&D2CSERVER ? "connected" : "failed");
	SENDSTR(ns, buf);
	sprintf(buf, "Connetion to D2DBS: %s\r\n",
		status&D2DBSERVER ? "connected" : "failed");
	SENDSTR(ns, buf);

	hPsapi = LoadLibrary("Psapi.dll");
	if (hPsapi) {
		GetProcMem = (GetProcessMemoryInfoFunc)GetProcAddress(hPsapi, "GetProcessMemoryInfo");
		if (GetProcMem) {
			GetProcMem(hProcess, &psmem, sizeof(psmem));
			sprintf(buf, "Physical memory usage: %7.3fMB/%7.3fMB\r\n",
				psmem.WorkingSetSize/1048576.0, psmem.PeakWorkingSetSize/1048576.0);
			SENDSTR(ns, buf);
			sprintf(buf, "Virtual memory usage:  %7.3fMB/%7.3fMB\r\n",
				psmem.PagefileUsage/1048576.0, psmem.PeakPagefileUsage/1048576.0);
			SENDSTR(ns, buf);
		}
		FreeLibrary(hPsapi);
	} else {
		SENDSTR(ns, "No meomory info while Psapi.dll unavailable\r\n");
	}

	QueryPerformanceFrequency(&freq);
	if (freq.QuadPart==0) return;
	QueryPerformanceCounter(&start);
	GetProcessTimes(hProcess, &ct, &et, &kt0, &ut0);
	Sleep(100);
	QueryPerformanceCounter(&end);
	GetProcessTimes(hProcess, &ct, &et, &kt, &ut);
	p0=(LONGLONG*)&kt0;	p=(LONGLONG*)&kt;
	sprintf(buf, "Kernel CPU usage: %6.2f%%\r\n",
		(*p-*p0)*freq.QuadPart/(end.QuadPart-start.QuadPart)/1e5);
	SENDSTR(ns, buf);
	p0=(LONGLONG*)&ut0;	p=(LONGLONG*)&ut;
	sprintf(buf, "User CPU usage:   %6.2f%%\r\n",
		(*p-*p0)*freq.QuadPart/(end.QuadPart-start.QuadPart)/1e5);
	SENDSTR(ns, buf);
	SENDSTR(ns, "\r\n");

	/* net statistic */
	D2GSGetNetStatistic(&netstat);
	SENDSTR(ns, "Game Server Net Statistic: (rate is KBytes/second)\r\n");
	SENDSTR(ns, "        RecvPkts    RecvBytes   SendPkts    SendBytes\r\n");
	sprintf(buf,"D2CS  %10lu   %10lu %10lu   %10lu\r\n",
		netstat.d2cs.recvpacket, netstat.d2cs.recvbytes,
		netstat.d2cs.sendpacket, netstat.d2cs.sendbytes);
	SENDSTR(ns, buf);
	sprintf(buf,"D2DBS %10lu   %10lu %10lu   %10lu\r\n",
		netstat.d2dbs.recvpacket, netstat.d2dbs.recvbytes,
		netstat.d2dbs.sendpacket, netstat.d2dbs.sendbytes);
	SENDSTR(ns, buf);
	SENDSTR(ns, "        RecvRate PeakRecvRate   SendRate PeakSendRate\r\n");
	sprintf(buf, "D2CS  %10.3f %12.3f %10.3f %12.3f\r\n",
		netstat.d2cs.recvrate, netstat.d2cs.peakrecvrate,
		netstat.d2cs.sendrate, netstat.d2cs.peaksendrate);
	SENDSTR(ns, buf);
	sprintf(buf, "D2DBS %10.3f %12.3f %10.3f %12.3f\r\n",
		netstat.d2dbs.recvrate, netstat.d2dbs.peakrecvrate,
		netstat.d2dbs.sendrate, netstat.d2dbs.peaksendrate);
	SENDSTR(ns, buf);
	SENDSTR(ns, "\r\n");

	/* MOTD */
	SENDSTR(ns, "Message of the day:\r\n");
	SENDSTR(ns, d2gsconf.motd);
	SENDSTR(ns, "\r\n\r\n");

	CloseHandle(hProcess);
	switch (D2GSGetShutdownStatus() - 1)
	{
	case 0:
		SENDSTR(ns, "The GS is in restarting progress!\r\n");
		break;
	case 1:
		SENDSTR(ns, "The GS is in shutting down progress!\r\n");
		break;
	case 2:
		SENDSTR(ns, "The GS is in D2CS restarting progress!\r\n");
		break;
	case 3:
		SENDSTR(ns, "The GS is in D2CS shutting down progres!\r\n");
		break;
	default:
		break;
	}
	return;

} /* End of admin_getstatus() */


/**********************************************************
 * Function: admin_msg
 * Return: None
 ************************************************************/
void admin_msg(unsigned int ns, u_char *param)
{
	int		argc;
	char	**argv;
	DWORD	dwMsgType;
	int		ret;

	if (!param || *param=='\0') {
		SENDSTR(ns, "Invalid char name\r\n");
		return;
	}
	argv = strtoargv(param, &argc);
	if (argv==NULL) {
		SENDSTR(ns, "Internal error\r\n");
		return;
	}
	if (argc!=3) {
		SENDSTR(ns, "Invalid parameter\r\n");
		return;
	}

	if (stricmp(argv[0], "SYS")==0)
		dwMsgType = CHAT_MESSAGE_TYPE_SYS_MESSAGE;
	else if (stricmp(argv[0], "C")==0)
		dwMsgType = CHAT_MESSAGE_TYPE_CHAT;
	else if (stricmp(argv[0], "WT")==0)
		dwMsgType = CHAT_MESSAGE_TYPE_WHISPER_TO;
	else if (stricmp(argv[0], "WF")==0)
		dwMsgType = CHAT_MESSAGE_TYPE_WHISPER_FROM;
	else if (stricmp(argv[0], "SC")==0)
		dwMsgType = CHAT_MESSAGE_TYPE_SCROLL;
	else
		dwMsgType = CHAT_MESSAGE_TYPE_SYS_MESSAGE;

	string_color(argv[2]);

	ret = -1;
	switch(argv[1][0])
	{
	case '#':
		{
			WORD	GameId;
			if (stricmp(argv[1], "#all")==0) {
				ret = chat_message_announce_all(dwMsgType, argv[2]);
			} else {
				GameId = (WORD)atoi(&argv[1][1]);
				if (GameId==0)
					SENDSTR(ns, "Invalid game id\r\n");
				else 
					ret = chat_message_announce_game(dwMsgType, GameId, argv[2]);
			}
		}
		break;
	default:
		ret = chat_message_announce_char(dwMsgType, argv[1], argv[2]);
	}

	free(argv);

	if (ret)
		SENDSTR(ns, "Failed\r\n");
	else
		SENDSTR(ns, "Success\r\n");

	return;
 
} /* End of admin_msg() */


/**********************************************************
 * Function: admin_setmotd
 * Return: None
 ************************************************************/
void admin_setmotd(unsigned int ns, u_char *param)
{
	if (!param) {
		SENDSTR(ns, "Invalid parameter\r\n");
		return;
	}

	if (D2GSSetConfigString(REGKEY_MOTD, param)) {
		SENDSTR(ns, "Change MOTD successfully!\r\n");
		strcpy(d2gsconf.motd, param);
	}
	else
		SENDSTR(ns, "Failed change MOTD!\r\n");

} /* End of admin_setmotd() */


void admin_showwe(unsigned int ns, u_char* param)
{
	unsigned char		buf[256];
	unsigned char		timestr[32] = { 0 };
	const WORLDEVENT* ret = 0;
	ret = D2GSInitConfig();
	if (!ret->bHasMsg)
	{
		SENDSTR(ns, "World Event : Disable\r\n");
		return;
	}
	SENDSTR(ns, "     World Event : Enable\r\n");
	sprintf(buf, "        Key Item : %s\r\n", (UCHAR*)(ret + 1));//0x20
	SENDSTR(ns, buf);
	sprintf(buf, "     Total Spawn : %lu\r\n", ret->WorldEventTotalSpawn);//1C
	SENDSTR(ns, buf);
	sprintf(buf, "     Base Count : %lu\r\n", ret->WorldEventBaseCount);//0x04
	SENDSTR(ns, buf);
	sprintf(buf, "Last Spawn Count : %lu\r\n", ret->WorldEventLastSpawnCount);//0C
	SENDSTR(ns, buf);
	sprintf(buf, "     Current Count : %lu\r\n", ret->WorldEventCurrentSpawnCount);//0x08
	SENDSTR(ns, buf);
	sprintf(buf, "Next Spawn Count : %lu\r\n", ret->WorldEventNextSpawnCount);//0x10
	SENDSTR(ns, buf);

	if (ret->WorldEventCurrentSpawnCount != 0)
	{
		FormatTimeString(ret->WorldEventLastSellTime, timestr, 0x20, 1);
	}
	else
	{
		strcpy(timestr, "enon");
	}
	sprintf(buf, "  Last Sell Time : %s\r\n", timestr);
	SENDSTR(ns, buf);

	if (ret->WorldEventTotalSpawn != 0)
	{
		FormatTimeString(ret->WorldEventLastSpawnTime, timestr, 0x20, 1);
	}
	else
	{
		strcpy(timestr, "enon");
	}
	sprintf(buf, "  Last Spawn Time : %s\r\n", timestr);
	SENDSTR(ns, buf);
	SENDSTR(ns, "\r\n");
}
