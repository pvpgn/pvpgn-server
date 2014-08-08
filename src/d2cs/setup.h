/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#ifndef INCLUDED_D2CS_SETUP_H
#define INCLUDED_D2CS_SETUP_H

#define tf(a)	 ((a)?1:0)

#define strcmp_charname		strcasecmp
#define strncmp_charname	strncasencmp

#define BEGIN_LIST_TRAVERSE_DATA(list,data,type) \
{\
	t_elem * curr_elem_; \
for (curr_elem_ = list_get_first(list); curr_elem_ && (data = (type*)elem_get_data(curr_elem_)); \
	curr_elem_ = elem_get_next(list, curr_elem_))

#define END_LIST_TRAVERSE_DATA() \
}

#define BEGIN_LIST_TRAVERSE_DATA_CONST(list,data,type)\
{\
	t_elem const * curr_elem_; \
for (curr_elem_ = list_get_first_const(list); curr_elem_ && (data = (type*)elem_get_data(curr_elem_)); \
	curr_elem_ = elem_get_next_const(list, curr_elem_))

#define END_LIST_TRAVERSE_DATA_CONST() \
}

#define BEGIN_HASHTABLE_TRAVERSE_DATA(hashtable,data,type)\
{\
	t_entry * curr_entry_; \
for (curr_entry_ = hashtable_get_first(hashtable); curr_entry_ && (data = (type*)entry_get_data(curr_entry_)); \
	curr_entry_ = entry_get_next(curr_entry_))

#define END_HASHTABLE_TRAVERSE_DATA()	\
}

#define BEGIN_HASHTABLE_TRAVERSE_MATCHING_DATA(hashtable,data,hash,type)\
{\
	t_entry * curr_entry_; \
for (curr_entry_ = hashtable_get_first_matching(hashtable, hash); \
	curr_entry_ && (data = (type*)entry_get_data(curr_entry_)); \
	curr_entry_ = entry_get_next_matching(curr_entry_))

#define END_HASHTABLE_TRAVERSE_DATA()	\
}

#define CASE(condition,func) case condition:\
	func; \
	break; \

#define ASSERT(var,retval) if (!var) { eventlog(eventlog_level_error,__FUNCTION__,"got NULL " #var); return retval; }
#define DECLARE_PACKET_HANDLER(handler) static int handler(t_connection *, t_packet *);
#define NELEMS(s)		sizeof(s)/sizeof(s[0])

#define MAX_SAVEFILE_SIZE	32 * 1024
#define MAX_CHAR_PER_GAME	8
#define D2CS_SERVER_PORT	6113
#define MAX_GAME_IDLE_TIME	0
#define DEFAULT_S2S_RETRYINTERVAL	60
#define DEFAULT_S2S_TIMEOUT		60
#define DEFAULT_SQ_CHECKINTERVAL	300
#define DEFAULT_SQ_TIMEOUT		300
#define DEFAULT_REALM_NAME		"D2CS"
#define DEFAULT_SHUTDOWN_DELAY		300
#define DEFAULT_SHUTDOWN_DECR		60
#define DEFAULT_S2S_IDLETIME		300
#define DEFAULT_S2S_KEEPALIVE_INTERVAL	60
#define DEFAULT_TIMEOUT_CHECKINTERVAL	60
#define DEFAULT_ACC_ALLOWED_SYMBOLS     "-_[]"
#define DEFAULT_D2GS_RESTART_DELAY	300

#define MAJOR_VERSION_EQUAL(v1,v2,mask)         (((v1) & (mask)) == ((v2) & (mask)))


#ifndef D2CS_DEFAULT_CONF_FILE
# define D2CS_DEFAULT_CONF_FILE       "conf/d2cs.conf"
#endif

#define DEFAULT_LOG_FILE	"/usr/local/var/d2cs.std::log"
#define DEFAULT_LOG_LEVELS	"info,warn,error"
#define DEFAULT_MEMLOG_FILE	"/tmp/d2cs-mem.std::log"

#define D2CS_SERVER_ADDRS	"0.0.0.0"
#define D2GS_SERVER_LIST	"192.168.0.1"
#define BNETD_SERVER_LIST	"192.168.0.1"
#define MAX_D2GAME_NUMBER	30
#define MAX_CHAR_PER_ACCT	8
// MAX_MAX_CHAR_PER_ACCT is needed cause D2 client can at worst case only handle 18 chars
#define MAX_MAX_CHAR_PER_ACCT	18
#define MAX_CLIENT_IDLETIME	30 * 60

#define D2CS_CHARINFO_DIR	"/usr/local/var/charinfo"
#define D2CS_CHARSAVE_DIR	"/usr/local/var/charsave"
#define D2CS_BAK_CHARINFO_DIR	"/usr/local/var/bak/charinfo"
#define D2CS_BAK_CHARSAVE_DIR	"/usr/local/var/bak/charsave"
#define D2CS_CHARSAVE_NEWBIE	"/usr/local/var/files/newbie.save"
#define D2CS_TRANS_FILE		"/usr/local/etc/address_translation.conf"
#define D2CS_LADDER_DIR		"/usr/local/var/ladders"

#define D2CS_MOTD		"No MOTD yet"

#define LIST_PURGE_INTERVAL		300
#define GAMEQUEUE_CHECK_INTERVAL	60

#define MAX_GAME_LIST			20

#endif
