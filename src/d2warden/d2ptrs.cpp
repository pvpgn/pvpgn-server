#include <windows.h>
#include "d2ptrs.h"

enum DllNo { DLLNO_D2CLIENT, DLLNO_D2COMMON, DLLNO_D2GFX, DLLNO_D2WIN, DLLNO_D2LANG, DLLNO_D2CMP, DLLNO_D2MULTI, DLLNO_BNCLIENT, DLLNO_D2NET, DLLNO_STORM, DLLNO_FOG, DLLNO_D2GAME };

enum DllBase {
	DLLBASE_D2CLIENT = 0x6FAB0000,
	DLLBASE_D2COMMON = 0x6FD50000,
	DLLBASE_D2GFX = 0x6FA80000,
	DLLBASE_D2WIN = 0x6F8E0000,
	DLLBASE_D2LANG = 0x6FC00000,
	DLLBASE_D2CMP = 0x6FE10000,
	DLLBASE_D2MULTI = 0x6F9D0000,
	DLLBASE_BNCLIENT = 0x6FF20000,
	DLLBASE_D2NET = 0x6FFB0000, // conflict with STORM.DLL
	DLLBASE_STORM = 0x6FBF0000,
	DLLBASE_FOG = 0x6FF50000,
	DLLBASE_D2GAME = 0x6FC20000
};

#define DLLOFFSET(a1,b1) ((DLLNO_##a1)|(( ((b1)<0)?(b1):(b1)-DLLBASE_##a1 )<<8))

//NOTE :- reference vars buggy
#define D2FUNCPTR(d1,v1,t1,t2,o1) typedef t1 d1##_##v1##_t t2; d1##_##v1##_t *d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define D2VARPTR(d1,v1,t1,o1)     typedef t1 d1##_##v1##_t;    d1##_##v1##_t *p_##d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define D2ASMPTR(d1,v1,o1)        DWORD d1##_##v1 = DLLOFFSET(d1,o1);


DWORD D2CLIENT_D2BeginOfPtr = DLLOFFSET(D2CLIENT, 0xfeeeeeee); // D2ASMPTR(D2CLIENT, D2BeginOfPtr, 0xfeeeeeee);
DWORD D2CLIENT_D2EndOfPtr = DLLOFFSET(D2CLIENT, 0xeeeeeeee); // D2ASMPTR(D2CLIENT, D2EndOfPtr, 0xeeeeeeee)

#if defined(D2_1_11_b)
// D2NET ptrs
D2FUNCPTR(D2NET, SendPacket, void __stdcall, (DWORD unk1, DWORD ClientID, unsigned char* ThePacket, DWORD PacketLen), -10018)
// by marsgod D2Game
D2FUNCPTR(D2GAME, GetClient_I, DWORD __fastcall, (DWORD ClientID), 0x6FD04A30)
D2FUNCPTR(D2GAME, LeaveCriticalSection_I, void __stdcall, (DWORD ClientID), 0x6FD03AD0)
D2FUNCPTR(D2GAME, Send0XAEPacket_I, DWORD __fastcall, (void* ptPlayer, DWORD Length, DWORD* Packet), 0x6FCC0EA0)
D2FUNCPTR(D2GAME, SendPacket_I, DWORD __fastcall, (DWORD* ptClient, unsigned char* Packet, DWORD Length), 0x6FCC0D50)
D2FUNCPTR(D2GAME, KickCharFromGame, DWORD __stdcall, (DWORD ClientID), -10037)
D2FUNCPTR(D2GAME, GameFindUnitFunc_I, DWORD __fastcall, (DWORD ptGame, DWORD dwUnitType, DWORD dwUnitId), 0x6FCDEF80)

D2FUNCPTR(D2NET, GetClient, DWORD __stdcall, (DWORD ClientID), -10005)

#elif defined(D2_1_13_a)
// D2NET ptrs
D2FUNCPTR(D2NET, SendPacket, void __stdcall, (DWORD unk1, DWORD ClientID, unsigned char* ThePacket, DWORD PacketLen), -10030)
// by marsgod D2Game
D2FUNCPTR(D2GAME, GetClient_I, DWORD __fastcall, (DWORD ClientID), 0x6FC61ED0)
D2FUNCPTR(D2GAME, LeaveCriticalSection_I, void __stdcall, (DWORD ClientID), 0x6FC60F50)
D2FUNCPTR(D2GAME, Send0XAEPacket_I, DWORD __fastcall, (void* ptPlayer, DWORD Length, DWORD* Packet), 0x6FCCDD70)
D2FUNCPTR(D2GAME, KickCharFromGame, DWORD __stdcall, (DWORD ClientID), -10042)

