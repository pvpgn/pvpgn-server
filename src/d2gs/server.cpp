#include "server.h"

#include <Windows.h>

#include "eventlog.h"
#include "vars.h"


namespace pvpgn
{

	namespace d2gs
	{

		static CLEANUP_RT_ITEM* pCleanupRT = NULL;

		/*********************************************************************
		 * Purpose: to add an cleanup routine item to the list
		 * Return: TRUE(success) or FALSE(failed)
		 *********************************************************************/
		int CleanupRoutineInsert(CLEANUP_ROUTINE pRoutine, char* comment)
		{
			CLEANUP_RT_ITEM* pitem;

			if (pRoutine == NULL) return FALSE;
			pitem = (CLEANUP_RT_ITEM*)malloc(sizeof(CLEANUP_RT_ITEM));
			if (!pitem)
			{
				D2GSEventLog("CleanupRoutineInsert", "Can't alloc memory");
				return FALSE;
			}
			ZeroMemory(pitem, sizeof(CLEANUP_RT_ITEM));

			/* fill the structure */
			if (comment)
				strncpy(pitem->comment, comment, sizeof(pitem->comment) - 1);
			else
				strncpy(pitem->comment, "unknown", sizeof(pitem->comment) - 1);
			pitem->cleanup = pRoutine;
			pitem->next = pCleanupRT;
			pCleanupRT = pitem;

			return TRUE;

		} /* End of CleanupRoutineInsert() */


		/*********************************************************************
		 * Purpose: call the cleanup routine to do real cleanup work
		 * Return: TRUE or FALSE
		 *********************************************************************/
		int DoCleanup(void)
		{
			CLEANUP_RT_ITEM* pitem, * pprev;

			pitem = pCleanupRT;
			while (pitem)
			{
				D2GSEventLog("DoCleanup", "Calling cleanup routine '%s'", pitem->comment);
				pitem->cleanup();
				pprev = pitem;
				pitem = pitem->next;
				free(pprev);
			}
			pCleanupRT = NULL;

			/* at last, cleanup event log system */
			D2GSEventLog("DoCleanup", "Cleanup done.");
			D2GSEventLogCleanup();

			/* Close the mutex */
			if (hD2GSMutex)	CloseHandle(hD2GSMutex);
			if (hStopEvent) CloseHandle(hStopEvent);

#ifdef DEBUG_ON_CONSOLE
			printf("Press Any Key to Continue");
			_getch();
#endif

			return TRUE;

		} /* End of DoCleanup() */


		/*********************************************************************
		 * Purpose: cleanup routine for release the global server mutex
		 * Return: TRUE or FALSE
		 *********************************************************************/
		int CleanupRoutineForServerMutex(void)
		{
			if (!hD2GSMutex) return FALSE;
			return CloseHandle(hD2GSMutex);

		} /* End of CleanupRoutineServerMutex() */

	}

}