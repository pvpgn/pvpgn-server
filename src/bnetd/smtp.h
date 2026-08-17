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

#include <string>

namespace pvpgn
{

	namespace bnetd
	{

		bool smtp_init(const char* prefs_smtp_ca_cert_store, const char* prefs_smtp_server_url, unsigned int prefs_smtp_port, const char* prefs_smtp_username, const char* prefs_smtp_password);
		bool smtp_reconfig(const char* prefs_smtp_ca_cert_store, const char* prefs_smtp_server_url, unsigned int prefs_smtp_port, const char* prefs_smtp_username, const char* prefs_smtp_password);
		void smtp_cleanup();
		void smtp_send_email(const std::string& to_address, const std::string& from_address, const std::string& from_name, const std::string& subject, std::string message);

		bool download_ca_cert_store();

	}

}