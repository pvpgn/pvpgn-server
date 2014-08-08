/*
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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

#define APIREGISTER_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "handle_apireg.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "common/eventlog.h"
#include "common/bnethash.h"
#include "common/wolhash.h"
#include "common/list.h"
#include "common/packet.h"

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"

#include "prefs.h"
#include "irc.h"
#include "account.h"
#include "account_wrap.h"
#include "message.h"
#include "server.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_apireg_tag)(t_apiregmember * apiregmember, char * param);

		static t_list * apireglist_head = NULL;

		typedef struct {
			const char * apireg_tag_string;
			t_apireg_tag  apireg_tag_handler;
		} t_apireg_tag_table_row;

		static int _handle_email_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_bmonth_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_bday_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_byear_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_langcode_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_sku_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_ver_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_serial_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_sysid_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_syscheck_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_oldnick_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_oldpass_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_newnick_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_newpass_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_newpass2_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_parentemail_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_newsletter_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_shareinfo_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_request_apiregtag(t_apiregmember * apiregmember, char * param);
		static int _handle_end_apiregtag(t_apiregmember * apiregmember, char * param);

		static const t_apireg_tag_table_row apireg_tag_table[] =
		{
			{ "EMAIL", _handle_email_apiregtag },
			{ "BMONTH", _handle_bmonth_apiregtag },
			{ "BDAY", _handle_bday_apiregtag },
			{ "BYEAR", _handle_byear_apiregtag },
			{ "LANGCODE", _handle_langcode_apiregtag },
			{ "SKU", _handle_sku_apiregtag },
			{ "VER", _handle_ver_apiregtag },
			{ "SERIAL", _handle_serial_apiregtag },
			{ "SYSID", _handle_sysid_apiregtag },
			{ "SYSCHECK", _handle_syscheck_apiregtag },
			{ "OLDNICK", _handle_oldnick_apiregtag },
			{ "OLDPASS", _handle_oldpass_apiregtag },
			{ "NEWNICK", _handle_newnick_apiregtag },
			{ "NEWPASS", _handle_newpass_apiregtag },
			{ "NEWPASS2", _handle_newpass2_apiregtag },
			{ "PARENTEMAIL", _handle_parentemail_apiregtag },
			{ "NEWSLETTER", _handle_newsletter_apiregtag },
			{ "SHAREINFO", _handle_shareinfo_apiregtag },
			{ "REQUEST", _handle_request_apiregtag },
			{ "END", _handle_end_apiregtag },

			{ NULL, NULL }
		};

		static t_apiregmember * apiregmember_create(t_connection * conn)
		{
			t_apiregmember * temp;

			if (!conn) {
				ERROR0("got NULL conn");
				return NULL;
			}

			temp = (t_apiregmember*)xmalloc(sizeof(t_apiregmember));

			eventlog(eventlog_level_info, __FUNCTION__, "creating apiregmember");

			temp->conn = conn;
			temp->email = NULL;
			temp->bday = NULL;
			temp->bmonth = NULL;
			temp->byear = NULL;
			temp->langcode = NULL;   //"0";
			temp->sku = NULL;
			temp->ver = NULL;
			temp->serial = NULL;
			temp->sysid = NULL;   //"((SysID))";
			temp->syscheck = NULL;   //"((SysCheck))";
			temp->oldnick = NULL;
			temp->oldpass = NULL;
			temp->newnick = NULL;   //"((NewPass))";
			temp->newpass = NULL;   //"((NewPass))";
			temp->newpass2 = NULL;   //"((NewPass2))";
			temp->parentemail = NULL;   //"((ParentEmail))";
			temp->newsletter = false;
			temp->shareinfo = false;
			temp->request = NULL;   //"((Request))";

			list_append_data(apireglist_head, temp);

			return temp;
		}

		static int apiregmember_destroy(t_apiregmember * apiregmember, t_elem ** curr)
		{
			eventlog(eventlog_level_info, __FUNCTION__, "destroying apiregmember");

			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (list_remove_data(apireglist_head, apiregmember, curr) < 0){
				ERROR0("could not remove item from list");
				return -1;
			}

			if (apiregmember->email)
				xfree((void *)apiregmember->email); /* avoid warning */

			if (apiregmember->bday)
				xfree((void *)apiregmember->bday); /* avoid warning */

			if (apiregmember->bmonth)
				xfree((void *)apiregmember->bmonth); /* avoid warning */

			if (apiregmember->byear)
				xfree((void *)apiregmember->byear); /* avoid warning */

			if (apiregmember->langcode)
				xfree((void *)apiregmember->langcode); /* avoid warning */

			if (apiregmember->sku)
				xfree((void *)apiregmember->sku); /* avoid warning */

			if (apiregmember->ver)
				xfree((void *)apiregmember->ver); /* avoid warning */

			if (apiregmember->serial)
				xfree((void *)apiregmember->serial); /* avoid warning */

			if (apiregmember->sysid)
				xfree((void *)apiregmember->sysid); /* avoid warning */

			if (apiregmember->syscheck)
				xfree((void *)apiregmember->syscheck); /* avoid warning */

			if (apiregmember->oldnick)
				xfree((void *)apiregmember->oldnick); /* avoid warning */

			if (apiregmember->oldpass)
				xfree((void *)apiregmember->oldpass); /* avoid warning */

			if (apiregmember->newnick)
				xfree((void *)apiregmember->newnick); /* avoid warning */

			if (apiregmember->newpass)
				xfree((void *)apiregmember->newpass); /* avoid warning */

			if (apiregmember->newpass2)
				xfree((void *)apiregmember->newpass2); /* avoid warning */

			if (apiregmember->parentemail)
				xfree((void *)apiregmember->parentemail); /* avoid warning */

			//    if (apiregmember->newsletter)
			//        xfree((void *)apiregmember->newsletter); /* avoid warning */

			//    if (apiregmember->shareinfo)
			//        xfree((void *)apiregmember->shareinfo); /* avoid warning */

			if (apiregmember->request)
				xfree((void *)apiregmember->request); /* avoid warning */

			xfree(apiregmember);

			return 0;
		}

		static t_connection * apiregmember_get_conn(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->conn;
		}

		static char const * apiregmember_get_email(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->email;
		}

		static char const * apiregmember_get_bday(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->bday;
		}

		static char const * apiregmember_get_bmonth(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->bmonth;
		}

		static char const * apiregmember_get_newnick(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->newnick;
		}

		static char const * apiregmember_get_newpass(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->newpass;
		}

		static char const * apiregmember_get_request(t_apiregmember const * apiregmember)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return NULL;
			}

			return apiregmember->request;
		}

		extern int apireglist_create(void)
		{
			apireglist_head = list_create();
			return 0;
		}

		extern int apireglist_destroy(void)
		{
			t_apiregmember * apiregmember;
			t_elem * curr;

			if (apireglist_head) {
				LIST_TRAVERSE(apireglist_head, curr) {
					if (!(apiregmember = (t_apiregmember*)elem_get_data(curr))) {
						ERROR0("channel list contains NULL item");
						continue;
					}
					apiregmember_destroy(apiregmember, &curr);
				}

				if (list_destroy(apireglist_head) < 0)
					return -1;
				apireglist_head = NULL;
			}

			return 0;
		}

		extern t_list * apireglist(void)
		{
			return apireglist_head;
		}

		static t_apiregmember * apireglist_find_apiregmember_by_conn(t_connection * conn)
		{
			t_apiregmember * apiregmember;
			t_elem * curr;

			if (!conn) {
				ERROR0("got NULL conn");
				return NULL;
			}

			LIST_TRAVERSE(apireglist(), curr) {
				t_apiregmember * apiregmember = (t_apiregmember *)elem_get_data(curr);
				if (conn == apiregmember_get_conn(apiregmember)) {
					return apiregmember;
				}
			}

			return NULL;
		}

		static int handle_apireg_tag(t_apiregmember * apiregmember, char const * tag, char * param)
		{
			t_apireg_tag_table_row const *p;

			for (p = apireg_tag_table; p->apireg_tag_string != NULL; p++) {
				if (strcasecmp(tag, p->apireg_tag_string) == 0) {
					if (p->apireg_tag_handler != NULL)
						return ((p->apireg_tag_handler)(apiregmember, param));
				}
			}
			return -1;
		}

		static int handle_apireg_line(t_connection * conn, char const * apiregline)
		{
			/* <command>=[param] */
			char * line; /* copy of apiregline */
			char * tag = NULL; /* mandatory */
			char * param = NULL; /* param of tag */
			t_apiregmember * apiregmember = apireglist_find_apiregmember_by_conn(conn);

			if (!conn) {
				ERROR0("got NULL connection");
				return -1;
			}
			if (!apiregline) {
				ERROR0("got NULL apiregline");
				return -1;
			}
			if (apiregline[0] == '\0') {
				ERROR0("got empty apiregline");
				return -1;
			}

			if (std::strlen(apiregline) > 254) {
				char * tmp = (char *)apiregline;
				WARN0("line to long, truncation...");
				tmp[254] = '\0';
			}

			if (!apiregmember) {
				apiregmember = apiregmember_create(conn);
			}
			line = xstrdup(apiregline);

			/* split the line */
			tag = line;
			param = std::strchr(tag, '=');
			if (param)
				*param++ = '\0';


			eventlog(eventlog_level_debug, __FUNCTION__, "[%d] got \"%s\" [%s]", conn_get_socket(conn), tag, ((param) ? (param) : ("")));

			if (handle_apireg_tag(apiregmember, tag, param) != -1) {}
			xfree(line);

			return 0;
		}

		extern int handle_apireg_packet(t_connection * conn, t_packet const * const packet)
		{
			unsigned int i;
			char apiregline[MAX_IRC_MESSAGE_LEN];
			char const * data;

			if (!packet) {
				ERROR0("got NULL packet");
				return -1;
			}
			if (conn_get_class(conn) != conn_class_apireg) {
				ERROR0("FIXME: handle_apireg_packet without any reason (conn->class != conn_class_apireg)");
				return -1;
			}

			/* eventlog(eventlog_level_debug,__FUNCTION__,"got \"%s\"",packet_get_raw_data_const(packet,0)); */

			std::memset(apiregline, 0, sizeof(apiregline));

			data = conn_get_ircline(conn); /* fetch current status */
			if (data)
				std::strcpy(apiregline, data);

			unsigned apiregpos = std::strlen(apiregline);
			data = (const char *)packet_get_raw_data_const(packet, 0);

			for (i = 0; i < packet_get_size(packet); i++) {
				if ((data[i] == '\r') || (data[i] == '\0')) {
					/* kindly ignore \r and NUL ... */
				}
				else if (data[i] == '\n') {
					/* end of line */
					handle_apireg_line(conn, apiregline);
					std::memset(apiregline, 0, sizeof(apiregline));
					apiregpos = 0;
				}
				else {
					if (apiregpos < MAX_IRC_MESSAGE_LEN - 1)
						apiregline[apiregpos++] = data[i];
					else {
						apiregpos++; /* for the statistic :) */
						WARN2("[%d] client exceeded maximum allowed message length by %d characters", conn_get_socket(conn), apiregpos - MAX_IRC_MESSAGE_LEN);
						if (apiregpos > 100 + MAX_IRC_MESSAGE_LEN) {
							/* automatic flood protection */
							ERROR1("[%d] excess flood", conn_get_socket(conn));
							return -1;
						}
					}
				}
			}
			conn_set_ircline(conn, apiregline); /* write back current status */
			return 0;
		}

		static int apireg_send(t_connection * conn, char const * command)
		{
			t_packet * p;
			char data[MAX_IRC_MESSAGE_LEN + 1];
			unsigned len;
			t_apiregmember * apiregmember;
			t_elem * curr;

			p = packet_create(packet_class_raw);

			if (command)
				len = (std::strlen(command));

			if (len > MAX_IRC_MESSAGE_LEN) {
				ERROR1("message to send is too large (%u bytes)", len);
				return -1;
			}
			else {
				std::sprintf(data, "%s", command);
			}

			packet_set_size(p, 0);
			packet_append_data(p, data, len);
			DEBUG2("[%d] sent \"%s\"", conn_get_socket(conn), data);
			conn_push_outqueue(conn, p);
			packet_del_ref(p);

			/* In apiregister server we must destroy apiregmember and connection after send packet */

			LIST_TRAVERSE(apireglist(), curr) {
				t_apiregmember * tempapireg = (t_apiregmember*)elem_get_data(curr);

				if (conn == apiregmember_get_conn(tempapireg))
					apiregmember_destroy(tempapireg, &curr);
			}

			conn_set_state(conn, conn_state_destroy);

			return 0;
		}

		static int _handle_email_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->email)
				xfree((void *)apiregmember->email);   /* avoid warning */

			if (param)
				apiregmember->email = xstrdup(param);

			return 0;
		}

		static int _handle_bmonth_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->bmonth)
				xfree((void *)apiregmember->bmonth);   /* avoid warning */

			if (param)
				apiregmember->bmonth = xstrdup(param);

			return 0;
		}

		static int _handle_bday_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->bday)
				xfree((void *)apiregmember->bday);   /* avoid warning */

			if (param)
				apiregmember->bday = xstrdup(param);

			return 0;
		}

		static int _handle_byear_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->byear)
				xfree((void *)apiregmember->byear);   /* avoid warning */

			if (param)
				apiregmember->byear = xstrdup(param);

			return 0;
		}

		static int _handle_langcode_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->langcode)
				xfree((void *)apiregmember->langcode);   /* avoid warning */

			if (param)
				apiregmember->langcode = xstrdup(param);

			return 0;
		}

		static int _handle_sku_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/* We have SKUs */

			return 0;
		}

		static int _handle_ver_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/* We have VERs */

			return 0;
		}

		static int _handle_serial_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/* We have SERIALs */

			return 0;
		}

		static int _handle_sysid_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->sysid)
				xfree((void *)apiregmember->sysid);   /* avoid warning */

			if (param)
				apiregmember->sysid = xstrdup(param);

			return 0;
		}

		static int _handle_syscheck_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->syscheck)
				xfree((void *)apiregmember->syscheck);   /* avoid warning */

			if (param)
				apiregmember->syscheck = xstrdup(param);

			return 0;
		}

		static int _handle_oldnick_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/* We have OLDNICKs */

			return 0;
		}

		static int _handle_oldpass_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/* We have OLDPASSs */

			return 0;
		}

		static int _handle_newnick_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->newnick)
				xfree((void *)apiregmember->newnick);

			if (param)
				apiregmember->newnick = xstrdup(param);

			return 0;
		}

		static int _handle_newpass_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->newpass)
				xfree((void *)apiregmember->newpass);   /* avoid warning */

			if (param)
				apiregmember->newpass = xstrdup(param);

			return 0;
		}

		static int _handle_newpass2_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->newpass2)
				xfree((void *)apiregmember->newpass2);   /* avoid warning */

			if (param)
				apiregmember->newpass2 = xstrdup(param);

			return 0;
		}

		static int _handle_parentemail_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->parentemail)
				xfree((void *)apiregmember->parentemail);   /* avoid warning */

			if (param)
				apiregmember->parentemail = xstrdup(param);

			return 0;
		}

		static int _handle_newsletter_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/*	if (param) {
				   if (std::strcmp(param, "1") == 0)
				   apiregmember->newsletter = true;
				   else
				   apiregmember->newsletter = false;
				   }*/

			return 0;
		}

		static int _handle_shareinfo_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			/*	if (param) {
				   if (std::strcmp(param, "1") == 0)
				   apiregmember->shareinfo = true;
				   else
				   apiregmember->shareinfo = false;
				   }*/

			return 0;
		}

		static int _handle_request_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			if (!apiregmember) {
				ERROR0("got NULL apiregmember");
				return -1;
			}

			if (apiregmember->request)
				xfree((void *)apiregmember->request);   /* avoid warning */

			if (param)
				apiregmember->request = xstrdup(param);

			return 0;
		}

		/*static char _handle_apireg_message_format(char const * hresult, char const * message, char const * newnick, char const * newpass, char const * age, char const * consent)
		{

		}*/

		static int _handle_end_apiregtag(t_apiregmember * apiregmember, char * param)
		{
			char data[MAX_IRC_MESSAGE_LEN];
			char temp[MAX_IRC_MESSAGE_LEN];
			t_connection * conn = apiregmember_get_conn(apiregmember);
			t_elem * curr;
			t_account * account;
			char const * newnick = apiregmember_get_newnick(apiregmember);
			char const * newpass = apiregmember_get_newpass(apiregmember);
			char const * email = apiregmember_get_email(apiregmember);
			char const * request = apiregmember_get_request(apiregmember);
			//    char const * newpass2 = apiregmember_get_newpass2(apiregmember);
			char hresult[12];
			char message[MAX_IRC_MESSAGE_LEN];
			char age[8];
			char consent[12];

			std::memset(data, 0, sizeof(data));
			std::memset(temp, 0, sizeof(temp));

			std::memset(hresult, 0, sizeof(hresult));
			std::memset(message, 0, sizeof(message));
			std::memset(age, 0, sizeof(age));
			std::memset(consent, 0, sizeof(consent));

			snprintf(hresult, sizeof(hresult), "0");
			snprintf(message, sizeof(message), "((Message))");
			snprintf(age, sizeof(age), "((Age))");
			snprintf(consent, sizeof(consent), "((Consent))");

			//   	if (!newnick)
			//   	   snprintf(newnick,sizeof(newnick),"((NewNick))");

			//   	if (!newpass)
			//   	   snprintf(newpass,sizeof(newpass),"((NewPass))");

			//   	if (!email)
			//   	   snprintf(email,sizeof(email),"((Email))");

			DEBUG3("APIREG:/%s/%s/%s/", apiregmember_get_request(apiregmember), apiregmember_get_newnick(apiregmember), apiregmember_get_newpass(apiregmember));

			if ((request) && (std::strcmp(apiregmember_get_request(apiregmember), REQUEST_AGEVERIFY) == 0)) {
				snprintf(data, sizeof(data), "HRESULT=%s\nMessage=%s\nNewNick=((NewNick))\nNewPass=((NewPass))\n", hresult, message);
				/* FIXME: Count real age here! */
				snprintf(age, sizeof(age), "28"); /* FIXME: Here must be counted age */
				snprintf(temp, sizeof(temp), "Age=%s\nConsent=((Consent))\nEND\r", age);
				std::strcat(data, temp);
				apireg_send(apiregmember_get_conn(apiregmember), data);
				return 0;
			}
			else if ((request) && (std::strcmp(apiregmember_get_request(apiregmember), REQUEST_GETNICK) == 0)) {
				if (!prefs_get_allow_new_accounts()){
					snprintf(message, sizeof(message), "Account creation is not allowed");
					snprintf(hresult, sizeof(hresult), "-2147221248");
				}
				else {
					if (!newnick) {
						snprintf(message, sizeof(message), "Nick must be specifed!");
						snprintf(hresult, sizeof(hresult), "-2147221248");
					}
					else if (!newpass) {
						snprintf(message, sizeof(message), "Pussword must be specifed!");
						snprintf(hresult, sizeof(hresult), "-2147221248");
					}
					else if (account = accountlist_find_account(newnick)) {
						snprintf(message, sizeof(message), "That login is already in use! Please try another NICK name.");
						snprintf(hresult, sizeof(hresult), "-2147221248");
					}
					else {  /* done, we can create new account */
						t_account * tempacct;
						t_hash bnet_pass_hash;
						t_wolhash wol_pass_hash;

						/* Here we can also check serials and/or emails... */

						bnet_hash(&bnet_pass_hash, std::strlen(newpass), newpass);
						wol_hash(&wol_pass_hash, std::strlen(newpass), newpass);

						tempacct = accountlist_create_account(newnick, hash_get_str(bnet_pass_hash));

						if (!tempacct) {
							// ERROR: Account is not created! - Why? :)
							return 0;
						}
						else {
							eventlog(eventlog_level_debug, __FUNCTION__, "WOLHASH: %s", wol_pass_hash);
							account_set_wol_apgar(tempacct, wol_pass_hash);
							if (apiregmember_get_email(apiregmember))
								account_set_email(tempacct, apiregmember_get_email(apiregmember));
							snprintf(message, sizeof(message), "Welcome in the amazing world of PvPGN! Your login can be used for all PvPGN Supported games!");
							snprintf(hresult, sizeof(hresult), "0");
						}
					}
				}
				snprintf(data, sizeof(data), "HRESULT=%s\nMessage=%s\nNewNick=%s\nNewPass=%s\nAge=%s\nConsent=0\nEND\r", hresult, message, newnick, newpass, age, consent);
				apireg_send(apiregmember_get_conn(apiregmember), data);
				return 0;
			}
			else {
				/* Error: Unknown request - closing connection */
				ERROR1("got UNKNOWN request /%s/ closing connection", apiregmember->request);
				LIST_TRAVERSE(apireglist(), curr) {
					t_apiregmember * apiregmemberlist = (t_apiregmember*)elem_get_data(curr);

					if (conn == apiregmember_get_conn(apiregmemberlist))
						apiregmember_destroy(apiregmember, &curr);
				}

				conn_set_state(conn, conn_state_destroy);
				return 0;
			}

			return 0;
		}

	}

}
