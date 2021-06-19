#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "charlist.h"


namespace pvpgn
{

	namespace d2gs
	{

		/* variables */
		static unsigned int		clitbl_len = 0;
		static D2CHARLIST** clitbl = NULL;


		unsigned int string_hash(char const* string)
		{
			unsigned int	i;
			unsigned int	pos;
			unsigned int	hash;
			unsigned int	ch;

			if (!string) return 0;

			for (hash = 0, pos = 0, i = 0; i < strlen(string); i++)
			{
				if (isascii((int)string[i]))
					ch = (unsigned int)(unsigned char)tolower((int)string[i]);
				else
					ch = (unsigned int)(unsigned char)string[i];
				hash ^= ROTL(ch, pos, sizeof(unsigned int) * CHAR_BIT);
				pos += CHAR_BIT - 1;
			}

			return hash;
		}


		int charlist_init(unsigned int tbllen)
		{
			if (!tbllen) return -1;
			charlist_destroy();
			clitbl = (D2CHARLIST**)malloc(tbllen * sizeof(D2CHARLIST**));
			if (!clitbl) return -1;
			memset(clitbl, 0, tbllen * sizeof(D2CHARLIST**));
			clitbl_len = tbllen;
			return 0;
		}


		int charlist_destroy(void)
		{
			charlist_flush();
			if (clitbl)	free(clitbl);
			clitbl = NULL;
			clitbl_len = 0;
			return 0;
		}


		void charlist_flush(void)
		{
			unsigned int	i;
			D2CHARLIST* pcl, * ptmp;

			if (clitbl)
			{
				for (i = 0; i < clitbl_len; i++)
				{
					pcl = clitbl[i];
					while (pcl)
					{
						ptmp = pcl;
						pcl = pcl->next;
						free(ptmp);
					}
				}
			}
			return;
		}


		void* charlist_getdata(const char* charname, int type)
		{
			D2CHARLIST* pcl;
			unsigned int	hashval;
			void* ret;

			if (!charname) return NULL;
			if (!clitbl_len) return NULL;
			if (strlen(charname) >= (MAX_CHARNAME_LEN)) return NULL;

			ret = NULL;
			hashval = string_hash(charname) % clitbl_len;
			pcl = clitbl[hashval];
			while (pcl)
			{
				if (strcmpi(pcl->charname, charname) == 0)
				{
					switch (type)
					{
					case CHARLIST_GET_CHARINFO:
						ret = pcl->pCharInfo;
						break;
					case CHARLIST_GET_GAMEINFO:
						ret = pcl->pGameInfo;
						break;
					default:
						ret = NULL;
					}
					break;
				}
				pcl = pcl->next;
			}
			return ret;
		}


		int charlist_insert(const char* charname, void* pCharInfo, void* pGameInfo)
		{
			D2CHARLIST* pcl, * ptmp;
			unsigned int	hashval;

			if (!charname) return -1;
			if (!clitbl_len) return -1;
			if (strlen(charname) >= (MAX_CHARNAME_LEN)) return -2;

			hashval = string_hash(charname) % clitbl_len;
			pcl = clitbl[hashval];
			ptmp = NULL;
			while (pcl)
			{
				if (strcmpi(pcl->charname, charname) == 0)
					return -3;   /* found the char, error */
				ptmp = pcl;
				pcl = pcl->next;
			}

			/* not found, insert one */
			pcl = (D2CHARLIST*)malloc(sizeof(D2CHARLIST));
			if (!pcl) return -4;    /* no free memory available :( */
			memset(pcl, 0, sizeof(D2CHARLIST));
			strncpy(pcl->charname, charname, MAX_CHARNAME_LEN - 1);
			pcl->pCharInfo = pCharInfo;
			pcl->pGameInfo = pGameInfo;

			/* add to hash table link list */
			if (ptmp) ptmp->next = pcl;
			else clitbl[hashval] = pcl;

			return 0;
		}


		int charlist_delete(const char* charname)
		{
			D2CHARLIST* pcl, * ptmp;
			unsigned int	hashval;

			if (!charname) return -1;
			if (!clitbl_len) return -1;
			if (strlen(charname) >= (MAX_CHARNAME_LEN)) return -1;

			hashval = string_hash(charname) % clitbl_len;
			pcl = clitbl[hashval];
			ptmp = NULL;
			while (pcl)
			{
				if (strcmpi(pcl->charname, charname) == 0)
				{
					if (ptmp) ptmp->next = pcl->next;
					else clitbl[hashval] = pcl->next;
					free(pcl);
					return 0;
				}
				ptmp = pcl;
				pcl = pcl->next;
			}
			return 0;
		}

	}

}