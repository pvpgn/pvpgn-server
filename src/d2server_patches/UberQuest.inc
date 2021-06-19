;Uber Quest
;Most code from PlugY, thanks

; The idea
; Use the game quest structure store the uber quest state
; When the game has been created, the uber quest state is zero
; After player open portals, the uber quest state updated
; Every time one player try to open portal, first check the uber quest state
; quest state: 
;			bit 0--0x85 level portal open
;			bit 1--0x86 level portal open
;			bit 2--0x87 level portal open
;			bit 3--0x88 level portal open(Final portal)
;			bit 4--0x85 level BOSS spawn
;			bit 5--0x86 level BOSS spawn
;			bit 6--0x87 level BOSS spawn
;			bit 7--0x88 level BOSS0 spawn
;			bit 8--0x88 level BOSS1 spawn
;			bit 9--0x88 level BOSS2 spawn

;D2 export entrys
D2Common_10141_GetMyPosition dd 0
D2Common_10826_GetLevelIdFromRoom dd 0
D2Common_10331_GethRoomFUnit dd 0
D2Common_10654_GetFreeTile dd 0
D2Common_10258_GetPortalFlag dd 0
D2Common_10111_UpdatePortalFlag dd 0
D2Common_10346_UpdateRoomHavePortalFlag dd 0
D2Common_10174_GetQuestFlag dd 0
D2Common_10003_SetQuestFlag dd 0
D2Common_10014_GetLevelTxt dd 0

D2GAME_0X200E0 dd 0
D2GAME_0X24950_SpawnSuperUnique dd 0
D2GAME_0X1280_GetFreeTitle dd 0
D2GAME_0XE1D90_InitTheUnit dd 0
D2GAME_0XA22E0_CreatePortalInTown dd 0
D2GAME_RemoveMonsterCorpse dd 0

D2GAME_StallSomeTime dd 0
D2GAME_GetAreaSpawnUnitNum dd 0
D2GAME_UseASkill dd 0
D2GAME_SpawnAMonster dd 0
D2GAME_sgptDataTables dd 0
D2GAME_GetMonstatsTxtRec dd 0

D2GAME_0XA240B dd 0

D2GAME_Baal_AI dd 0
D2GAME_Mephisto_AI dd 0
D2GAME_Diablo_AI dd 0

