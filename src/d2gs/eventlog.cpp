#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "d2gs.h"
#include "vars.h"
#include "eventlog.h"


namespace pvpgn
{

	namespace d2gs
	{

		static FILE* eventstrm = NULL;		/* general log stream */
		static FILE* gestrm = NULL;			/* for game engine */

		/* the following var used in hexdump.c, for debuging */
		extern FILE* hexstrm;


		/*********************************************************************
		 * Purpose: to initialize the envet log system
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int D2GSEventLogInitialize(void)
		{
			eventstrm = fopen("d2gs.log", "a");
			gestrm = fopen("d2ge.log", "a");

			/* for debug, used by hexdump.c */
			hexstrm = fopen("debug.log", "a");
			if (!hexstrm) return FALSE;

			if (eventstrm && gestrm) return TRUE;
			else return FALSE;

		} /* End of D2GSEventLogInitialize() */


		/*********************************************************************
		 * Purpose: to cleanup the envet log system
		 * Return: none
		 *********************************************************************/
		void D2GSEventLogCleanup(void)
		{
			if (eventstrm) fclose(eventstrm);
			if (gestrm) fclose(gestrm);

#ifdef DEBUG
			if (hexstrm) fclose(hexstrm);
#endif

			hexstrm = eventstrm = gestrm = NULL;

			return;

		} /* End of D2GSEventLogCleanup() */


		/*********************************************************************
		 * Purpose: to log the event to the file or the standard output
		 * Return: none
		 *********************************************************************/
		void D2GSEventLog(char const* module, char const* fmt, ...)
		{
			va_list		args;
			char		time_string[EVENT_TIME_MAXLEN];
			SYSTEMTIME	st;

			if (!eventstrm) return;

			GetLocalTime(&st);
			sprintf(time_string, "%02d/%02d %02d:%02d:%02d.%03d", st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			if (!module)
			{
				fprintf(eventstrm, "%s eventlog: got NULL module\n", time_string);
				fflush(eventstrm);
#ifdef DEBUG_ON_CONSOLE
				fprintf(stdout, "%s eventlog: got NULL module\n", time_string);
#endif
				return;
			}
			if (!fmt)
			{
				fprintf(eventstrm, "%s eventlog: got NULL fmt\n", time_string);
				fflush(eventstrm);
#ifdef DEBUG_ON_CONSOLE
				fprintf(stdout, "%s eventlog: got NULL fmt\n", time_string);
#endif
				return;
			}

			fprintf(eventstrm, "%s %s: ", time_string, module);
#ifdef DEBUG_ON_CONSOLE
			fprintf(stdout, "%s %s: ", time_string, module);
#endif
			va_start(args, fmt);
			vfprintf(eventstrm, fmt, args);
#ifdef DEBUG_ON_CONSOLE
			vfprintf(stdout, fmt, args);
#endif
			va_end(args);
			fprintf(eventstrm, "\n");
			fflush(eventstrm);
#ifdef DEBUG_ON_CONSOLE
			fprintf(stdout, "\n");
#endif
			return;

		} /* End of D2GSEventLog() */


		/*********************************************************************
		 * Purpose: to log the event to the file or the standard output, for GE
		 * Return: none
		 *********************************************************************/
		void D2GEEventLog(char const* module, char const* fmt, ...)
		{
			va_list		args;
			char		time_string[EVENT_TIME_MAXLEN];
			SYSTEMTIME	st;

			if (!d2gsconf.enablegelog) return;
			if (!gestrm) return;

			GetLocalTime(&st);
			sprintf(time_string, "%02d/%02d %02d:%02d:%02d.%03d", st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			if (!module)
			{
				fprintf(gestrm, "%s eventlog: got NULL module\n", time_string);
				fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
				fprintf(stdout, "%s eventlog: got NULL module\n", time_string);
#endif
				return;
			}
			if (!fmt)
			{
				fprintf(gestrm, "%s eventlog: got NULL fmt\n", time_string);
				fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
				fprintf(stdout, "%s eventlog: got NULL fmt\n", time_string);
#endif
				return;
			}

			fprintf(gestrm, "%s  (*)%s: ", time_string, module);
#ifdef DEBUG_ON_CONSOLE
			fprintf(stdout, "%s  (*)%s: ", time_string, module);
#endif
			va_start(args, fmt);
			vfprintf(gestrm, fmt, args);
#ifdef DEBUG_ON_CONSOLE
			vfprintf(stdout, fmt, args);
#endif
			va_end(args);
			fprintf(gestrm, "\n");
			fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
			fprintf(stdout, "\n");
#endif
			return;

		} /* End of D2GEEventLog() */


		/*********************************************************************
		 * Purpose: to log the argument
		 * Return: none
		 *********************************************************************/
		void LogAP(LPCSTR lpModule, LPCSTR lpFormat, va_list ap)
		{
			SYSTEMTIME	st;
			DWORD		len;
			char		msg[MAX_LINE_LEN];
			char		tmp[MAX_LINE_LEN];

			if (!d2gsconf.enablegemsg) return;
			if (!lpModule || !lpFormat)  return;
			GetLocalTime(&st);
			len = wsprintf(msg, "%02d/%02d %02d:%02d:%02d.%03d", st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			len += vsprintf(tmp, lpFormat, ap);
			strcat(msg, tmp);
			strcat(msg, "\n");
			len += strlen("\n");
			fprintf(gestrm, "%s", msg);
			fflush(gestrm);
#ifdef DEBUG_ON_CONSOLE
			fprintf(stdout, "%s", msg);
#endif
			return;

		} /* End of LogAP */


		/*********************************************************************
		 * Purpose: to dump the portrait of a char
		 * Return: none
		 *********************************************************************/
		void PortraitDump(LPCSTR lpAccountName, LPCSTR lpCharName, LPCSTR lpCharPortrait)
		{
#ifndef DEBUG

			return;

#else
			FILE* fp;
			char	filename[MAX_PATH];

			if (!lpAccountName || !lpCharName) return;
			sprintf(filename, "portrait\\%s", lpCharName);
			fp = fopen(filename, "wb");
			if (!fp) return;
			fwrite(lpCharPortrait, 1, strlen(lpCharPortrait) + 1, fp);
			fclose(fp);
			return;

#endif

		} /* End of PortraitDump() */

	}

}