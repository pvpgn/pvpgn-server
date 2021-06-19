#include <windows.h>
#include "d2ptrs.h"
#include "d2callstub.h"

DWORD __declspec(naked) __fastcall D2GAME_GetClient_STUB(DWORD ClientID)
{
	__asm {
		push esi
		mov esi, ecx
		call D2GAME_GetClient_I
		pop esi
		ret
	}
}


DWORD __declspec(naked) __fastcall D2GAME_LeaveCriticalSection_STUB(DWORD unk)
{
	__asm {
		push eax
		mov eax, ecx
		call D2GAME_LeaveCriticalSection_I
		pop eax
		ret
	}
}

DWORD __declspec(naked) __fastcall D2GAME_Send0XAEPacket_STUB(void *ptPlayer,DWORD Length,DWORD *Packet)
{
	__asm {
		mov esi, ecx
		mov eax, edx
		push Packet
		call D2GAME_Send0XAEPacket_I
		ret
	}
}

#if defined(D2_1_11_b) || defined(D2_1_13_d)

DWORD __declspec(naked) __fastcall D2GAME_GameFindUnitFunc_STUB(DWORD ptGame, DWORD dwUnitId, DWORD dwUnitType)
{
	__asm {
		mov eax,[esp+4]
		call D2GAME_GameFindUnitFunc_I
		retn 4
	}
}


DWORD __declspec(naked) __fastcall D2GAME_SendPacket_STUB(char *ptConnection,unsigned char *Packet,DWORD PacketLength)
{
	__asm {
		mov eax,[esp+4]
		push eax
		push edx
		mov eax, ecx
		call D2GAME_SendPacket_I
		retn 4
	}
}

#endif