; Every Game should call InitUberQuestState once and only once!
; ecx=ptExtendGameInfoStruct
; edx=rand_h
InitUberQuestState proc
								push esi
								push edi
                xor     eax, eax
                
                mov edi,ecx		; ptExtendGameInfoStruct
                
                mov     [edi+ExtendGameInfoStruct.PortalOpenedFlag],eax
                
                mov [edi+ExtendGameInfoStruct.rand_h],edx
                mov [edi+ExtendGameInfoStruct.rand_l],29Ah
                
                lea eax,[edi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[edi+ExtendGameInfoStruct.rand_l]
                push eax
                push 0Ah	; divv
                call RandomStep
                add eax,0Ah ; 0A~13
                mov [edi+ExtendGameInfoStruct.BOSS_A_AREA],eax ; uberandariel
                
                lea eax,[edi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[edi+ExtendGameInfoStruct.rand_l]
                push eax
                push 28h	; divv
                call RandomStep
                add eax,14h ; 14~3B
                mov [edi+ExtendGameInfoStruct.BOSS_B_AREA],eax ; uberduriel
                
                lea eax,[edi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[edi+ExtendGameInfoStruct.rand_l]
                push eax
                push 14h	; divv
                call RandomStep
                add eax,0Ah ; 0A~1D
                mov [edi+ExtendGameInfoStruct.BOSS_C_AREA],eax ; uberizual
                
                lea eax,[edi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[edi+ExtendGameInfoStruct.rand_l]
                push eax
                push 0Ah	; divv
                call RandomStep
                add eax,5 ; 5~0E
                mov esi,eax
                
                lea eax,[edi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[edi+ExtendGameInfoStruct.rand_l]
                push eax
                push 04h	; divv
                call RandomStep
                add eax,esi ; 5~12
                mov [edi+ExtendGameInfoStruct.BOSS_D0_AREA],eax ; ubermephisto
               
                
;                lea eax,[edi+ExtendGameInfoStruct.rand_h]
;                push eax
;                lea eax,[edi+ExtendGameInfoStruct.rand_l]
;                push eax
;                push 04h	; divv
;                call RandomStep
;                add eax,esi ; 5~12
                mov [edi+ExtendGameInfoStruct.BOSS_D1_AREA],eax ; uberdiablo
               
;                lea eax,[edi+ExtendGameInfoStruct.rand_h]
;                push eax
;                lea eax,[edi+ExtendGameInfoStruct.rand_l]
;                push eax
;                push 04h	; divv
;                call RandomStep
;                add eax,esi ; 5~12
                mov [edi+ExtendGameInfoStruct.BOSS_D2_AREA],eax ; uberbaal
								
								pop edi
								pop esi
                retn
InitUberQuestState endp

                align 10h

openPandPortal proc
var_24          = dword ptr -24h
var_20          = dword ptr -20h
var_1C          = dword ptr -1Ch
var_18          = dword ptr -18h
var_14          = dword ptr -14h

                sub     esp, 1Ch
                push    ebx
                push    edi
                push		esi
                mov     edi, edx	; ptPlayer
                mov     ebx, ecx	; ptGame
                
                
                call		PortalConditionCheck
                test		eax,eax
                jnz			loc_10018B68
                
                
over:
								pop			esi
                pop     edi
                xor     eax, eax
                pop     ebx
                add     esp, 1Ch
                retn

loc_10018B68:
                push		ebx	; ptGame
								call		GetExtendGameInfoStructPtr
								mov			esi,eax		; save ExtendGameInfoStructPtr
                mov     ecx, [esi+ExtendGameInfoStruct.PortalOpenedFlag]
                cmp			ecx,0
                jnz			UberQuestInited
                mov			ecx,esi	; ExtendGameInfoStructPtr
                mov			edx,[edi+20h]	; dwseed, a random number from player unit
                call		InitUberQuestState
UberQuestInited:
								mov			eax,[esi+ExtendGameInfoStruct.PortalOpenedFlag]
								and			eax,7                
                cmp			al, 7
                jz			over	; all 3 portal already opened, we can't open another portal
                
rand_again:
                lea eax,[esi+ExtendGameInfoStruct.rand_h]
                push eax
                lea eax,[esi+ExtendGameInfoStruct.rand_l]
                push eax
								push		3	; divv
                call		RandomStep
                bt			[esi+ExtendGameInfoStruct.PortalOpenedFlag],eax	; if the eax==0 and cl==1 then again, try to select next portal
                jc			rand_again
                bts			[esi+ExtendGameInfoStruct.PortalOpenedFlag],eax
                add			eax,85h
                
                push    eax ; target level
                push    edi ; ptUnit
                mov			edi,eax	; save the level will be generated 
                push    ebx ; ptGame
                call    OpenningPortalToLevel
                add     esp, 0Ch
                test    eax, eax
                jz      short loc_10018C45 ; the portal open failed..... strange....
                cmp     edi, 85h        ; First Portal Open?
                jnz     short loc_10018C1E
                mov     eax, [esi+ExtendGameInfoStruct.PortalOpenedFlag]
                or      eax, 1
                jmp     short loc_10018C40

loc_10018C1E:
                cmp     edi, 86h        ; Second Portal Open?
                jnz     short loc_10018C30
                mov     eax, [esi+ExtendGameInfoStruct.PortalOpenedFlag]
                or      eax, 2
                jmp     short loc_10018C40

loc_10018C30:
                cmp     edi, 87h        ; Third Portal Open?
                jnz     short loc_10018C45
                mov     eax, [esi+ExtendGameInfoStruct.PortalOpenedFlag]
                or      eax, 4

loc_10018C40:
                mov     [esi+ExtendGameInfoStruct.PortalOpenedFlag], eax
                mov			eax,edi

loc_10018C45:
                pop			esi
                pop     edi
                pop     ebx
                add     esp, 1Ch
                retn
openPandPortal endp

openPandFinalPortal proc

var_10          = dword ptr -10h
var_C           = dword ptr -0Ch
var_8           = dword ptr -8

                sub     esp, 8
                push    esi
                push    edi
                push		ebx

                mov     esi, edx	; ptUnit
                mov     edi, ecx	; ptGame
                

                call		PortalConditionCheck
                test		eax,eax
                jz			over

                ; check the final portal opened or not?
                push		edi ; ptGame
								call		GetExtendGameInfoStructPtr
								mov			ecx,[eax+ExtendGameInfoStruct.PortalOpenedFlag]
								mov			ebx,eax	; save eax
								xor			eax,eax
								bt			ecx,3
								jc			over	; the final portal already opened.
                cmp			ecx,0
                jnz			UberQuestInited
                mov			ecx,ebx	; ExtendGameInfoStructPtr
                mov			edx,[esi+20h]	; dwseed, a random number from player unit
                call		InitUberQuestState
UberQuestInited:                
								
                lea     eax, [esp+10h+var_8]
                push    eax
                push    esi ; ptUnit
                call    D2Common_10141_GetMyPosition
                mov     ecx, [esp+0Ch]
                mov     edx, [esp+10h+var_8]


                push    88h
                push    esi ; ptUnit
                push    edi ; ptGame
                call    OpenningPortalToLevel
                add     esp, 0Ch ; CHECK!!!

								mov			esi,eax
								cmp     eax,0
								jz			over
                push		edi ; ptGame
								call		GetExtendGameInfoStructPtr
								mov			ecx,[eax+ExtendGameInfoStruct.PortalOpenedFlag]
								bts			ecx,3
								mov			[eax+ExtendGameInfoStruct.PortalOpenedFlag],ecx
                mov     eax, esi
over:
								pop			ebx
                pop     edi
                pop     esi
                add     esp, 8
                retn
openPandFinalPortal endp


SpawnUberBoss proc

arg_4           = dword ptr  8

								sub			esp,4
                push    esi
                push    edi
                mov     edi, [esp+10h+4]
                push    edi
                call    D2Common_10826_GetLevelIdFromRoom	; Get Level ID , 1 arg
                mov     esi, [esp+0Ch+4]	; ptGame
                add     eax, 0FFFFFF7Bh ; eax=eax-85h
                cmp     eax, 3
                ja      loc_10018EDC		; Not Uber Quest Levels, quit
                push    ebx
                push		eax
								push		esi	; ptGame
								call		GetExtendGameInfoStructPtr
								mov			ebx,eax	; save ExtendGameInfoStructPtr
								pop			eax
                cmp eax,0
                jz loc_10018D2C ; level 85h
                cmp eax,1
                jz loc_10018D67	; level 86h
                cmp eax,2
                jz loc_10018DA2	; level 87h
                jmp loc_10018DDD; level 88h

loc_10018D2C:
								mov			eax,10h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 10h	; BOSS_A Spawned?
                jnz     loc_10018EDD
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_A_AREA]
                test    eax, eax
                jz      short loc_10018D5B ; we spawn the boss at last step
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_A_AREA], eax
                jmp			loc_10018EDD

loc_10018D5B:
                mov     ecx,2C3h ; 707=uberandariel
                mov     edx,0
                jmp     loc_10018EB1

loc_10018D67:
								mov			eax,20h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 20h ; BOSS_B Spawned?
                jnz     loc_10018EDD
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_B_AREA]
                test    eax, eax
                jz      short loc_10018D96 ; we spawn the boss at last step
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_B_AREA], eax
                jmp			loc_10018EDD

loc_10018D96:
                mov     ecx,2C4h ; 708=uberduriel
                mov     edx,0
                jmp     loc_10018EB1

loc_10018DA2:
								mov			eax,40h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 40h ; BOSS_C Spawned?
                jnz     loc_10018EDD
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_C_AREA]
                test    eax, eax
                jz      short loc_10018DD1 ; we spawn the boss at last step
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_C_AREA], eax
                jmp			loc_10018EDD

