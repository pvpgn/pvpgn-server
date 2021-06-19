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
#include "smtp.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

#include <curl/curl.h>
#include <fmt/core.h>
#include <fmt/chrono.h>

#include "compat/localtime_s.h"

#include "common/eventlog.h"
#include "common/xalloc.h"

#include "prefs.h"
#include "server.h"

#include "common/setup_after.h"

#define SMTP_TIMEOUT_DEFAULT 1000

namespace pvpgn
{

	namespace bnetd
	{

		static bool is_curl_initialized = false;
		static std::thread smtp_thread;
		static std::array<std::tuple<CURLM*, std::mutex>, 2> handles_and_mutexes = {};
		static std::atomic<bool> need_to_download_ca_cert_store(false);

		// smtp data
		static std::string smtp_ca_cert_store;
		static std::string smtp_server_url;
		static long smtp_port;
		static std::string smtp_username;
		static std::string smtp_password;

		typedef struct
		{
			std::string message;
			std::size_t bytes_remaining;
		} read_callback_message;


		// Callback function for CURLOPT_WRITEFUNCTION.
		static std::size_t write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
		{
			try
			{
				const std::size_t total_size = size * nmemb;
				std::string* buffer = static_cast<std::string*>(userdata);
				buffer->append(ptr, ptr + total_size);

				return total_size;
			}
			catch (const std::exception& e)
			{
				return 0;
			}
		}

		static void smtp_consumer()
		{
			while (is_curl_initialized == true)
			{
				// check if a new ca cert store file needs to be downloaded
				if (need_to_download_ca_cert_store.load() == true)
				{
					need_to_download_ca_cert_store.store(false);

					CURL* curl_handle = curl_easy_init();

					curl_easy_setopt(curl_handle, CURLOPT_URL, prefs_get_smtp_ca_cert_store_remote_url());
					curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
					curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
					curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
					curl_easy_setopt(curl_handle, CURLOPT_CAINFO, smtp_ca_cert_store.c_str());
					std::string buffer;
					curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(&buffer));
					CURLcode c = curl_easy_perform(curl_handle);
					curl_easy_cleanup(curl_handle);

					std::ofstream ca_cert_store_file(prefs_get_smtp_ca_cert_store_file(), std::ios::binary);
					if (ca_cert_store_file.is_open())
					{
						ca_cert_store_file << buffer;
					}
				}

				// now handle pending curl easy handles
				for (auto& tuple : handles_and_mutexes)
				{
					CURLM* curl_multi_handle = std::get<0>(tuple);
					std::mutex& curl_multi_handle_mutex = std::get<1>(tuple);

					if (curl_multi_handle_mutex.try_lock() == false)
					{
						continue;
					}

					long timeout = -1;
					curl_multi_timeout(curl_multi_handle, &timeout);
					if (timeout == -1)
					{
						timeout = SMTP_TIMEOUT_DEFAULT;
					}

					curl_multi_wait(curl_multi_handle, nullptr, 0, timeout, nullptr);

					int running_handles = 0; // unused
					curl_multi_perform(curl_multi_handle, &running_handles);

					// After calling curl_multi_perform(), free all curl easy handles that are done.
					{
						struct CURLMsg* curlmsg;
						do
						{
							int msgq = 0;
							curlmsg = curl_multi_info_read(curl_multi_handle, &msgq);
							if (curlmsg && (curlmsg->msg == CURLMSG_DONE))
							{
								CURL* curl = curlmsg->easy_handle;

								running_handles -= 1;
	
								curl_multi_remove_handle(curl_multi_handle, curl);

								struct curl_slist* recipient = nullptr;
								curl_easy_getinfo(curl, CURLINFO_PRIVATE, &recipient);
								if (recipient != nullptr)
								{
									curl_slist_free_all(recipient);
								}

								curl_easy_cleanup(curl);
							}
						} while (curlmsg);
					}

					curl_multi_handle_mutex.unlock();
				}
				
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
		}

