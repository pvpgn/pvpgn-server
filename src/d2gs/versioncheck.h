#ifndef INCLUDED_VERSIONCHECK_H
#define INCLUDED_VERSIONCHECK_H


namespace pvpgn
{

	namespace d2gs
	{

		/*
			Return Value: FALSE if version check failed,
			otherwise, a non-zero checksum will be returned.
		*/
		extern DWORD VersionCheck(void);

	}

}

#endif