loc_10018DD1:
                mov     ecx,2C2h ; 706=uberizual
                mov     edx,0
                jmp     loc_10018EB1

loc_10018DDD:
								mov			eax,80h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 80h ; BOSS_D0 Spawned?
                jnz     short loc_10018E2E
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_D0_AREA]
                test    eax, eax
                jz      short loc_10018DFC
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_D0_AREA], eax
                jmp     short loc_10018E2E

loc_10018DFC:
                push    0FFFFFFFFh
                push    0
                push    0
                push    edi
                push    esi
                mov     eax,1
                mov     ecx,2C0h ; 704=ubermephisto
                mov     edx,0
                call    D2GAME_0X24950_SpawnSuperUnique
                
                test    eax, eax
                jz      short loc_10018E2E
                mov			eax,[esp+12]	; restore
                or      [ebx+ExtendGameInfoStruct.PortalOpenedFlag], eax

loc_10018E2E:
								mov			eax,100h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 100h ; BOSS_D1 Spawned?
                jnz     short loc_10018E7F
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_D1_AREA]
                test    eax, eax
                jz      short loc_10018E4D
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_D1_AREA], eax
                jmp     short loc_10018E7F

loc_10018E4D:
                push    0FFFFFFFFh
                push    0
                push    0
                push    edi
                push    esi
                mov     eax,1
                mov     ecx,2C1h ; 705=uberdiablo
                mov     edx,0
                call    D2GAME_0X24950_SpawnSuperUnique
                test    eax, eax
                jz      short loc_10018E7F
                mov			eax,[esp+12]	; restore
                or      [ebx+ExtendGameInfoStruct.PortalOpenedFlag], eax

loc_10018E7F:
								mov			eax,200h
								mov			[esp+12],eax	; save
                mov     eax, [ebx+ExtendGameInfoStruct.PortalOpenedFlag]
                test    eax, 200h ; BOSS_D2 Spawned?
                jnz     short loc_10018EDD
                mov     eax, [ebx+ExtendGameInfoStruct.BOSS_D2_AREA]
                test    eax, eax
                jz      short loc_10018EAA
                dec     eax
                mov     [ebx+ExtendGameInfoStruct.BOSS_D2_AREA], eax
                jmp			loc_10018EDD

loc_10018EAA:
                mov     ecx,2C5h ; 709=uberbaal
                mov     edx,0

loc_10018EB1:
                push    0FFFFFFFFh
                push    0
                push    0
                push    edi
                push    esi
                mov     eax,1
                mov     edx,0;eax
                call    D2GAME_0X24950_SpawnSuperUnique
                test    eax, eax
                jz      short loc_10018EDD
                mov			eax,[esp+12]	; restore
                or      [ebx+ExtendGameInfoStruct.PortalOpenedFlag], eax
loc_10018EDD:
                pop     ebx
loc_10018EDC:
                push    edi
                push    esi
                call    D2GAME_0X200E0
                pop     edi
                pop     esi
                add			esp,4
                retn    8
SpawnUberBoss endp

PortalConditionCheck proc
; ecx=ptGame
; edx=ptUnit
; Check Game Difficulty
								push		esi
								push		edi
								
								mov			esi,ecx	; ptGame
								mov			edi,edx	; ptUnit
                mov     cl, [esi+6Dh] ; Game Difficulty, hell=2
                cmp     cl, 2
                jnz     short over

; Check in ACT5 town
                push    edi
                call    D2Common_10331_GethRoomFUnit
                push    eax
                call    D2Common_10826_GetLevelIdFromRoom
                cmp     eax, 6Dh
                jnz     short over