		// This function will be called at least twice for every message, the last call must return 0.
		static std::size_t read_callback(char* buffer, std::size_t size, std::size_t nitems, void* message)
		{
			read_callback_message* rcbmessage = static_cast<read_callback_message*>(message);
			std::size_t buffer_size = size * nitems;
			// Copy at least (rcbmessage->bytes_remaining) bytes and at most (buffer_size) bytes
			std::size_t copy_size = rcbmessage->bytes_remaining <= buffer_size ? rcbmessage->bytes_remaining : buffer_size;

			if (copy_size > 0)
			{
				std::memcpy(buffer, rcbmessage->message.c_str(), copy_size);
				rcbmessage->bytes_remaining -= copy_size;
			}

			if (copy_size == 0)
			{
				delete rcbmessage;
			}

			return copy_size;
		}

		/**
		* Initializes smtp_server_url, smtp_port, smtp_username, and smtp_password from the four function parameters.
		* Will return false if prefs_smtp_port is greater than 65535.
		*/
		static bool smtp_config(const char* prefs_smtp_ca_cert_store, const char* prefs_smtp_server_url, unsigned int prefs_smtp_port, const char* prefs_smtp_username, const char* prefs_smtp_password)
		{
			if (prefs_smtp_ca_cert_store == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Received NULL prefs_smtp_ca_cert_store");
				return false;
			}

			if (prefs_smtp_server_url == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Received NULL prefs_smtp_server_url");
				return false;
			}

			// ports are 16 bit integers, so they should never be above UINT16_MAX (65,535)
			if (prefs_smtp_port > UINT16_MAX)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Received out-of-range port number ({})", prefs_smtp_port);
				return false;
			}

			if (prefs_smtp_username == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Received NULL prefs_smtp_username");
				return false;
			}

			if (prefs_smtp_password == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Received NULL prefs_smtp_password");
				return false;
			}

			smtp_ca_cert_store = prefs_smtp_ca_cert_store;
			smtp_server_url = fmt::format("smtp://{}", prefs_smtp_server_url);
			smtp_port = prefs_smtp_port;
			smtp_username = prefs_smtp_username;
			smtp_password = prefs_smtp_password;

			return true;
		}

