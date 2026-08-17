#include <Windows.h>

DWORD __fastcall D2GAME_GetClient_STUB(DWORD ClientID);
DWORD __fastcall D2GAME_LeaveCriticalSection_STUB(DWORD unk);
DWORD __fastcall D2GAME_Send0XAEPacket_STUB(void* ptPlayer, DWORD Length, DWORD* Packet);
#if defined(D2_1_11_b) || defined(D2_1_13_d)
DWORD __fastcall D2GAME_GameFindUnitFunc_STUB(DWORD ptGame, DWORD dwUnitId, DWORD dwUnitType);
DWORD __fastcall D2GAME_SendPacket_STUB(char* ptConnection, unsigned char* Packet, DWORD PacketLength);
#endif

#define D2GAME_GetClient			D2GAME_GetClient_STUB
#define D2GAME_LeaveCriticalSection	D2GAME_LeaveCriticalSection_STUB
#define D2GAME_Send0XAEPacket		D2GAME_Send0XAEPacket_STUB
#if defined(D2_1_11_b) || defined(D2_1_13_d)
#define D2GAME_SendPacket			D2GAME_SendPacket_STUB
#define D2GAME_GameFindUnitFunc		D2GAME_GameFindUnitFunc_STUB
#endif