; Check player finished a5q3?
								push		edi
								call		D2Common_10920_GetpPlayerDataFromUnit
								movzx   ecx, byte ptr [esi+6Dh] ; 难度，hell=2   
								mov     eax, [eax+ecx*4+10h]	; Player's Quest Structure
								push    0
								push    40	; Eve of Destruction
								push    eax
								call    D2Common_10174_GetQuestFlag
								test		eax,eax
								jz			over             
								mov			eax,1
								pop			edi
								pop			esi
								ret
over:
								xor			eax,eax
								pop			edi
								pop			esi
								ret
PortalConditionCheck endp

OpenningPortalToLevel proc
var_38          = dword ptr -38h
var_34          = dword ptr -34h
var_30          = dword ptr -30h
var_2C          = dword ptr -2Ch
var_18          = dword ptr -18h
var_10          = dword ptr -10h
arg_4           = dword ptr  8
arg_8           = dword ptr  0Ch

                sub     esp, 8
                push    ebx
                mov     ebx, [esp+18h]
                push    esi
                push    edi
                mov     edi, [esp+1Ch]
                push    edi
                call    D2Common_10331_GethRoomFUnit
                mov     esi, eax
                push    esi
                call    D2Common_10826_GetLevelIdFromRoom
                cmp     eax, 6Dh
                jnz     short loc_10018A97
                lea     eax, [esp+0Ch]
                push    eax
                push    edi
                call    D2Common_10141_GetMyPosition
                push    0
                push    0C01h
                push    3E01h
                push    3
                lea     ecx, [esp+1Ch]
                push    ecx
                lea     edx, [esp+20h]
                push    edx
                push    esi
                call    D2Common_10654_GetFreeTile
                test    eax, eax
                jz      short loc_10018A97
                mov     eax, [esp+10h]
                mov     edx, [esp+0Ch]
                push    eax
                mov     ecx, esi
                call    D2GAME_0X1280_GetFreeTitle_1

                mov     edi, eax
                test    edi, edi
                jnz     short loc_10018AA0

loc_10018A97:
                pop     edi
                pop     esi
                xor     eax, eax
                pop     ebx
                add     esp, 8
                retn

loc_10018AA0:
                mov     ecx, [esp+10h]
                mov     edx, [esp+0Ch]
                push    ebp
                mov     ebp, [esp+1Ch]
                push    0
                push    1
                push    1
                push    edi
                push    ebp
                push    ecx
                push    edx
                mov     edx, 3Ch
                mov     ecx, 2
                call    D2GAME_0XE1D90_InitTheUnit
                mov     esi, eax
                mov     eax, [esi+14h]
                push    1
                push    esi
                mov     [eax+4], bl
                call    D2Common_11090_ChangeCurrentMode
                push    6Dh
                push    ebx
                push    esi
                push    ebp
                call    D2GAME_0XA22E0_CreatePortalInTown
                push    esi
                call    D2Common_10258_GetPortalFlag
                or      eax, 3
                push    eax
                push    esi
                call    D2Common_10111_UpdatePortalFlag
                push    0
                push    edi
                call    D2Common_10346_UpdateRoomHavePortalFlag
                push    0
                push    esi
                call    D2Common_10331_GethRoomFUnit
                push    eax
                call    D2Common_10346_UpdateRoomHavePortalFlag
                pop     ebp
                pop     edi
                pop     esi
                mov     eax, 1
                pop     ebx
                add     esp, 8
                retn
OpenningPortalToLevel endp


;ptAIParam { 
;   D2AIControl*      hControl;            //+00 
;   DWORD         unk;                  //+04 
;   Unit*         ptTarget;            //+08 
;   DWORD         unk;                  //+0C 
;   DWORD         unk;                  //+10 
;   int            DistanceToTarget;         //+14 
;   BOOL            EngagedInCombat;         //+18 
;   MonStatsTxt*      pMonStatsRec;            //+1C 
;   MonStats2Txt*      pMonStats2Rec;         //+20 
;}; 
;
;D2AIControl
;{
;	DWORD unk1;	// +00 SpecialState - stuff like terror, confusion goes here
;	DWORD ptAI_Function_Offset; // +04 fpAiFunction(); - the primary ai function to call (void * __fastcall)(pGame,pUnit,pAiTickArgs); 
;	DWORD unk2;	// +08 AiFlags
;	DWORD unk3;// +0C OwnerGUID - the global unique identifier of the boss or minion owner 
;	DWORD unk4; // +10 eOwnerType - the unit type of the boss or minion owner
;	DWORD unk5; // +14 dwArgs[3] - three dwords holding custom data used by ai func to store counters (etc)
;	
;	DWORD unk6;// +20 pCmdCurrents
;	DWORD unk7;// +24 pCmdLast
;	DWORD ptGame;// +28 pGame
;	DWORD unk8;// +2C OwnerGUID - the same as +008
;	DWORD unk8;// +30 eOwnerType - the same as +00C 
;	DWORD unk8;// +34 pMinionList - list of all minions, for boss units (SetBoss in MonStats, Unique, SuperUnique etc)
;	DWORD unk8;// +3C eTrapNo - used by shadows for summoning traps (so they stick to one type usually)
;}
;struct AttackParam { 
;   DWORD   eMode;            // +00 
;   Unit*   pAttacker;         // +04 
;   Unit*   pDefender;         // +08 
;   DWORD   unk1;            // +0C 
;   DWORD   unk2;            // +10 
;   BYTE      unk3;            // +14 
;   BYTE      modeFlag;         // +15 100 when mode exists, 101 otherwise 
;   BYTE      unk5;            // +16 
;   BYTE      unk6;            // +17 
;   DWORD   unk7;            // +18 
;   DWORD   unk8;            // +1C 
;};