		/**
		* Initializes libcurl's global context if it hasn't already been initialized. 
		* There must only be exactly one call to smtp_init() and smtp_cleanup().
		*
		* On success, returns true.
		* On failure, returns false. Will fail if libcurl couldn't initialize global context.
		*/
		bool smtp_init(const char* prefs_smtp_ca_cert_store, const char* prefs_smtp_server_url, unsigned int prefs_smtp_port, const char* prefs_smtp_username, const char* prefs_smtp_password)
		{
			if (is_curl_initialized)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "libcurl has already been initialized");
				return false;
			}

			if (smtp_config(prefs_smtp_ca_cert_store, prefs_smtp_server_url, prefs_smtp_port, prefs_smtp_username, prefs_smtp_password) == false)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to set SMTP data");
				return false;
			}

			for (auto& tuple : handles_and_mutexes)
			{
				CURLM** curl_multi_handle = &std::get<0>(tuple);
				*curl_multi_handle = curl_multi_init();
				if (*curl_multi_handle == nullptr)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "Failed to initialize curl multi handle");
					return false;
				}
			}

			if (curl_global_init(CURL_GLOBAL_NOTHING) != 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to initialize curl global context");
				return false;
			}

			smtp_thread = std::thread(smtp_consumer);

			is_curl_initialized = true;

			return true;
		}

		bool smtp_reconfig(const char* prefs_smtp_ca_cert_store, const char* prefs_smtp_server_url, unsigned int prefs_smtp_port, const char* prefs_smtp_username, const char* prefs_smtp_password)
		{
			return smtp_config(prefs_smtp_ca_cert_store, prefs_smtp_server_url, prefs_smtp_port, prefs_smtp_username, prefs_smtp_password);
		}

		void smtp_cleanup()
		{
			if (is_curl_initialized)
			{
				is_curl_initialized = false;

				// wait for the smtp thread to finish executing
				smtp_thread.join();

				// free all the curl multi handles
				for (auto& tuple : handles_and_mutexes)
				{
					CURLM* curl_multi_handle = std::get<0>(tuple);
					curl_multi_cleanup(curl_multi_handle);
				}

				// free curl's global context
				curl_global_cleanup();
			}
		}

		void smtp_send_email(const std::string& to_address, const std::string& from_address, const std::string& from_name, const std::string& subject, std::string message)
		{
			if (!is_curl_initialized)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "libcurl not initialized, returning without attempting to send email");
				return;
			}

			CURL* curl = curl_easy_init();
			if (curl == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to initialize CURL easy handle");
				return;
			}

			// direct curl to use TLS
			curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
			curl_easy_setopt(curl, CURLOPT_CAINFO, smtp_ca_cert_store.c_str());

			// set smtp server connection information
			curl_easy_setopt(curl, CURLOPT_URL, smtp_server_url.c_str());
			curl_easy_setopt(curl, CURLOPT_PORT, smtp_port);
			curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_username.c_str());
			curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_password.c_str());
			
			// set 'from' address
			curl_easy_setopt(curl, CURLOPT_MAIL_FROM, fmt::format("<{}>", from_address).c_str());

			// set 'to' address
			struct curl_slist* recipient = curl_slist_append(nullptr, fmt::format("<{}>", to_address).c_str());
			if (recipient == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to append recipient address to recipient list");
				return;
			}
			curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipient);

			// store pointer to recipient struct in the handle and retrieve it later to free the memory
			curl_easy_setopt(curl, CURLOPT_PRIVATE, reinterpret_cast<void*>(recipient));

			// set function for curl to call when it wants to read the message into curl's desired buffer
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

			// prepend email headers to the message
			std::string current_time_str;
			{
				std::time_t now = (std::time_t)(-1);
				std::time(&now);
				struct std::tm tmnow = {};

				if (now != (std::time_t)(-1) && pvpgn::localtime_s(&now, &tmnow) != nullptr)
				{
					current_time_str = fmt::format("Date: {:%a, %d %b %Y %T %z}\r\n", tmnow);
				}
			}
			message.insert(0, fmt::format("MIME-Version: 1.0\r\nContent-Type: text/plain; charset=\"UTF-8\"\r\n{}From: {} <{}>\r\nTo: <{}>\r\nSubject: {}\r\n\r\n", current_time_str, from_name, from_address, to_address, subject));

			// this is the pointer that will be passed in to read_callback().
			// passing in a pointer to the message alone is not sufficient because read_callback() is called by curl at least twice.
			// the buffer provided by curl may not be sufficiently large enough for read_callback() to copy the entire message into during a single call.
			// therefore read_callback() needs a way to keep track of the number of bytes it still needs to copy from the message.
			read_callback_message* rcbmessage = new read_callback_message;
			rcbmessage->message = message;
			rcbmessage->bytes_remaining = message.length() + 1;
			curl_easy_setopt(curl, CURLOPT_READDATA, static_cast<void*>(rcbmessage));

			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

			// lock an available mutex and then add the curl easy handle to the associated curl multi handle
			while (true)
			{
				for (auto& tuple : handles_and_mutexes)
				{
					std::mutex& curl_multi_handle_mutex = std::get<1>(tuple);

					if (curl_multi_handle_mutex.try_lock() == false)
					{
						continue;
					}

					CURLM* curl_multi_handle = std::get<0>(tuple);
					CURLMcode code = curl_multi_add_handle(curl_multi_handle, curl);
					if (code == CURLM_OK)
					{
						eventlog(eventlog_level_trace, __FUNCTION__, "Added handle to CURL multi handle ({})", curl_multi_handle);
					}
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "Failed to add handle to CURL multi handle (CURLMcode: {})", code);
					}

					curl_multi_handle_mutex.unlock();

					return;
				}
			}

		}

		// The actual logic is in smtp_consumer()
		bool download_ca_cert_store()
		{
			need_to_download_ca_cert_store.store(true);

			return true;
		}

	}

}
