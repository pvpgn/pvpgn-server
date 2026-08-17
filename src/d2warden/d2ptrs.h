#ifndef D2PTRS_H
#define D2PTRS_H

extern DWORD D2CLIENT_D2BeginOfPtr;
#define pD2PtrsListStart D2CLIENT_D2BeginOfPtr

extern DWORD D2CLIENT_D2EndOfPtr;
#define pD2PtrsListEnd D2CLIENT_D2EndOfPtr


#if defined(D2_1_13_c)
typedef void __stdcall D2NET_SendPacket_t(DWORD unk1, DWORD ClientID, unsigned char* ThePacket, DWORD PacketLen);
extern D2NET_SendPacket_t* D2NET_SendPacket;

typedef DWORD __fastcall D2GAME_GetClient_I_t(DWORD ClientID);
extern D2GAME_GetClient_I_t* D2GAME_GetClient_I;

typedef void __stdcall D2GAME_LeaveCriticalSection_I_t(DWORD ClientID);
extern D2GAME_LeaveCriticalSection_I_t* D2GAME_LeaveCriticalSection_I;

typedef DWORD __fastcall D2GAME_Send0XAEPacket_I_t(void* ptPlayer, DWORD Length, DWORD* Packet);
extern D2GAME_Send0XAEPacket_I_t* D2GAME_Send0XAEPacket_I;

typedef DWORD __stdcall D2GAME_KickCharFromGame_t(DWORD ClientID);
extern D2GAME_KickCharFromGame_t* D2GAME_KickCharFromGame;

typedef DWORD __stdcall D2NET_GetClient_t(DWORD ClientID);
extern D2NET_GetClient_t* D2NET_GetClient;

#elif defined(D2_1_13_d)
typedef void __stdcall D2NET_SendPacket_t(DWORD unk1, DWORD ClientID, unsigned char* ThePacket, DWORD PacketLen);
extern D2NET_SendPacket_t* D2NET_SendPacket;

typedef DWORD __fastcall D2GAME_GetClient_I_t(DWORD ClientID);
extern D2GAME_GetClient_I_t* D2GAME_GetClient_I;

typedef void __stdcall D2GAME_LeaveCriticalSection_I_t(DWORD ClientID);
extern D2GAME_LeaveCriticalSection_I_t* D2GAME_LeaveCriticalSection_I;

typedef DWORD __fastcall D2GAME_Send0XAEPacket_I_t(void* ptPlayer, DWORD Length, DWORD* Packet);
extern D2GAME_Send0XAEPacket_I_t* D2GAME_Send0XAEPacket_I;

typedef DWORD __fastcall D2GAME_SendPacket_I_t(DWORD* ptClient, unsigned char* Packet, DWORD Length);
extern D2GAME_SendPacket_I_t* D2GAME_SendPacket_I;

typedef DWORD __stdcall D2GAME_KickCharFromGame_t(DWORD ClientID);
extern D2GAME_KickCharFromGame_t* D2GAME_KickCharFromGame;

typedef DWORD __fastcall D2GAME_GameFindUnitFunc_I_t(DWORD ptGame, DWORD dwUnitType, DWORD dwUnitId);
extern D2GAME_GameFindUnitFunc_I_t* D2GAME_GameFindUnitFunc_I;

typedef DWORD __stdcall D2NET_GetClient_t(DWORD ClientID);
extern D2NET_GetClient_t* D2NET_GetClient;
#endif


BOOL RelocD2Ptrs(DWORD* pPtrsStart, DWORD* pPtrsEnd);

#endif