; ptUnit+0x64=下一次Spawn时间
; ptUnit+0x68=Spawn Counter
;
;ecx=ptGame
;edx=pMonster
;arg_0=pPar
UberDiablo_AI0 proc
	arg_0           = dword ptr  4
	push		ecx
;	mov     eax, [esp+arg_0+4] ; pPar
;	mov     eax, [eax]      ; pPar->hControl
	mov     ecx, [ecx+0A8h] ; 游戏当前时间
	mov     [edx+64h], ecx  ; 下一次Spawn时间
;	mov     dword ptr [edx+68h], 0 ; Spawn Counter
	pop			ecx	; ptGame
	
	; 初始化随机数种子，用于随机Spawn怪物
	push		esi
	push		edx
	push		ecx
	call		GetExtendGameInfoStructPtr
	pop			edx
	mov			esi,eax
	mov			eax,[edx+20h]
	mov			[esi+ExtendGameInfoStruct.Diablo_rand_h],eax
	mov			eax,[edx+24h]
	mov			[esi+ExtendGameInfoStruct.Diablo_rand_l],eax
	pop			esi
	retn    4
UberDiablo_AI0 endp

; +14 = 总计数？
; +18 = 当前次数？
; +1C = next State
UberMephisto_AI0 proc
	arg_0           = dword ptr  4
	push		ecx
;	mov     eax, [esp+arg_0+4] ; pPar
;	mov     eax, [eax]      ; pPar->hControl
	mov     ecx, [ecx+0A8h] ; 游戏当前时间
	mov     [edx+64h], ecx  ; 下一次Spawn时间
;	mov     dword ptr [edx+68h], 0 ; Spawn Counter
	pop			ecx	; ptGame
	
	; 初始化随机数种子，用于随机Spawn怪物
	push		esi
	push		edx
	push		ecx
	call		GetExtendGameInfoStructPtr
	pop			edx
	mov			esi,eax
	mov			eax,[edx+20h]
	mov			[esi+ExtendGameInfoStruct.Meph_rand_h],eax
	mov			eax,[edx+24h]
	mov			[esi+ExtendGameInfoStruct.Meph_rand_l],eax
	pop			esi
	retn    4
UberMephisto_AI0 endp

;ecx=ptGame
;edx=pMonster
;arg_0=pPar
UberMephisto_AI proc
; 首先创建一个随机数，然后决定是Spawn一个随机怪物，还是继续原来的Mephisto_AI
	push	ecx	; save ecx
	push	edx	; save edx
	
	push		ecx
	call		GetExtendGameInfoStructPtr
	lea		ecx,[eax+ExtendGameInfoStruct.Meph_rand_h]
	push	ecx
	lea		ecx,[eax+ExtendGameInfoStruct.Meph_rand_l]
	push	ecx
	push		100
	call RandomStep
	
	pop		edx	; restore edx
	pop		ecx ; restore ecx
	
	cmp eax, SpawnProbability_M
	jg	call_orig_Mephisto_AI
	
	push ecx
	push edx
	mov eax,[esp+4]
	push eax
	call UberSpawnerMeph_AI
	pop edx
	pop ecx
	test eax,eax
	jz call_orig_Mephisto_AI
	ret 4
call_orig_Mephisto_AI:
	mov eax,[esp+4]
	push eax
	call D2GAME_Mephisto_AI
	ret 4
UberMephisto_AI endp

UberDiablo_AI proc
; 首先创建一个随机数，然后决定是Spawn一个随机怪物，还是继续原来的Diablo_AI
	push	ecx	; save ecx
	push	edx ; save edx
	
	push		ecx
	call		GetExtendGameInfoStructPtr
	lea		ecx,[eax+ExtendGameInfoStruct.Diablo_rand_h]
	push	ecx
	lea		ecx,[eax+ExtendGameInfoStruct.Diablo_rand_l]
	push	ecx
	push		100
	call RandomStep
	
	pop		edx	; restore edx
	pop		ecx ; restore ecx

	cmp eax, SpawnProbability_D
	jg	call_orig_Diablo_AI
	
	push ecx
	push edx
	mov eax,[esp+4]
	push eax
	call UberSpawnerDiablo_AI
	pop edx
	pop ecx
	test eax,eax
	jz call_orig_Diablo_AI
	ret 4
call_orig_Diablo_AI:
	mov eax,[esp+4]
	push eax
	call D2GAME_Diablo_AI
	ret 4
UberDiablo_AI endp

SpawnProbability_D dd 30
MaxSpawnNum_D dd 40
SpawnInterv_D	dd 5
ActivArea_D		dd 30
StallTime_D		dd 2
TypeOfSpawns_D	dd 3 ; 最多10种
SpawnMonsters_D	dd 712,731,732,0,0,0,0,0,0,0 ; 10种怪物的HCIDX

SpawnProbability_M dd 30
MaxSpawnNum_M dd 30
SpawnInterv_M	dd 5
ActivArea_M		dd 30
StallTime_M		dd 2
TypeOfSpawns_M	dd 6 ; 最多10种
SpawnMonsters_M	dd 725,726,727,728,729,730,0,0,0,0 ; 10种怪物的HCIDX

