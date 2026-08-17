/*
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
#include "setup.h"
#include "handle_d2gs.h"

#include <cstdint>
#include <fstream>
#include <functional>
#include <type_traits>
#include <unordered_map>

#include <nonstd/optional.hpp>

#include "common/addr.h"
#include "common/bn_type.h"
#include "common/d2char_checksum.h"
#include "common/d2cs_d2gs_character.h"
#include "common/d2dbs_d2gs_protocol.h"
#include "common/eventlog.h"
#include "common/packet.h"
#include "common/xalloc.h"
#include "common/xstring.h"

#include "compat/mkdir.h"
#include "compat/rename.h"

#include "charlock.h"
#include "connection.h"
#include "d2ladder.h"
#include "prefs.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace d2dbs
	{

		static unsigned int dbs_packet_savedata_charsave(t_d2dbs_connection* c, const char* account_name, const char* char_name, const char* data, unsigned int datalen);
		static unsigned int dbs_packet_savedata_charinfo(t_d2dbs_connection* c, const char* account_name, const char* char_name, const char* data, unsigned int datalen);
		static nonstd::optional<std::vector<std::uint8_t>> dbs_packet_getdata_charsave(t_d2dbs_connection* c, const char* account_name, const char* char_name);
		static nonstd::optional<std::vector<std::uint8_t>> dbs_packet_getdata_charinfo(t_d2dbs_connection* c, const char* account_name, const char* char_name);

		static int dbs_packet_savedata(t_d2dbs_connection* c, const t_packet* const packet);
		static int dbs_packet_getdata(t_d2dbs_connection* c, const t_packet* const packet);
		static int dbs_packet_updateladder(t_d2dbs_connection* c, const t_packet* const packet);
		static int dbs_packet_charlock(t_d2dbs_connection* c, const t_packet* const packet);
		static int dbs_packet_echoreply(t_d2dbs_connection* c, const t_packet* const packet);

		static int dbs_packet_fix_charinfo(t_d2dbs_connection* c, const char* AccountName, const char* CharName, const char* charsave);
		static void dbs_packet_set_charinfo_level(const char* char_name, char* charinfo);


		std::unordered_map<decltype(packet_get_type(std::declval<t_packet*>())), std::function<int(t_d2dbs_connection*, const t_packet* const packet)>> d2gs_packet_table = {
			{ D2GS_D2DBS_SAVE_DATA_REQUEST, dbs_packet_savedata },
			{ D2GS_D2DBS_GET_DATA_REQUEST, dbs_packet_getdata },
			{ D2GS_D2DBS_UPDATE_LADDER, dbs_packet_updateladder },
			{ D2GS_D2DBS_CHAR_LOCK, dbs_packet_charlock },
			{ D2DBS_D2GS_ECHOREQUEST, dbs_packet_echoreply }
		};


		/*
		* return value:
		* 1  :  process one or more packet
		* 0  :  not get a whole packet,do nothing
		* -1 :  error
		*/
		int handle_d2gs_packet(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got NULL connection");
				return -1;
			}

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got NULL packet", c->sd);
				return -1;
			}

			if (packet_get_class(packet) != packet_class_d2dbs_d2gs)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad packet (class {})", c->sd, static_cast<std::underlying_type<t_packet_class>::type>(packet_get_class(packet)));
				return -1;
			}

			switch (conn_get_state(c))
			{
			case conn_state_loggedin:
				switch (d2gs_packet_table.at(packet_get_type(packet))(c, packet))
				{
				case 1:
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] unknown (logged in) d2gs packet type 0x{:04x}, len {}", c->sd, packet_get_type(packet), packet_get_size(packet));
					break;
				case -1:
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] (logged in) got error handling packet type 0x{:04x}, len {}", c->sd, packet_get_type(packet), packet_get_size(packet));
					break;
				};
				break;

			default:
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] invalid login state {}", c->sd, static_cast<std::underlying_type<t_conn_state>::type>(conn_get_state(c)));
			};

			return 1;
		}


		static unsigned int dbs_packet_savedata_charsave(t_d2dbs_connection* c, const char* account_name, const char* char_name, const char* data, unsigned int datalen)
		{
			char* AccountName = xstrdup(account_name);
			if (!AccountName)
			{
				return 0;
			}

			char* CharName = xstrdup(char_name);
			if (!CharName)
			{
				xfree(AccountName);
				return 0;
			}

			std::FILE* fd;
			int checksum_header;
			int checksum_calc;

			strtolower(AccountName);
			strtolower(CharName);

			//check if checksum is ok
			checksum_header = bn_int_get((bn_basic*)&data[D2CHARSAVE_CHECKSUM_OFFSET]);
			checksum_calc = d2charsave_checksum((unsigned char*)data, datalen, D2CHARSAVE_CHECKSUM_OFFSET);

			if (checksum_header != checksum_calc)
			{
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "received ({}) and calculated({}) checksum do not match - discarding charsave", checksum_header, checksum_calc);
				return 0;
			}


			std::string filename = fmt::format("{}/.{}.tmp", d2dbs_prefs_get_charsave_dir(), CharName);
			fd = std::fopen(filename.c_str(), "wb");
			if (!fd)
			{
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "open() failed : {}", filename);
				return 0;
			}

			std::size_t curlen = 0;
			std::size_t leftlen = datalen;
			while (curlen < datalen)
			{
				std::size_t writelen = leftlen > 2000 ? 2000 : leftlen;

				std::size_t readlen = std::fwrite(data + curlen, 1, writelen, fd);
				if (readlen <= 0)
				{
					std::fclose(fd);
					xfree(AccountName);
					xfree(CharName);
					eventlog(eventlog_level_error, __FUNCTION__, "write() failed error : {}", std::strerror(errno));
					return 0;
				}
				curlen += readlen;
				leftlen -= readlen;
			}
			std::fclose(fd);

			std::string bakfile = fmt::format("{}/{}", prefs_get_charsave_bak_dir(), CharName);
			std::string savefile = fmt::format("{}/{}", d2dbs_prefs_get_charsave_dir(), CharName);
			if (p_rename(savefile.c_str(), bakfile.c_str()) == -1)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "error std::rename {} to {}", savefile, bakfile);
			}
			if (p_rename(filename.c_str(), savefile.c_str()) == -1)
			{
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "error std::rename {} to {}", filename, savefile);
				return 0;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "saved charsave {}(*{}) for gs {}({})", CharName, AccountName, c->serverip, c->serverid);
			xfree(AccountName);
			xfree(CharName);
			return datalen;
		}

		static unsigned int dbs_packet_savedata_charinfo(t_d2dbs_connection* c, const char* account_name, const char* char_name, const char* data, unsigned int datalen)
		{
			char* AccountName = xstrdup(account_name);
			if (!AccountName)
			{
				return 0;
			}

			char* CharName = xstrdup(char_name);
			if (!CharName)
			{
				xfree(AccountName);
				return 0;
			}

			strtolower(AccountName);
			strtolower(CharName);

			std::string filepath = fmt::format("{}/{}", prefs_get_charinfo_bak_dir(), AccountName);
			{
				struct stat statbuf;
				if (stat(filepath.c_str(), &statbuf) == -1)
				{
					if (p_mkdir(filepath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0)
					{
						eventlog(eventlog_level_info, __FUNCTION__, "created charinfo directory: {}", filepath);
					}
					else
					{
						eventlog(eventlog_level_info, __FUNCTION__, "failed to create charinfo directory \"{}\" (errno: {})", filepath, errno);
						xfree(AccountName);
						xfree(CharName);
						return 0;
					}
				}
			}

			std::string filename = fmt::format("{}/{}/.{}.tmp", d2dbs_prefs_get_charinfo_dir(), AccountName, CharName);
			std::FILE* fd = std::fopen(filename.c_str(), "wb");
			if (!fd)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "open() failed : {}", filename);
				xfree(AccountName);
				xfree(CharName);
				return 0;
			}

			std::size_t curlen = 0;
			std::size_t leftlen = datalen;
			while (curlen < datalen)
			{
				std::size_t writelen = leftlen > 2000 ? 2000 : leftlen;

				std::size_t readlen = std::fwrite(data + curlen, 1, writelen, fd);
				if (readlen <= 0)
				{
					std::fclose(fd);
					xfree(AccountName);
					xfree(CharName);
					eventlog(eventlog_level_error, __FUNCTION__, "write() failed error : {}", std::strerror(errno));
					return 0;
				}
				curlen += readlen;
				leftlen -= readlen;
			}
			std::fclose(fd);

			std::string bakfile = fmt::format("{}/{}/{}", prefs_get_charinfo_bak_dir(), AccountName, CharName);
			std::string savefile = fmt::format("{}/{}/{}", d2dbs_prefs_get_charinfo_dir(), AccountName, CharName);
			if (p_rename(savefile.c_str(), bakfile.c_str()) == -1)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "error std::rename {} to {}", savefile, bakfile);
			}
			if (p_rename(filename.c_str(), savefile.c_str()) == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error std::rename {} to {}", filename, savefile);
				xfree(AccountName);
				xfree(CharName);
				return 0;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "saved charinfo {}(*{}) for gs {}({})", CharName, AccountName, c->serverip, c->serverid);
			xfree(AccountName);
			xfree(CharName);
			return datalen;
		}

		static nonstd::optional<std::vector<std::uint8_t>> dbs_packet_getdata_charsave(t_d2dbs_connection* c, const char* account_name, const char* char_name)
		{
			char* AccountName = xstrdup(account_name);
			if (!AccountName)
			{
				return nonstd::nullopt;
			}

			char* CharName = xstrdup(char_name);
			if (!CharName)
			{
				xfree(AccountName);
				return nonstd::nullopt;
			}

			strtolower(AccountName);
			strtolower(CharName);

			std::string filename = fmt::format("{}/{}", d2dbs_prefs_get_charsave_dir(), CharName);
			std::string filename_d2closed = fmt::format("{}/{}.d2s", d2dbs_prefs_get_charsave_dir(), CharName);
			if ((access(filename.c_str(), F_OK) < 0) && (access(filename_d2closed.c_str(), F_OK) == 0))
			{
				if (std::rename(filename_d2closed.c_str(), filename.c_str()) != 0)
				{
					xfree(AccountName);
					xfree(CharName);
					eventlog(eventlog_level_error, __FUNCTION__, "failed to rename file \"{}\" to \"{}\"", filename_d2closed, filename);
					return nonstd::nullopt;
				}
			}

			std::FILE* fd = std::fopen(filename.c_str(), "rb");
			if (!fd)
			{
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "open() failed : {}", filename);
				return nonstd::nullopt;
			}

			if (std::fseek(fd, 0, SEEK_END) != 0)
			{
				std::fclose(fd);
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "std::fseek() failed");
				return nonstd::nullopt;
			}

			long filesize = std::ftell(fd);
			if (filesize == -1L)
			{
				std::fclose(fd);
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "ftell() failed");
				return nonstd::nullopt;
			}

			std::rewind(fd);

			std::vector<std::uint8_t> charsave(filesize);
			std::size_t readlen = std::fread(charsave.data(), sizeof(decltype(charsave)::value_type), charsave.size(), fd);
			if (readlen < filesize)
			{
				std::fclose(fd);
				eventlog(eventlog_level_error, __FUNCTION__, "failed to read charinfo {}(*{}): read {} bytes, expected {} bytes", CharName, AccountName, readlen, filesize);
				xfree(AccountName);
				xfree(CharName);
				return nonstd::nullopt;
			}

			std::fclose(fd);
			eventlog(eventlog_level_info, __FUNCTION__, "loaded charsave {}(*{}) for gs {}({})", CharName, AccountName, c->serverip, c->serverid);
			xfree(AccountName);
			xfree(CharName);

			return charsave;
		}

		static nonstd::optional<std::vector<std::uint8_t>> dbs_packet_getdata_charinfo(t_d2dbs_connection* c, const char* account_name, const char* char_name)
		{
			char* AccountName = xstrdup(account_name);
			if (!AccountName)
			{
				return nonstd::nullopt;
			}

			char* CharName = xstrdup(char_name);
			if (!CharName)
			{
				xfree(AccountName);
				return nonstd::nullopt;
			}

			strtolower(AccountName);
			strtolower(CharName);

			std::string filename = fmt::format("{}/{}/{}", d2dbs_prefs_get_charinfo_dir(), AccountName, CharName);
			std::FILE* fd = std::fopen(filename.c_str(), "rb");
			if (!fd)
			{
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "open() failed : {}", filename);
				return nonstd::nullopt;
			}

			if (std::fseek(fd, 0, SEEK_END) != 0)
			{
				std::fclose(fd);
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "std::fseek() failed");
				return nonstd::nullopt;
			}

			long filesize = std::ftell(fd);
			if (filesize == -1)
			{
				std::fclose(fd);
				xfree(AccountName);
				xfree(CharName);
				eventlog(eventlog_level_error, __FUNCTION__, "std::ftell() failed");
				return nonstd::nullopt;
			}

			std::rewind(fd);

			std::vector<std::uint8_t> charinfo(filesize);
			std::size_t readlen = std::fread(charinfo.data(), sizeof(decltype(charinfo)::value_type), charinfo.size(), fd);
			if (readlen < filesize)
			{
				std::fclose(fd);
				eventlog(eventlog_level_error, __FUNCTION__, "failed to read charinfo {}(*{}): read {} bytes, expected {} bytes", CharName, AccountName, readlen, filesize);
				xfree(AccountName);
				xfree(CharName);
				return nonstd::nullopt;
			}

			std::fclose(fd);
			eventlog(eventlog_level_info, __FUNCTION__, "loaded charinfo {}(*{}) for gs {}({})", CharName, AccountName, c->serverip, c->serverid);
			xfree(AccountName);
			xfree(CharName);

			return charinfo;
		}

		static int dbs_packet_savedata(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (packet_get_size(packet) < sizeof(t_d2gs_d2dbs_save_data_request))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad savedata packet (expected {} bytes, got {})", c->sd, sizeof(t_d2gs_d2dbs_save_data_request), packet_get_size(packet));
				return -1;
			}

			const auto datatype = bn_short_get(packet->u.d2gs_d2dbs_save_data_request.datatype);
			const auto datalen = bn_short_get(packet->u.d2gs_d2dbs_save_data_request.datalen);

			const char* const account_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_save_data_request), MAX_USERNAME_LEN);
			if (!account_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad SAVEDATAREQUEST (missing or too long account name)", c->sd);
				return -1;
			}

			const char* const char_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_save_data_request) + std::strlen(account_name) + 1, MAX_CHARNAME_LEN);
			if (!char_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad SAVEDATAREQUEST (missing or too long char name)", c->sd);
				return -1;
			}

			const char* const realm_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_save_data_request) + std::strlen(account_name) + 1 + std::strlen(char_name) + 1, MAX_REALMNAME_LEN);
			if (!realm_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad SAVEDATAREQUEST (missing or too long realm name)", c->sd);
				return -1;
			}

			const void* const data = packet_get_data_const(packet, sizeof(t_d2gs_d2dbs_save_data_request) + std::strlen(account_name) + 1 + std::strlen(char_name) + 1 + std::strlen(realm_name) + 1, datalen);
			if (!data)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad SAVEDATAREQUEST (missing or too long data)", c->sd);
				return -1;
			}

			int result = D2DBS_SAVE_DATA_FAILED;
			if (datatype == D2GS_DATA_CHARSAVE)
			{
				if (dbs_packet_savedata_charsave(c, account_name, char_name, static_cast<const char*>(data), datalen) > 0 &&
					dbs_packet_fix_charinfo(c, account_name, char_name, static_cast<const char*>(data)))
				{
					result = D2DBS_SAVE_DATA_SUCCESS;
				}
				else
				{
					result = D2DBS_SAVE_DATA_FAILED;
				}
			}
			else if (datatype == D2GS_DATA_PORTRAIT)
			{
				char* modified_data = static_cast<char*>(xmalloc(datalen));
				if (!modified_data)
				{
					return -1;
				}

				std::memcpy(modified_data, data, datalen);

				/* if level is > 255 , sets level to 255 */
				dbs_packet_set_charinfo_level(char_name, modified_data);
				if (dbs_packet_savedata_charinfo(c, account_name, char_name, modified_data, datalen) > 0)
				{
					result = D2DBS_SAVE_DATA_SUCCESS;
				}
				else
				{
					result = D2DBS_SAVE_DATA_FAILED;
				}

				xfree(modified_data);
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown data type {}", datatype);
				return -1;
			}

			{
				t_packet* rpacket = packet_create(packet_class_d2dbs_d2gs);
				if (!rpacket)
				{
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_d2dbs_d2gs_save_data_reply));
				packet_set_type(rpacket, D2DBS_D2GS_SAVE_DATA_REPLY);

				bn_int_set(&rpacket->u.d2dbs_d2gs_save_data_reply.h.seqno, bn_int_get(packet->u.d2dbs_d2gs_save_data_reply.h.seqno));

				bn_int_set(&rpacket->u.d2dbs_d2gs_save_data_reply.result, result);
				bn_short_set(&rpacket->u.d2dbs_d2gs_save_data_reply.datatype, datatype);
				packet_append_string(rpacket, char_name);

				conn_push_outqueue(c, rpacket);

				packet_del_ref(rpacket);
			}

			return 0;
		}

		static int dbs_packet_echoreply(t_d2dbs_connection* c, const t_packet* const packet)
		{
			c->last_active = std::time(nullptr);

			return 0;
		}

		static int dbs_packet_getdata(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (packet_get_size(packet) < sizeof(t_d2gs_d2dbs_get_data_request))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST packet (expected {} bytes, got {})", c->sd, sizeof(t_d2gs_d2dbs_get_data_request), packet_get_size(packet));
				return -1;
			}

			const auto datatype = bn_short_get(packet->u.d2gs_d2dbs_save_data_request.datatype);

			const char* const account_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_get_data_request), MAX_USERNAME_LEN);
			if (!account_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long account name)", c->sd);
				return -1;
			}

			const char* const char_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_get_data_request) + std::strlen(account_name) + 1, MAX_CHARNAME_LEN);
			if (!char_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long char name)", c->sd);
				return -1;
			}

			const char* const realm_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_get_data_request) + std::strlen(account_name) + 1 + std::strlen(char_name) + 1, MAX_REALMNAME_LEN);
			if (!realm_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long realm name)", c->sd);
				return -1;
			}


			nonstd::optional<std::vector<std::uint8_t>> charinfo = nonstd::nullopt;
			nonstd::optional<std::vector<std::uint8_t>> charsave = nonstd::nullopt;
			unsigned int result;
			if (datatype == D2GS_DATA_CHARSAVE)
			{
				unsigned int gsid = 0;
				if (cl_query_charlock_status((unsigned char*)char_name, (unsigned char*)realm_name, &gsid) != 0)
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "char {}(*{})@{} is already locked on gs {}", char_name, account_name, realm_name, gsid);
					result = D2DBS_GET_DATA_CHARLOCKED;
				}
				else if (cl_lock_char((unsigned char*)char_name, (unsigned char*)realm_name, c->serverid) != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to lock char {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);
					result = D2DBS_GET_DATA_CHARLOCKED;
				}
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "lock char {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);

					charsave = dbs_packet_getdata_charsave(c, account_name, char_name);
					if (charsave.has_value())
					{
						charinfo = dbs_packet_getdata_charinfo(c, account_name, char_name);
						if (charinfo.has_value())
						{
							result = D2DBS_GET_DATA_SUCCESS;
						}
						else
						{
							result = D2DBS_GET_DATA_FAILED;

							if (cl_unlock_char((unsigned char*)char_name, (unsigned char*)realm_name, gsid) != 0)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "failed to unlock char {}(*{})@{} for gs {}({})", char_name, \
									account_name, realm_name, c->serverip, c->serverid);
							}
							else
							{
								eventlog(eventlog_level_info, __FUNCTION__, "unlock char {}(*{})@{} for gs {}({})", char_name, \
									account_name, realm_name, c->serverip, c->serverid);
							}
						}
					}
					else
					{
						result = D2DBS_GET_DATA_FAILED;

						if (cl_unlock_char((unsigned char*)char_name, (unsigned char*)realm_name, gsid) != 0)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "faled to unlock char {}(*{})@{} for gs {}({})", char_name, \
								account_name, realm_name, c->serverip, c->serverid);
						}
						else
						{
							eventlog(eventlog_level_info, __FUNCTION__, "unlock char {}(*{})@{} for gs {}({})", char_name, \
								account_name, realm_name, c->serverip, c->serverid);
						}

					}
				}
			}
			else if (datatype == D2GS_DATA_PORTRAIT)
			{
				charinfo = dbs_packet_getdata_charinfo(c, account_name, char_name);

				result = charinfo.has_value() ? D2DBS_GET_DATA_SUCCESS : D2DBS_GET_DATA_FAILED;
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown data type {}", datatype);
				return -1;
			}

			{
				t_packet* rpacket = packet_create(packet_class_d2dbs_d2gs);
				if (!rpacket)
				{
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_d2dbs_d2gs_get_data_reply));
				packet_set_type(rpacket, D2DBS_D2GS_GET_DATA_REPLY);

				bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.h.seqno, bn_int_get(packet->u.d2dbs_d2gs_get_data_reply.h.seqno));

				bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.result, result);
				if (datatype == D2GS_DATA_CHARSAVE && result == D2DBS_GET_DATA_SUCCESS && charinfo.has_value())
				{
					bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.charcreatetime, bn_int_get(reinterpret_cast<t_d2charinfo_file*>(charinfo.value().data())->header.create_time));

					// FIXME: this should be rewritten to support string formatted std::time
					if (bn_int_get(reinterpret_cast<t_d2charinfo_file*>(charinfo.value().data())->header.create_time) >= prefs_get_ladderinit_time())
					{
						bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.allowladder, 1);
					}
					else
					{
						bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.allowladder, 0);
					}
				}
				else
				{
					bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.charcreatetime, 0);
					bn_int_set(&rpacket->u.d2dbs_d2gs_get_data_reply.allowladder, 0);
				}
				bn_short_set(&rpacket->u.d2dbs_d2gs_get_data_reply.datatype, datatype);
				packet_append_string(rpacket, char_name);
				if (result == D2DBS_GET_DATA_SUCCESS)
				{
					if (datatype == D2GS_DATA_CHARSAVE)
					{
						if ((charsave.value().size() + packet_get_size(rpacket)) > MAX_PACKET_SIZE)
						{
							// Adding a charsave to the packet may cause the packet size to exceed MAX_PACKET_SIZE
							// In that case, set the values of everything in the packet except the 'data' (charsave) and send that out first
							// then create enough raw packets to send the charsave
							bn_short_set(&rpacket->u.d2dbs_d2gs_get_data_reply.datalen, charsave.value().size());
							conn_push_outqueue(c, rpacket);
							packet_del_ref(rpacket);
							rpacket = nullptr;

							std::size_t num_bytes_already_sent = 0;
							while (num_bytes_already_sent < charsave.value().size())
							{
								t_packet* rpacket = packet_create(packet_class_raw);
								if (!rpacket)
								{
									return -1;
								}

								std::size_t num_bytes_remaining = charsave.value().size() - num_bytes_already_sent;
								std::size_t num_bytes_to_send = num_bytes_remaining > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : num_bytes_remaining;
								std::memcpy(packet_get_raw_data_build(rpacket, 0), charsave.value().data() + num_bytes_already_sent, num_bytes_to_send);
								packet_set_size(rpacket, num_bytes_to_send);
								conn_push_outqueue(c, rpacket);
								packet_del_ref(rpacket);
								rpacket = nullptr;

								num_bytes_already_sent += num_bytes_to_send;
							}

							return 0;
						}
						else
						{
							bn_short_set(&rpacket->u.d2dbs_d2gs_get_data_reply.datalen, charsave.value().size());
							packet_append_data(rpacket, charsave.value().data(), charsave.value().size());
						}
					}
					else
					{
						bn_short_set(&rpacket->u.d2dbs_d2gs_get_data_reply.datalen, charinfo.value().size());
						packet_append_data(rpacket, charinfo.value().data(), charinfo.value().size());
					}
				}
				else
				{
					bn_short_set(&rpacket->u.d2dbs_d2gs_get_data_reply.datalen, 0);
				}


				conn_push_outqueue(c, rpacket);

				packet_del_ref(rpacket);
			}

			return 0;
		}

		static int dbs_packet_updateladder(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (packet_get_size(packet) < sizeof(t_d2gs_d2dbs_update_ladder))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad UPDATELADDER packet (expected {} bytes, got {})", c->sd, sizeof(t_d2gs_d2dbs_update_ladder), packet_get_size(packet));
				return -1;
			}

			const char* const char_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_update_ladder), MAX_CHARNAME_LEN);
			if (!char_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad UPDATELADDER (missing or too long char name)", c->sd);
				return -1;
			}

			const char* const realm_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_update_ladder) + std::strlen(char_name) + 1, MAX_REALMNAME_LEN);
			if (!realm_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad UPDATELADDER (missing or too long realm name)", c->sd);
				return -1;
			}

			t_d2ladder_info charladderinfo = {};
			std::strcpy(charladderinfo.charname, char_name);
			charladderinfo.experience = bn_int_get(packet->u.d2gs_d2dbs_update_ladder.charexplow);
			charladderinfo.level = bn_int_get(packet->u.d2gs_d2dbs_update_ladder.charlevel);
			charladderinfo.status = bn_short_get(packet->u.d2gs_d2dbs_update_ladder.charstatus);
			charladderinfo.chclass = bn_short_get(packet->u.d2gs_d2dbs_update_ladder.charclass);

			eventlog(eventlog_level_info, __FUNCTION__, "update ladder for {}@{} for gs {}({})", char_name, realm_name, c->serverip, c->serverid);
			d2ladder_update(&charladderinfo);

			return 0;
		}

		static int dbs_packet_charlock(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (packet_get_size(packet) < sizeof(t_d2gs_d2dbs_char_lock))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad CHARLOCK packet (expected {} bytes, got {})", c->sd, sizeof(t_d2gs_d2dbs_char_lock), packet_get_size(packet));
				return -1;
			}

			const char* const account_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_char_lock), MAX_USERNAME_LEN);
			if (!account_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long account name)", c->sd);
				return -1;
			}

			const char* const char_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_char_lock) + std::strlen(account_name) + 1, MAX_CHARNAME_LEN);
			if (!char_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long char name)", c->sd);
				return -1;
			}

			const char* const realm_name = packet_get_str_const(packet, sizeof(t_d2gs_d2dbs_char_lock) + std::strlen(account_name) + 1 + std::strlen(char_name) + 1, MAX_REALMNAME_LEN);
			if (!realm_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad GETDATAREQUEST (missing or too long realm name)", c->sd);
				return -1;
			}

			if (bn_int_get(packet->u.d2gs_d2dbs_char_lock.lockstatus))
			{
				if (cl_lock_char((unsigned char*)char_name, (unsigned char*)realm_name, c->serverid) != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to lock character {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);
				}
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "lock character {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);
				}
			}
			else
			{
				if (cl_unlock_char((unsigned char*)char_name, (unsigned char*)realm_name, c->serverid) != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to unlock character {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);
				}
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "unlock character {}(*{})@{} for gs {}({})", char_name, account_name, realm_name, c->serverip, c->serverid);
				}
			}

			return 0;
		}


		/*************************************************************************************/
