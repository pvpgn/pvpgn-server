#ifndef INCLUDED_SERVER_H
#define INCLUDED_SERVER_H

namespace pvpgn
{

	namespace d2gs
	{

		/* structures */
		typedef int (*CLEANUP_ROUTINE)(void);
		typedef struct RAW_CLEANUP_RT_ITEM
		{
			char				comment[64];
			CLEANUP_ROUTINE		cleanup;
			struct RAW_CLEANUP_RT_ITEM* next;
		} CLEANUP_RT_ITEM, * PCLEANUP_RT_ITEM;


		int CleanupRoutineInsert(CLEANUP_ROUTINE pRoutine, char* comment);
		int DoCleanup(void);
		int CleanupRoutineForServerMutex(void);

	}

}

#endif