; ecx = ptGame
; edx = pMonster
; arg_0 = pPar

UberSpawnerMeph_AI proc

var_28          = dword ptr -28h
arg_0           = dword ptr  4

                sub     esp, 28h
                mov     eax, [esp+28h+arg_0] ; pPar
                push    ebx
                push    ebp
                mov     ebp, [eax]      ; pPar->hControl
                mov     ebx, ecx        ; ptGame
                push    esi
                push    edi
                mov     esi, edx        ; pMonster
                mov     edx, ActivArea_M ; 激活区域
                cmp     [eax+14h], edx  ; pPar->DistanceToTarget
                jg      short Meph_6FC886C2
                mov     edx, [ebx+0A8h] ; 游戏当前时间
                cmp     edx, [esi+64h]  ; 下一个Spawn的时间
                jge     short Meph_6FC886D9

Meph_6FC886C2:
								xor			eax,eax
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 28h
                retn    4
Meph_6FC886D9:
                push    offset D2GAME_GetAreaSpawnUnitNumUberMeph_CheckFunc
                push    esi             ; pMonster
                lea     ecx, [esp+40h+var_28]
                mov     eax, 1          ; 本次检查类型，1=monster
                mov     edx, ebx        ; ptGame
                mov     [esp+40h+var_28], 0
                call    D2GAME_GetAreaSpawnUnitNum
                mov     ecx, MaxSpawnNum_M ; 相应难度下的AI Par2，该Area的Spawn最大数目
                cmp     [esp+38h+var_28], ecx ; Spawn最大数目
                jge     Meph_6FC8874F

								push		ebx
								call		GetExtendGameInfoStructPtr
								push		ecx
								lea			ecx,[eax+ExtendGameInfoStruct.Meph_rand_h]
								push		ecx
								lea			ecx,[eax+ExtendGameInfoStruct.Meph_rand_l]
								push		ecx
								push		TypeOfSpawns_M
                call		RandomStep
                pop			ecx
                mov			eax,SpawnMonsters_M[eax*4]

                push		eax
                mov			ecx,ebx
                mov			edx,esi
                call		MySpawnAMonster
                
                mov     edx, SpawnInterv_M ; 相应难度下的AI Par3，Spawn延迟
                mov     ecx, [ebx+0A8h] ; 游戏当前时间
                add     edx, ecx
                mov     [esi+64h], edx  ; 更新下一个Spawn的时间
                mov     eax, StallTime_M ; 相应难度下的AI Par2，停顿时间
                push    eax
                mov     eax, ebx
                call    D2GAME_StallSomeTime
                mov			eax,1
                jmp			Meph_over
                
Meph_6FC8874F:
								xor			eax,eax
Meph_over:
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 28h
                retn    4
UberSpawnerMeph_AI endp

; ecx = ptGame
; edx = pMonster
; arg_0 = pPar

UberSpawnerDiablo_AI proc

var_28          = dword ptr -28h
arg_0           = dword ptr  4

                sub     esp, 28h
                mov     eax, [esp+28h+arg_0] ; pPar
                push    ebx
                push    ebp
                mov     ebp, [eax]      ; pPar->hControl
                mov     ebx, ecx        ; ptGame
                push    esi
                push    edi
                mov     esi, edx        ; pMonster
                mov     edx, ActivArea_D ; 激活区域
                cmp     [eax+14h], edx  ; pPar->DistanceToTarget
                jg      short Diablo_6FC886C2
                mov     edx, [ebx+0A8h] ; 游戏当前时间
                cmp     edx, [esi+64h]  ; 下一个Spawn的时间
                jge     short Diablo_6FC886D9

Diablo_6FC886C2:
								xor			eax,eax
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 28h
                retn    4
Diablo_6FC886D9:
                push    offset D2GAME_GetAreaSpawnUnitNumUberDiablo_CheckFunc
                push    esi             ; pMonster
                lea     ecx, [esp+40h+var_28]
                mov     eax, 1          ; 本次检查类型，1=monster
                mov     edx, ebx        ; ptGame
                mov     [esp+40h+var_28], 0
                call    D2GAME_GetAreaSpawnUnitNum
                mov     ecx, MaxSpawnNum_D ; 相应难度下的AI Par2，该Area的Spawn最大数目
                cmp     [esp+38h+var_28], ecx ; Spawn最大数目
                jge     Diablo_6FC8874F

								push		ebx
								call		GetExtendGameInfoStructPtr
								push		ecx
								lea			ecx,[eax+ExtendGameInfoStruct.Diablo_rand_h]
								push		ecx
								lea			ecx,[eax+ExtendGameInfoStruct.Diablo_rand_l]
								push		ecx
								push		TypeOfSpawns_D
                call		RandomStep
                pop			ecx
                mov			eax,SpawnMonsters_D[eax*4]

                push		eax
                mov			ecx,ebx
                mov			edx,esi
                call		MySpawnAMonster
                
                mov     edx, SpawnInterv_D ; 相应难度下的AI Par3，Spawn延迟
                mov     ecx, [ebx+0A8h] ; 游戏当前时间
                add     edx, ecx
                mov     [esi+64h], edx  ; 更新下一个Spawn的时间
                mov     eax, StallTime_D ; 相应难度下的AI Par2，停顿时间
                push    eax
                mov     eax, ebx
                call    D2GAME_StallSomeTime
                mov			eax,1
                jmp			Diablo_over
                