#define CHARINFO_SIZE			0xC0
#define CHARINFO_PORTRAIT_LEVEL_OFFSET	0x89
#define CHARINFO_PORTRAIT_STATUS_OFFSET	0x8A
#define CHARINFO_SUMMARY_LEVEL_OFFSET	0xB8
#define CHARINFO_SUMMARY_STATUS_OFFSET	0xB4
#define CHARINFO_PORTRAIT_GFX_OFFSET	0x72
#define CHARINFO_PORTRAIT_COLOR_OFFSET	0x7E

#define CHARSAVE_LEVEL_OFFSET		0x2B
#define CHARSAVE_STATUS_OFFSET		0x24
#define CHARSAVE_GFX_OFFSET		0x88
#define CHARSAVE_COLOR_OFFSET		0x98

#define charstatus_to_portstatus(status) ((((status & 0xFF00) << 1) | (status & 0x00FF)) | 0x8080)
#define portstatus_to_charstatus(status) (((status & 0x7F00) >> 1) | (status & 0x007F))

		static void dbs_packet_set_charinfo_level(const char* char_name, char* charinfo)
		{
			if (prefs_get_difficulty_hack())
			{ /* difficulty hack enabled */
				unsigned int	level = bn_int_get((bn_basic*)&charinfo[CHARINFO_SUMMARY_LEVEL_OFFSET]);
				unsigned int	plevel = bn_byte_get((bn_basic*)&charinfo[CHARINFO_PORTRAIT_LEVEL_OFFSET]);

				/* levels 257 thru 355 */
				if (level != plevel)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "level mis-match for {} ( {} != {} ) setting to 255", char_name, level, plevel);
					bn_byte_set((bn_byte*)&charinfo[CHARINFO_PORTRAIT_LEVEL_OFFSET], 255);
					bn_int_set((bn_int*)&charinfo[CHARINFO_SUMMARY_LEVEL_OFFSET], 255);
				}
			}
		}

		static int dbs_packet_fix_charinfo(t_d2dbs_connection* c, const char* AccountName, const char* CharName, const char* charsave)
		{
			if (prefs_get_difficulty_hack())
			{
				unsigned int	level = bn_byte_get((bn_basic*)&charsave[CHARSAVE_LEVEL_OFFSET]);
				unsigned short	status = bn_short_get((bn_basic*)&charsave[CHARSAVE_STATUS_OFFSET]);
				unsigned short	pstatus = charstatus_to_portstatus(status);
				int		i;

				/*
				 * charinfo is only updated from level 1 to 99 (d2gs issue)
				 * from 100 to 256 d2gs does not send it
				 * when value rolls over (level 256 = 0)
				 * and charactar reaches level 257 (rolled over to level 1)
				 * d2gs starts sending it agian until level 356 (rolled over to 100)
				 * is reached agian. etc. etc. etc.
				 */
				if (level == 0) /* level 256, 512, 768, etc */
					level = 255;

				if (level < 100)
					return 1; /* d2gs will send charinfo - level will be set to 255 at that std::time if needed */

				eventlog(eventlog_level_info, __FUNCTION__, "level {} > 99 for {}", level, CharName);

				nonstd::optional<std::vector<std::uint8_t>> charinfo = dbs_packet_getdata_charinfo(c, AccountName, CharName);
				if (!charinfo.has_value())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "unable to get charinfo for {}", CharName);
					return 0;
				}

				/* if level in charinfo file is already set to 255,
				 * then is must have been set when d2gs sent the charinfo
				 * and got a level mis-match (levels 257 - 355)
				 * or level is actually 255. In eather case we set to 255
				 * this should work for any level mod
				 */
				if (bn_byte_get(&charinfo.value().data()[CHARINFO_PORTRAIT_LEVEL_OFFSET]) == 255)
					level = 255;

				eventlog(eventlog_level_info, __FUNCTION__, "updating charinfo for {} -> level = {} , status = 0x{:04X} , pstatus = 0x{:04X}", CharName, level, status, pstatus);
				bn_byte_set((bn_byte*)&charinfo.value().data()[CHARINFO_PORTRAIT_LEVEL_OFFSET], level);
				bn_int_set((bn_int*)&charinfo.value().data()[CHARINFO_SUMMARY_LEVEL_OFFSET], level);
				bn_short_set((bn_short*)&charinfo.value().data()[CHARINFO_PORTRAIT_STATUS_OFFSET], pstatus);
				bn_int_set((bn_int*)&charinfo.value().data()[CHARINFO_SUMMARY_STATUS_OFFSET], status);

				for (i = 0; i < 11; i++)
				{
					bn_byte_set((bn_byte*)&charinfo.value().data()[CHARINFO_PORTRAIT_GFX_OFFSET + i], bn_byte_get((bn_basic*)&charsave[CHARSAVE_GFX_OFFSET + i]));
					bn_byte_set((bn_byte*)&charinfo.value().data()[CHARINFO_PORTRAIT_COLOR_OFFSET + i], bn_byte_get((bn_basic*)&charsave[CHARSAVE_GFX_OFFSET + i]));
				}

				if (!(dbs_packet_savedata_charinfo(c, AccountName, CharName, reinterpret_cast<const char*>(charinfo.value().data()), charinfo.value().size())))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "unable to save charinfo for {}", CharName);
					return 0;
				}

				return 1; /* charinfo updated */
			}

			return 1; /* difficulty hack not enabled */
		}
	}
}