D2FUNCPTR(D2NET, GetClient, DWORD __stdcall, (DWORD ClientID), -10035)

#elif defined(D2_1_13_c)
// D2NET ptrs
D2FUNCPTR(D2NET, SendPacket, void __stdcall, (DWORD unk1, DWORD ClientID, unsigned char* ThePacket, DWORD PacketLen), -10002)
// by marsgod D2Game
D2FUNCPTR(D2GAME, GetClient_I, DWORD __fastcall, (DWORD ClientID), 0x6FC4AB70)
D2FUNCPTR(D2GAME, LeaveCriticalSection_I, void __stdcall, (DWORD ClientID), 0x6FC49BA0)
D2FUNCPTR(D2GAME, Send0XAEPacket_I, DWORD __fastcall, (void* ptPlayer, DWORD Length, DWORD* Packet), 0x6FCAA4F0)
D2FUNCPTR(D2GAME, KickCharFromGame, DWORD __stdcall, (DWORD ClientID), -10003)

D2FUNCPTR(D2NET, GetClient, DWORD __stdcall, (DWORD ClientID), -10015)

#elif defined(D2_1_13_d)
// D2NET ptrs
D2NET_SendPacket_t* D2NET_SendPacket = (D2NET_SendPacket_t*)DLLOFFSET(D2NET, -10012);
// by marsgod D2Game
D2GAME_GetClient_I_t* D2GAME_GetClient_I = (D2GAME_GetClient_I_t*)DLLOFFSET(D2GAME, 0x6FCDC790);
D2GAME_LeaveCriticalSection_I_t* D2GAME_LeaveCriticalSection_I = (D2GAME_LeaveCriticalSection_I_t*)DLLOFFSET(D2GAME, 0x6FCDB8E0);
D2GAME_Send0XAEPacket_I_t* D2GAME_Send0XAEPacket_I = (D2GAME_Send0XAEPacket_I_t*)DLLOFFSET(D2GAME, 0x6FCFB8D0);
D2GAME_SendPacket_I_t* D2GAME_SendPacket_I = (D2GAME_SendPacket_I_t*)DLLOFFSET(D2GAME, 0x6FCFB780);
D2GAME_KickCharFromGame_t* D2GAME_KickCharFromGame = (D2GAME_KickCharFromGame_t*)DLLOFFSET(D2GAME, -10019);
D2GAME_GameFindUnitFunc_I_t* D2GAME_GameFindUnitFunc_I = (D2GAME_GameFindUnitFunc_I_t*)DLLOFFSET(D2GAME, 0x6FC8DC40);

D2NET_GetClient_t* D2NET_GetClient = (D2NET_GetClient_t*)DLLOFFSET(D2NET, -10019);
#endif


DWORD GetDllOffset(char *dll, int offset)
{
	HMODULE hmod = GetModuleHandle(dll);
	if (!hmod)
		hmod = LoadLibrary(dll);
	if (!hmod) return 0;
	if (offset < 0) {
		return (DWORD)GetProcAddress(hmod, (LPCSTR)-offset);
	}
	return ((DWORD)hmod)+offset;
}

DWORD GetDllOffset(int num)
{
	static char *dlls[] = {"D2CLIENT.DLL", "D2COMMON.DLL", "D2GFX.DLL", "D2WIN.DLL", "D2LANG.DLL", "D2CMP.DLL", "D2MULTI.DLL", "BNCLIENT.DLL", "D2NET.DLL", "STORM.DLL", "FOG.DLL", "D2GAME.DLL"};
	return GetDllOffset(dlls[num&0xff], num>>8);
}

BOOL RelocD2Ptrs(DWORD* pPtrsStart, DWORD* pPtrsEnd)
{
	DWORD *p = pPtrsStart, ret = TRUE;
	do {
		if (!(*p = GetDllOffset(*p))) ret = FALSE;
	} while (++p <= pPtrsEnd);
	return ret;
}