Diablo_6FC8874F:
								xor			eax,eax
Diablo_over:
                pop     edi
                pop     esi
                pop     ebp
                pop     ebx
                add     esp, 28h
                retn    4
UberSpawnerDiablo_AI endp

D2GAME_GetAreaSpawnUnitNumUberDiablo_CheckFunc proc

arg_0           = dword ptr  8

                push    esi
                mov     esi, [esp+arg_0]
                cmp     edx, esi
                jz      short loc_6FC859E6
                test    esi, esi
                jz      short loc_6FC859E6
                cmp     dword ptr [esi], 1
                jnz     short loc_6FC859E6

	; eax=HCIDX       ; 检查是否是minion1（453=1C5）到minion8（460=1CC）
	push ebx
	push ecx
	
	mov eax,[esi+4]
	
	mov ebx, TypeOfSpawns_D
again:
	dec ebx
	mov ecx, SpawnMonsters_D[ebx*4]
	cmp ecx,eax
	jz  ok
	test ebx,ebx
	jnz again
	pop ecx
	pop ebx
	jmp loc_6FC859E6
ok:
	mov eax,[esi+10h]
	cmp eax,12
	jz  corpse_found
	cmp eax,0
	jnz monster_found
corpse_found:
	mov eax,esi
	pop ecx
	push ecx
	push ecx
	call D2GAME_RemoveMonsterCorpse
	pop ecx
	pop ebx
	jmp loc_6FC859E6
monster_found:
	pop ecx
	pop ebx
	
loc_6FC859D6:
                mov     eax, [esp+0Ch]
                inc     dword ptr [eax]

loc_6FC859E6:
                xor     eax, eax
                pop     esi
                retn    8
D2GAME_GetAreaSpawnUnitNumUberDiablo_CheckFunc endp ; sp = -8

D2GAME_GetAreaSpawnUnitNumUberMeph_CheckFunc proc

arg_0           = dword ptr  8

                push    esi
                mov     esi, [esp+arg_0]
                cmp     edx, esi
                jz      short loc_6FC859E6
                test    esi, esi
                jz      short loc_6FC859E6
                cmp     dword ptr [esi], 1
                jnz     short loc_6FC859E6

	; eax=HCIDX       ; 检查是否是minion1（453=1C5）到minion8（460=1CC）
	push ebx
	push ecx
	
	mov eax,[esi+4]
	
	mov ebx, TypeOfSpawns_M
again:
	dec ebx
	mov ecx, SpawnMonsters_M[ebx*4]
	cmp ecx,eax
	jz  ok
	test ebx,ebx
	jnz again
	pop ecx
	pop ebx
	jmp loc_6FC859E6
ok:
	mov eax,[esi+10h]
	cmp eax,12
	jz  corpse_found
	cmp eax,0
	jnz monster_found
corpse_found:
	mov eax,esi
	pop ecx
	push ecx
	push ecx
	call D2GAME_RemoveMonsterCorpse
	pop ecx
	pop ebx
	jmp loc_6FC859E6
monster_found:
	pop ecx
	pop ebx
	
loc_6FC859D6:
                mov     eax, [esp+0Ch]
                inc     dword ptr [eax]

loc_6FC859E6:
                xor     eax, eax
                pop     esi
                retn    8
D2GAME_GetAreaSpawnUnitNumUberMeph_CheckFunc endp ; sp = -8

MySpawnAMonster proc
	; ecx=ptGame
	; edx=pMonster
	; arg_0 = HCIDX
	push esi
	push edi
	push ecx
	push edx
	
	mov esi,ecx
	mov edi,edx
	
	
	push 0FFFFFFFFh
 	push 0
 	push 0
	
	push edi
	call D2Common_10331_GethRoomFUnit ; GetUnit_hRoom
	push eax ; hRoom

	push esi
	
	mov     eax,0
	mov     ecx,[esp+28h]
	mov     edx,0
	call    D2GAME_0X24950_SpawnSuperUnique
	
	test eax,eax
	jz over
	mov ecx,DisableUberUp
	test ecx,ecx
	jz over
	mov ecx,[eax+0C4h]
	or ecx,4020020h				; 杀死怪物不获得经验值，不掉落，尸体消失
	mov [eax+0C4h],ecx
over:
	pop edx
	pop ecx
	pop edi
	pop esi
	retn 4
MySpawnAMonster endp

;给sgptDataTables打补丁，对多人游戏的TC NoDrop进行预计算
Patch_sgptDataTables proc
	push eax
	push ecx

	push 134 ; get the level 134 record
	call D2Common_10014_GetLevelTxt
	test eax,eax
	jz over
	mov cx,0FFh
	mov word ptr [eax+0E4h],cx ; patch level 134, has no waypoint!!
over:
	pop ecx
	pop eax
	ret
Patch_sgptDataTables endp


