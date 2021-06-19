#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


namespace pvpgn
{

	namespace d2gs
	{

#define	DEFAULT_VERSIONCHECK_KEY	0x12345678
#define	DEFAULT_PIECE_NUMBER		100
#define	DEFAULT_CHECK_SIZE 			5000000

		static char const* CheckFileList[] = {
			"Patch_d2.mpq", "D2Data.mpq", "D2Exp.mpq", "d2server.dll", "D2Win.dll",
			"D2Game.dll", "D2Client.dll", "D2Common.dll", "D2Net.dll", "Fog.dll",
			"Storm.dll", "D2Lang.dll", "D2Cmp.dll" , NULL
		};

		static DWORD CheckFile(DWORD dwKey, LPDWORD pdwChecksum, LPCSTR lpFileName);

		extern DWORD VersionCheck(void)
		{
			DWORD	dwChecksum, dwKey;
			CHAR	temp[MAX_PATH];
			DWORD	i;

			dwChecksum = 0;
			if (!GetModuleFileName(NULL, temp, sizeof(temp)))
			{
				return FALSE;
			}
			dwKey = DEFAULT_VERSIONCHECK_KEY;
			if (!CheckFile(dwKey, &dwChecksum, temp))
			{
				return FALSE;
			}
			for (i = 0; CheckFileList[i]; i++)
			{
				if (!CheckFile(dwKey, &dwChecksum, CheckFileList[i]))
				{
					return FALSE;
				}
			}
			if (!dwChecksum) dwChecksum--;
			return dwChecksum;
		}

		static DWORD CheckFile(DWORD dwKey, LPDWORD pdwChecksum, LPCSTR lpFileName)
		{
			DWORD		dwSize, dwCount;
			DWORD		i, j, temp, data;
			FILE* fp;

			if (!pdwChecksum || !lpFileName) return FALSE;
			if (!(fp = fopen(lpFileName, "rb")))
			{
				return FALSE;
			}
			fseek(fp, 0, SEEK_END);
			dwSize = ftell(fp);
			if (dwSize > DEFAULT_CHECK_SIZE) temp = DEFAULT_CHECK_SIZE;
			else temp = dwSize;
			dwCount = (temp / DEFAULT_PIECE_NUMBER) / sizeof(data);
			for (j = 0; j < DEFAULT_PIECE_NUMBER; j++)
			{
				fseek(fp, (dwSize / DEFAULT_PIECE_NUMBER) * j, SEEK_SET);
				for (i = 0; i < dwCount; i++)
				{
					fread(&data, 1, sizeof(data), fp);
					*pdwChecksum += data;
					*pdwChecksum ^= dwKey;
				}
			}
			fclose(fp);
			*pdwChecksum += dwSize;
			return TRUE;
		}

	}

}