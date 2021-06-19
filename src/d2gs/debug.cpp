#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "d2gs.h"
#include "eventlog.h"
#include "hexdump.h"
#include "debug.h"
#include "vars.h"


#ifdef DEBUG

namespace pvpgn
{

	namespace d2gs
	{

		static DWORD	gdwTotalCount=0;
		static BOOL		DebugDumpParam(LPCSTR lpName, DWORD dwValue);

		extern void DebugEventCallback(char const * module, int count, ...)
		{
			SYSTEMTIME	st;
			va_list		ap;
			int		i,value;
			char		* name;
			char		function[MAX_LINE_LEN];

			if (!(d2gsconf.debugeventcallback)) return;

			GetLocalTime(&st);
			D2GEEventLog("DebugEventCallback","Event Called From Module \"%s\"",module);
			fprintf(hexstrm,"%d: Checking Module \"%s\" (%d)\tTime:%d.%d.%d.%d\n",\
					gdwTotalCount++,module,count,st.wHour,st.wMinute,\
					st.wSecond,st.wMilliseconds);
			sprintf (function,"%s(",module);
			va_start(ap,count);
			for (i=0; i<count; i++) {
				name=va_arg(ap, char *);
				value=va_arg(ap, int);
				strcat(function,name);
				if (i != count-1) strcat(function,", ");
				DebugDumpParam(name,value);
			}
			va_end(ap);
			strcat(function,")");
			fprintf(hexstrm,"Function: %s\n",function);
			fprintf(hexstrm,"\n\n");
			fflush(hexstrm);
			return;
		}


		static BOOL DebugDumpParam(LPCSTR lpName, DWORD dwValue)
		{
			void 		* p;

			p = (void *)dwValue;
			if (!hexstrm) return FALSE;
			fprintf (hexstrm,"%s: 0x%08X write:%s ",lpName,dwValue,\
				YESNO(!(IsBadWritePtr(p,sizeof(int)))));
			if (!IsBadReadPtr(p,sizeof(int))) {
				fprintf(hexstrm,"*%s=0x%08X\n",lpName,*(int *)p);
			} else fprintf(hexstrm,"\n");
			if (!IsBadReadPtr(p, DEBUG_DUMPSIZE)) {
				hexdump(p, DEBUG_DUMPSIZE);
			}
			return TRUE;
		}

		extern void DebugNetPacket(D2GSPACKET *lpPacket)
		{
			SYSTEMTIME	st;
			UCHAR		timestr[64];

			if (!(d2gsconf.debugnetpacket)) return;

			if (!lpPacket) return;
			if (!hexstrm) return;

			GetLocalTime(&st);
			sprintf(timestr, "    Time %d:%d:%d.%d",
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			switch(lpPacket->peer)
			{
			case PACKET_PEER_RECV_FROM_D2CS:
				fprintf(hexstrm, "From D2CS %d bytes:%s\n", lpPacket->datalen, timestr);
				break;
			case PACKET_PEER_SEND_TO_D2CS:
				fprintf(hexstrm, "To D2CS %d bytes:%s\n", lpPacket->datalen, timestr);
				break;
			case PACKET_PEER_RECV_FROM_D2DBS:
				fprintf(hexstrm, "From D2DBS %d bytes:%s\n", lpPacket->datalen, timestr);
				break;
			case PACKET_PEER_SEND_TO_D2DBS:
				fprintf(hexstrm, "To D2DBS %d bytes:%s\n", lpPacket->datalen, timestr);
				break;
			}
			if (lpPacket->datalen>0)
				hexdump(lpPacket->data, lpPacket->datalen);
			fprintf(hexstrm, "\n\n");
			fflush(hexstrm);
			return;
		}

	}

}

#endif