; D2 Random Number
; arg_0 = divv arg_4= ptr rand_l arg_8 = ptr rand_h
; mod result=eax
RandomStep proc
		push edi
		push ecx
		push edx
		mov     eax, [esp+12+12] ; rand_h
		mov			eax, [eax]
		mov     ecx, 6AC690C5h
		mul     ecx
		mov     edi, eax
		mov     eax, [esp+12+8]	; rand_l
		mov			eax, [eax]
		xor     ecx, ecx
		add     edi, eax
		adc     edx, ecx
		mov			eax, [esp+12+8]	; rand_l
		mov     [eax], edx
		mov     eax, [esp+12+12] ; rand_h
		mov     [eax], edi
		xor			eax,eax
		mov			ecx, [esp+12+4]	; divv
		cmp			ecx,0
		jz			over
		mov			eax,edi
		xor			edx, edx
		div			ecx
		mov			eax,edx
over:
		pop edx
		pop ecx
		pop edi
		retn 12
RandomStep endp

UberQuestPatchInit proc
	push ecx
	push edx
	push ebx
	push esi
	
	mov eax,6800B03Ch
	mov ebx,[eax] ; GetProcAddr
	mov eax,D2COMMON
	mov esi,[eax]
	
	push 10141
	push esi
	call ebx; GetProcAddr
	mov D2Common_10141_GetMyPosition,eax
	
	push 10826
	push esi
	call ebx; GetProcAddr
	mov D2Common_10826_GetLevelIdFromRoom,eax
	
	push 10654
	push esi
	call ebx; GetProcAddr
	mov D2Common_10654_GetFreeTile,eax
	
	push 10258
	push esi
	call ebx; GetProcAddr
	mov D2Common_10258_GetPortalFlag,eax
	
	push 10111
	push esi
	call ebx; GetProcAddr
	mov D2Common_10111_UpdatePortalFlag,eax
	
	push 10346
	push esi
	call ebx; GetProcAddr
	mov D2Common_10346_UpdateRoomHavePortalFlag,eax
	
	push 10331
	push esi
	call ebx; GetProcAddr
	mov D2Common_10331_GethRoomFUnit,eax
	
	push 10174
	push esi
	call ebx; GetProcAddr
	mov D2Common_10174_GetQuestFlag,eax
	
	push 10003
	push esi
	call ebx; GetProcAddr
	mov D2Common_10003_SetQuestFlag,eax
	
	push 10014
	push esi
	call ebx; GetProcAddr
	mov D2Common_10014_GetLevelTxt,eax
	
	mov eax,D2GAME
	mov edx,[eax]
	
	mov eax,edx
	add eax,1280h
	mov D2GAME_0X1280_GetFreeTitle,eax
	
	mov eax,edx
	add eax,0E1D90h
	mov D2GAME_0XE1D90_InitTheUnit,eax
	
	mov eax,edx
	add eax,0A22E0h
	mov D2GAME_0XA22E0_CreatePortalInTown,eax
	
	mov eax,edx
	add eax,200E0h
	mov D2GAME_0X200E0,eax
	
	mov eax,edx
	add eax,24950h
	mov D2GAME_0X24950_SpawnSuperUnique,eax
	
	mov eax,edx
	add eax,20760h
	mov D2GAME_RemoveMonsterCorpse,eax ;6FC43C40
	
	mov eax,edx
	add eax,0B8610h
	mov D2GAME_Baal_AI,eax ; 6FC4BC80

	mov eax,edx
	add eax,85B60h
	mov D2GAME_Mephisto_AI,eax ; 6FCF7BD0

	mov eax,edx
	add eax,0A9610h
	mov D2GAME_Diablo_AI,eax ; 6FCA5AA0

	mov eax,edx
	add eax,0F22C0h
	mov D2GAME_StallSomeTime,eax ; 6FD10CF0

	mov eax,edx
	add eax,0B17B0h
	mov D2GAME_GetAreaSpawnUnitNum,eax ; 6FCE2650

	mov eax,edx
	add eax,0F2380h
	mov D2GAME_UseASkill,eax ; 6FD10DB0

	mov eax,edx
	add eax,0EF650h
	mov D2GAME_SpawnAMonster,eax ; 6FD0F870

	mov eax,edx
	add eax,0F829Ch
	mov D2GAME_sgptDataTables,eax ; 6FD18288
	
	mov eax,edx
	add eax,1220h
	mov D2GAME_GetMonstatsTxtRec,eax ; 6FC21180
	
	mov eax,edx
	add eax,8E6F0h
	mov D2GAME_DestoryAEvent,eax ; 6FC90C50
	
	mov eax,edx
	add eax,0A240Bh
	mov D2GAME_0XA240B,eax ; 6FC30B1B
	
	mov eax,6800B03Ch
	mov ebx,[eax] ; GetProcAddr
	mov eax,D2NET
	mov esi,[eax]
	
	pop esi
	pop ebx
	pop edx
	pop ecx
	ret
UberQuestPatchInit endp

D2GAME_0X1280_GetFreeTitle_1 proc
	arg_0= dword ptr  0Ch
	
	push    edi
	push    ebx
	db      36h
	mov     edi, [esp+arg_0]
	mov     eax, ecx
	mov     ebx, edx
	call    D2GAME_0X1280_GetFreeTitle
	pop     ebx
	pop     edi
	retn    4
D2GAME_0X1280_GetFreeTitle_1 endp


; arg_0 = ptGame
; eax=ExtendGameInfoStructPtr
GetExtendGameInfoStructPtr proc
	mov eax,[esp+4]
	mov ax,[eax+28h]
	push eax
	call GetGameInfo
	add esp,4
	mov eax,[eax+4]
	retn 4
GetExtendGameInfoStructPtr endp
