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

#ifdef WIN32

#include <cstring>
#include <windows.h>
#include <winsvc.h>

#if !defined(WINADVAPI)
#if !defined(_ADVAPI32_)
#define WINADVAPI DECLSPEC_IMPORT
#else
#define WINADVAPI
#endif
#endif

extern char serviceLongName[];
extern char serviceName[];
extern char serviceDescription[];

extern int g_ServiceStatus;

#ifdef WIN32_GUI
extern int app_main(int argc, char ** argv);
#else
extern int main(int argc, char ** argv);
#endif

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;

typedef WINADVAPI BOOL(WINAPI *CSD_T)(SC_HANDLE, DWORD, LPCVOID);

void Win32_ServiceInstall()
{
	SERVICE_DESCRIPTION sdBuf;
	CSD_T ChangeServiceDescription;
	HMODULE advapi32;
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);

	if (serviceControlManager)
	{
		char path[_MAX_PATH + 10];
		if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0)
		{
			SC_HANDLE service;
			std::strcat(path, " --service");
			service = CreateService(serviceControlManager,
				serviceName, serviceLongName,
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
				0, 0, 0, 0, 0);
			if (service)
			{
				sdBuf.lpDescription = serviceDescription;

				if (!(advapi32 = GetModuleHandle("ADVAPI32.DLL")))
				{
					CloseServiceHandle(service);
					CloseServiceHandle(serviceControlManager);
					return;
				}

				if (!(ChangeServiceDescription = (CSD_T)GetProcAddress(advapi32, "ChangeServiceConfig2A")))
				{
					CloseServiceHandle(service);
					CloseServiceHandle(serviceControlManager);
					return;
				}

				ChangeServiceDescription(
					service,                // handle to service  
					SERVICE_CONFIG_DESCRIPTION, // change: description  
					&sdBuf);
				CloseServiceHandle(service);
			}
		}
		CloseServiceHandle(serviceControlManager);
	}
}

void Win32_ServiceUninstall()
{
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

	if (serviceControlManager)
	{
		SC_HANDLE service = OpenService(serviceControlManager,
			serviceName, SERVICE_QUERY_STATUS | DELETE);
		if (service)
		{
			SERVICE_STATUS serviceStatus;
			if (QueryServiceStatus(service, &serviceStatus))
			{
				if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
					DeleteService(service);
			}
			//DeleteService(service);

			CloseServiceHandle(service);
		}

		CloseServiceHandle(serviceControlManager);
	}

}

void WINAPI ServiceControlHandler(DWORD controlCode)
{
	switch (controlCode)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;

	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		g_ServiceStatus = 0;
		return;

	case SERVICE_CONTROL_PAUSE:
		g_ServiceStatus = 2;
		serviceStatus.dwCurrentState = SERVICE_PAUSED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		break;

	case SERVICE_CONTROL_CONTINUE:
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		g_ServiceStatus = 1;
		break;

	default:
		if (controlCode >= 128 && controlCode <= 255)
			// user defined control code
			break;
		else
			// unrecognized control code
			break;
	}

	SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI ServiceMain(DWORD argc, char *argv[])
{
	// initialise service status
	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_PAUSE_CONTINUE;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, ServiceControlHandler);

	if (serviceStatusHandle)
	{
		char path[_MAX_PATH + 1];
		unsigned int i, last_slash = 0;

		GetModuleFileName(0, path, sizeof(path) / sizeof(path[0]));

		for (i = 0; i < std::strlen(path); i++) {
			if (path[i] == '\\') last_slash = i;
		}

		path[last_slash] = 0;

		// service is starting
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		// do initialisation here
		SetCurrentDirectory(path);

		// running
		serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		////////////////////////
		// service main cycle //
		////////////////////////

		g_ServiceStatus = 1;
		argc = 1;


#ifdef WIN32_GUI
		app_main(argc, argv);
#else
		main(argc, argv);
#endif



		// service was stopped
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);

		// do cleanup here

		// service is now stopped
		serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
	}
}

void Win32_ServiceRun()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ serviceName, ServiceMain },
		{ 0, 0 }
	};

	if (!StartServiceCtrlDispatcher(serviceTable))
	{
	}
}

#endif
