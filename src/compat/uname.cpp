/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
 *
 * win32 part based on win32-uname.c from libguile
 *
 */
#include "common/setup_before.h"
#ifndef HAVE_UNAME

#include <cstring>
#include <cerrno>
#ifdef WIN32
# include <cstdio>
# include <windows.h>
#endif
#include "uname.h"
#include "common/setup_after.h"


namespace pvpgn
{

	extern int uname(struct utsname * buf)
	{
		if (!buf)
		{
			errno = EFAULT;
			return -1;
		}

#ifdef WIN32
		{
			enum { WinNT, Win95, Win98, WinUnknown };
			OSVERSIONINFOEX osver;
			SYSTEM_INFO sysinfo;
			DWORD sLength;
			DWORD os = WinUnknown;


			osver.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
			GetVersionEx((OSVERSIONINFO*)&osver);
			GetSystemInfo(&sysinfo); 

			// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724833(v=vs.85).aspx
			switch (osver.dwPlatformId)
			{
			case VER_PLATFORM_WIN32_NT:
				if (osver.dwMajorVersion == 4)
					std::strcpy(buf->sysname, "Windows NT4x");
				else if (osver.dwMajorVersion <= 3)
					std::strcpy(buf->sysname, "Windows NT3x");
				else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion < 1)
					std::strcpy(buf->sysname, "Windows 2000");
				else if (osver.dwMajorVersion == 5 && (osver.dwMinorVersion == 1 // x86
					|| (osver.dwMinorVersion == 2 && (osver.wProductType == VER_NT_WORKSTATION) && (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))) ) // x64
					std::strcpy(buf->sysname, "Windows XP");
				else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)
					std::strcpy(buf->sysname, "Windows Server 2003");
				else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0 && (osver.wProductType == VER_NT_WORKSTATION))
					std::strcpy(buf->sysname, "Windows Vista");
				else if (osver.dwMajorVersion == 6 && (osver.dwMinorVersion <= 1) && (osver.wProductType != VER_NT_WORKSTATION))
					std::strcpy(buf->sysname, "Windows Server 2008");
				else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1 && (osver.wProductType == VER_NT_WORKSTATION))
					std::strcpy(buf->sysname, "Windows 7");
				else if (osver.dwMajorVersion == 6 && (osver.dwMinorVersion >= 2) && (osver.wProductType != VER_NT_WORKSTATION))
					std::strcpy(buf->sysname, "Windows Server 2012");
				else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion >= 2 && (osver.wProductType == VER_NT_WORKSTATION))
					std::strcpy(buf->sysname, "Windows 8.x");
				// FIXME: dwMinorVersion returns "2" instead of "3" on Windows 8.1
				//else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 3 && (osver.wProductType == VER_NT_WORKSTATION))
				//	std::strcpy(buf->sysname, "Windows 8.1");
				else if (osver.wProductType != VER_NT_WORKSTATION)
					std::strcpy(buf->sysname, "Windows Server >2012");
				else
					std::strcpy(buf->sysname, "Windows >8");

				os = WinNT;
				break;

			case VER_PLATFORM_WIN32_WINDOWS: /* Win95, Win98 or WinME */
				if ((osver.dwMajorVersion > 4) ||
					((osver.dwMajorVersion == 4) && (osver.dwMinorVersion > 0)))
				{
					if (osver.dwMinorVersion >= 90)
						std::strcpy(buf->sysname, "Windows ME"); /* ME */
					else
						std::strcpy(buf->sysname, "Windows 98"); /* 98 */
					os = Win98;
				}
				else{
					std::strcpy(buf->sysname, "Windows 95"); /* 95 */
					os = Win95;
				}

				break;

			case VER_PLATFORM_WIN32s: /* Windows 3.x */
				std::strcpy(buf->sysname, "Windows");
				break;

			default:
				std::strcpy(buf->sysname, "Win32");
				break;
			}

			std::sprintf(buf->version, "%ld.%02ld", osver.dwMajorVersion, osver.dwMinorVersion);

			if (osver.szCSDVersion[0] != '\0' &&
				(std::strlen(osver.szCSDVersion) + std::strlen(buf->version) + 1) <
				sizeof (buf->version))
			{
				std::strcat(buf->version, " ");
				std::strcat(buf->version, osver.szCSDVersion);
			}

			std::sprintf(buf->release, "build %ld", osver.dwBuildNumber & 0xFFFF);

			switch (sysinfo.wProcessorArchitecture)
			{
			case PROCESSOR_ARCHITECTURE_PPC:
				std::strcpy(buf->machine, "ppc");
				break;
			case PROCESSOR_ARCHITECTURE_ALPHA:
				std::strcpy(buf->machine, "alpha");
				break;
			case PROCESSOR_ARCHITECTURE_MIPS:
				std::strcpy(buf->machine, "mips");
				break;
			case PROCESSOR_ARCHITECTURE_INTEL:
				/*
				 * dwProcessorType is only valid in Win95 and Win98 and WinME
				 * wProcessorLevel is only valid in WinNT
				 */
				switch (os)
				{
				case Win95:
				case Win98:
					switch (sysinfo.dwProcessorType)
					{
					case PROCESSOR_INTEL_386:
					case PROCESSOR_INTEL_486:
					case PROCESSOR_INTEL_PENTIUM:
						std::sprintf(buf->machine, "i%ld", sysinfo.dwProcessorType);
						break;
					default:
						std::strcpy(buf->machine, "i386");
						break;
					}
					break;
				case WinNT:
					std::sprintf(buf->machine, "i%d86", sysinfo.wProcessorLevel);
					break;
				default:
					std::strcpy(buf->machine, "unknown");
					break;
				}
				break;
			default:
				std::strcpy(buf->machine, "unknown");
				break;
			}

			sLength = sizeof (buf->nodename) - 1;
			GetComputerName(buf->nodename, &sLength);
		}
#else
		std::strcpy(buf->sysname, "unknown");
		std::strcpy(buf->version, "unknown");
		std::strcpy(buf->release, "unknown");
		std::strcpy(buf->machine, "unknown");
		std::strcpy(buf->nodename, "unknown");
#endif
		std::strcpy(buf->domainname, "");

		return 0;
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
