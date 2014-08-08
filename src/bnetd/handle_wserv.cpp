/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2005  Bryan Biedenkapp (gatekeep@gmail.com)
 * Copyright (C) 2006,2007,2008  Pelish (pelish@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "common/setup_before.h"
#include "handle_wserv.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "compat/strcasecmp.h"
#include "common/irc_protocol.h"
#include "common/eventlog.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/trans.h"

#include "compat/snprintf.h"

#include "prefs.h"
#include "irc.h"
#include "message.h"
#include "tick.h"
#include "server.h"
#include "autoupdate.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_wserv_command)(t_connection * conn, int numparams, char ** params, char * text);

		typedef struct {
			const char       * wserv_command_string;
			t_wserv_command    wserv_command_handler;
		} t_wserv_command_table_row;

		static int _handle_verchk_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_lobcount_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_whereto_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text);

		/* state "connected" handlers (on Westwood ServServ server are not loged-in) */
		static const t_wserv_command_table_row wserv_con_command_table[] =
		{
			{ "VERCHK", _handle_verchk_command },
			{ "LOBCOUNT", _handle_lobcount_command },
			{ "WHERETO", _handle_whereto_command },
			{ "QUIT", _handle_quit_command },

			{ NULL, NULL }
		};

		extern int handle_wserv_con_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			t_wserv_command_table_row const *p;

			for (p = wserv_con_command_table; p->wserv_command_string != NULL; p++) {
				if (strcasecmp(command, p->wserv_command_string) == 0) {
					if (p->wserv_command_handler != NULL)
						return ((p->wserv_command_handler)(conn, numparams, params, text));
				}
			}
			return -1;
		}

		static int _handle_verchk_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_clienttag clienttag;
			char *filestring;
			char const *ftphostname;
			char const *ftpusername;
			char const *ftppassword;

			/**
			 *  Heres the imput expected:
			 *  vercheck [SKU] [version]
			 *
			 *  Here is output expected for ServServ server:
			 *  1) Update non-existant:
			 *  :[servername] 602 [username] :Update record non-existant
			 *  2) Update existant:
			 *  :[servername] 606 [username] :[ftpserveraddr] [ftpusername] [ftppaswd] [path] [file.rtp] [newversion] [SKU] REQ
			 */

			if (numparams >= 2) {
				clienttag = tag_sku_to_uint(std::atoi(params[0]));

				if (clienttag != CLIENTTAG_WWOL_UINT)
					conn_set_clienttag(conn, clienttag);

				if (filestring = autoupdate_check(ARCHTAG_WINX86_UINT, clienttag, TAG_UNKNOWN_UINT, params[1], params[0])) {
					//:westwood-patch.ea.com update world96 lore3/1.003 65539_65536_6400.rtp 65539 6400 REQ
					ftphostname = prefs_get_wol_autoupdate_serverhost();
					ftpusername = prefs_get_wol_autoupdate_username();
					ftppassword = prefs_get_wol_autoupdate_password();
					snprintf(temp, sizeof(temp), ":%s %s %s %s 131075 %s REQ", ftphostname, ftpusername, ftppassword, filestring, params[0]);
					irc_send(conn, RPL_UPDATE_FTP, temp);
				}
				else
					irc_send(conn, RPL_UPDATE_NONEX, ":Update record non-existant");
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "VERCHK :Not enough parameters");
			return 0;
		}

		static int _handle_lobcount_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/* Ignore command but, return 1 */
			irc_send(conn, RPL_LOBCOUNT, "1");

			return 0;
		}

		static int _handle_whereto_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];

			/* Casted to avoid warnings */
			const char * wolip;
			const char * wolname = prefs_get_servername();
			const char * woltimezone = prefs_get_wol_timezone();
			const char * wollong = prefs_get_wol_longitude();
			const char * wollat = prefs_get_wol_latitude();

			{    /* trans support */
				unsigned short port = conn_get_real_local_port(conn);
				unsigned int addr = conn_get_real_local_addr(conn);

				trans_net(conn_get_addr(conn), &addr, &port);

				wolip = addr_num_to_ip_str(addr);
			}

			//irc_send(conn,RPL_UPDATE_EXIST,":You must update before connecting!");

			// Check if it's an allowed client type
			if (!tag_check_in_list(conn_get_clienttag(conn), prefs_get_allowed_clients())) {
				//  This is for anyone game but not for Emperor
				if (conn_get_clienttag(conn) != CLIENTTAG_EMPERORBD_UINT) {
					//        a.xwis.net 4009 RA2
					//        c.xwis.net 4000 TSUN
					//        c.xwis.net 4010 RNGD
					//        a.xwis.net 4010 YURI
					//            snprintf(temp, sizeof(temp), ":a.xwis.net 4009 '0:%s' %s %s %s", wolname, woltimezone, wollong, wollat);
					//            snprintf(temp, sizeof(temp), ":c.xwis.net 4000 '0:%s' %s %s %s", wolname, woltimezone, wollong, wollat);
					//            snprintf(temp, sizeof(temp), ":c.xwis.net 4010 '0:%s' %s %s %s", wolname, woltimezone, wollong, wollat);
					//            snprintf(temp, sizeof(temp), ":a.xwis.net 4010 '0:%s' %s %s %s", wolname, woltimezone, wollong, wollat);
					snprintf(temp, sizeof(temp), ":%s %d '0:%s' %s %s %s", wolip, BNETD_WOLV2_PORT, wolname, woltimezone, wollong, wollat);
					irc_send(conn, RPL_WOLSERV, temp);
				}

				//  Only for Emperor: Battle for Dune
				if (conn_get_clienttag(conn) == CLIENTTAG_EMPERORBD_UINT) {
					snprintf(temp, sizeof(temp), ":%s %d '0:Emperor %s' %s %s %s", wolip, BNETD_WOLV2_PORT, wolname, woltimezone, wollong, wollat);
					irc_send(conn, RPL_WOLSERV, temp);
				}

				//  Only for CnC Renegade
				if ((conn_get_clienttag(conn) == CLIENTTAG_RENEGADE_UINT) || (conn_get_clienttag(conn) == CLIENTTAG_RENGDFDS_UINT)) {
					snprintf(temp, sizeof(temp), ":%s 0 'Ping server' %s %s %s", wolip, woltimezone, wollong, wollat);
					irc_send(conn, RPL_PINGSERVER, temp);
					//I dont know for what is this server...? (used in renegade and yuri)
					//snprintf(temp, sizeof(temp), ":%s 4321 'Port Mangler' %s %s %s", wolip, woltimezone, wollong, wollat);
					//irc_send(conn,RPL_MANGLERSERV,temp);
					// on official server list is for Renegade also this server:
					//:noxchat1.wol.abn-sjc.ea.com 613 UserName :ea4.str.ea.com 0 '0,1,2,3,4,5,6,7,8,9,10:EA Ticket Server' -8 36.1083 -115.0582
				}

				//  There are servers for anyone game
				// FIXME: Check if is WOLv1 supported
				snprintf(temp, sizeof(temp), ":%s %d 'Live chat server' %s %s %s", wolip, BNETD_WOLV1_PORT, woltimezone, wollong, wollat);
				irc_send(conn, RPL_WOLSERV, temp);
			}

			// If game is not allowed than we still send this servers 
			snprintf(temp, sizeof(temp), ":%s %d 'Gameres server' %s %s %s", wolip, BNETD_WGAMERES_PORT, woltimezone, wollong, wollat);
			irc_send(conn, RPL_GAMERESSERV, temp);
			snprintf(temp, sizeof(temp), ":%s %d 'Ladder server' %s %s %s", wolip, BNETD_WOLV2_PORT, woltimezone, wollong, wollat);
			irc_send(conn, RPL_LADDERSERV, temp);
			// There is Word Domination Tour server for Firestorm (maybe for future coding)
			//snprintf(temp, sizeof(temp), ":%s %d 'WDT server' %s %s %s", wolip, BNETD_WOLV2_PORT, woltimezone, wollong, wollat); //I dont know for what is this server...?
			//irc_send(conn,RPL_WDTSERV,temp);

			return 0;
		}

		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			irc_send(conn, RPL_QUIT, ":goodbye");
			conn_set_state(conn, conn_state_destroy);

			return 0;
		}

	}

}
