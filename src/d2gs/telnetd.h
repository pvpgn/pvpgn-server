#ifndef INCLUDED_TELNETD_H
#define INCLUDED_TELNETD_H


namespace pvpgn
{

	namespace d2gs
	{

		/* TELNET Command Codes */
#define TC_IAC			(u_char)255
#define TC_DONT			(u_char)254
#define TC_DO			(u_char)253
#define TC_WONT			(u_char)252
#define TC_WILL			(u_char)251


/* TELNET Option Codes */
#define TC_TXBINARY		(u_char)0
#define TC_ECHO			(u_char)1
#define TC_NOGA			(u_char)3
#define TC_GA			(u_char)249


#define SENDSTR(ns, msg)		send(ns, msg, strlen(msg), 0)
#define SENDCHAR(ns, ch)		send(ns, ch, 1, 0)


/* const */
#define ADMIN_SESSION_TIMEOUT_UNIT		1

/* Types */
		typedef struct
		{
			char* keyword;
			BOOL	disable;
			void	(*adminfunc)(unsigned int, char*);
			char* param;
			char* annotation;
		}  ADMINCOMMAND, * PADMINCOMMAND;


		/* functions */
		int  D2GSAdminInitialize(void);
		int  CleanupRoutineForAdmin(void);
		DWORD WINAPI admin_service(LPVOID lpParam);
		DWORD WINAPI admin_thread(LPVOID lpParam);
		int  admin_analyse_cmd(char* buf, char* cmd, int x, int y);
		int  admin_to_stop(void);
		int  get_cmd_line(unsigned int ns, char* buf, int flag);
		int  admin_getchar(unsigned int ns);
		int  admin_check_pass(const char* pass);
		void admin_logo(unsigned int ns);
		void admin_help(unsigned int ns, char* param);
		void admin_show_game_list(unsigned int ns, char* param);
		void admin_show_char_in_game(unsigned int ns, char* param);
		void admin_restart(unsigned int ns, char* param);
		void admin_shutdown(unsigned int ns, char* param);
		void admin_uptime(unsigned int ns, char* param);
		void admin_setmaxgame(unsigned int ns, char* param);
		void admin_disablegame(unsigned int ns, char* param);
		void admin_enablegame(unsigned int ns, char* param);
		void admin_setmaxgamelife(unsigned int ns, char* param);
		void admin_getstatus(unsigned int ns, char* param);
		void admin_getversion(unsigned int ns, char* param);
		void admin_getcharinfo(unsigned int ns, char* param);
		void admin_chgpasswd(unsigned int ns, char* param);
		void admin_kick_user(unsigned int ns, char* param);
		void admin_msg(unsigned int ns, char* param);
		void admin_setmotd(unsigned int ns, char* param);
		void admin_setmaxpreferusers(unsigned int ns, char* param);
		void admin_setcpumask(unsigned int ns, char* param);
		void admin_reloadconf(unsigned int ns, char* param);
		void admin_showwe(unsigned int ns, char* param);
		void admin_enablegslog(unsigned int ns, char* param);

	}

}


#endif /* INCLUDED_TELNETD_H */