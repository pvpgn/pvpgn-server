.386 
.model flat, c

.data
unk1	db 256 dup(0)
unk_18C3010 db 80h
unk2	db 256 dup(0)

.code 

@HashGameSeed@16 proc SYSCALL; ecx=pt_Warden_struct(pt->HashResult),edx=pt_0X66_RC4_Key,arg_0=GameSeed,arg_4=4
	var_54= dword ptr -54h
	var_14= dword ptr -14h
	var_4= dword ptr -4
	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch       
	
	push    ebp
	mov     ebp, esp
	sub     esp, 54h
	push    ebx
	push    esi
	push    edi
	
	push ecx
	push edx
	
	
	loc_18C28BD:                            ; CODE XREF: sub_18C2891+25j
	push    [ebp+arg_0]						; Warden_GameSeed Offset
	mov     eax, [ebp+arg_4]			; 4 ，参数push 4
	lea     esi, [ebp+var_54]
	call    sub_18C1098

	push    10h
	lea     eax, [ebp+var_14]
	push    eax
	mov     eax, esi
	call    ManyBSHA______A
	
	pop edx
	lea esi,[ebp+var_14]
	mov edi,edx
	mov ecx,4
	rep movsd
	
	pop			eax								; 16B Session RC4 Key Store here
	push    10h
	push    eax
	lea     eax, [ebp+var_54]
	call    ManyBSHA______A
	
	pop     edi
	pop     esi
	pop     ebx

	add			esp, 54h
	pop			ebp
	retn 8
@HashGameSeed@16 endp

sub_18C1098 proc near

	var_4= dword ptr -4
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	push    ecx
	push    ebx
	mov     ebx, eax
	push    edi
	mov     [ebp+var_4], ebx
	shr     [ebp+var_4], 1
	push    [ebp+var_4]
	lea     eax, [esi+18h]
	push    [ebp+arg_0]
	push    eax
	call    sub_18C23BC
	mov     ecx, [ebp+arg_0]
	xor     eax, eax
	lea     edi, [esi+4]
	stosd
	stosd
	stosd
	stosd
	stosd
	mov     eax, [ebp+var_4]
	sub     ebx, eax
	push    ebx
	add     eax, ecx
	push    eax
	lea     eax, [esi+2Ch]
	push    eax
	call    sub_18C23BC
	mov     edi, esi
	call    sub_18C1112
	pop     edi
	pop     ebx
	leave
	retn    4
sub_18C1098 endp

RC4_Init proc near
	var_8= dword ptr -8
	var_2= byte ptr -2
	var_1= byte ptr -1
	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	
	push    ebp
	mov     ebp, esp
	push    ecx
	push    ecx
	xor     edx, edx
	xor     eax, eax
	
	loc_18C147C:                            ; CODE XREF: RC4_Init+12j
	mov     [eax+esi], al
	inc     eax
	cmp     eax, 100h
	jb      short loc_18C147C
	push    ebx
	mov     [esi+100h], dl
	mov     [esi+101h], dl
	mov     [ebp+var_1], dl
	mov     [ebp+var_8], edx
	push    edi
	
	loc_18C149B:                            ; CODE XREF: RC4_Init+5Cj
	mov     eax, [ebp+var_8]
	lea     edi, [eax+esi]
	mov     eax, [ebp+arg_0]
	movzx   ecx, dl
	mov     al, [ecx+eax]
	add     al, [edi]
	mov     dl, [edi]
	add     [ebp+var_1], al
	movzx   eax, [ebp+var_1]
	add     eax, esi
	mov     bl, [eax]
	mov     [edi], bl
	mov     [eax], dl
	xor     edx, edx
	lea     eax, [ecx+1]
	div     [ebp+arg_4]
	inc     [ebp+var_8]
	cmp     [ebp+var_8], 100h
	jb      short loc_18C149B
	pop     edi
	pop     ebx
	leave
	retn    8
RC4_Init endp

sub_18C23BC proc near                   ; CODE XREF: sub_18C1098+18p
                                        ; sub_18C1098+37p

	var_60= dword ptr -60h
	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	arg_8= dword ptr  10h
	
	push    ebp
	mov     ebp, esp
	sub     esp, 60h
	push    esi
	lea     eax, [ebp+var_60]
	call    BSHA_Init
	push    [ebp+arg_8]
	mov     eax, [ebp+arg_4]
	lea     esi, [ebp+var_60]
	call    Call_BSHA_Process
	push    [ebp+arg_0]
	mov     eax, esi
	call    sub_18C2300
	pop     esi
	leave
	retn    0Ch
sub_18C23BC endp

ManyBSHA______A proc near                   ; CODE XREF: sub_18C2891+42p
                                        ; sub_18C2891+5Dp

	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	
	push    esi
	xor     esi, esi
	cmp     [esp+arg_4], esi
	push    edi
	mov     edi, eax
	jbe     short loc_18C110D
	
	loc_18C10ED:                            ; CODE XREF: ManyBSHA______A+2Aj
	cmp     dword ptr [edi], 14h
	jnz     short loc_18C10F7
	call    sub_18C1112
	
	loc_18C10F7:                            ; CODE XREF: ManyBSHA______A+Fj
	mov     eax, [edi]
	mov     al, [eax+edi+4]
	mov     ecx, [esp+4+arg_0]
	mov     [esi+ecx], al
	inc     dword ptr [edi]
	inc     esi
	cmp     esi, [esp+4+arg_4]
	jb      short loc_18C10ED
	
	loc_18C110D:                            ; CODE XREF: ManyBSHA______A+Aj
	pop     edi
	pop     esi
	retn    8
ManyBSHA______A endp

sub_18C1112 proc near                   ; CODE XREF: sub_18C1098+3Ep
                                        ; ManyBSHA______A+11p

	var_60= dword ptr -60h
	
	push    ebp
	mov     ebp, esp
	sub     esp, 60h
	push    ebx
	push    esi
	lea     eax, [ebp+var_60]
	call    BSHA_Init
	push    14h
	lea     eax, [edi+18h]
	lea     esi, [ebp+var_60]
	call    Call_BSHA_Process
	lea     ebx, [edi+4]
	push    14h
	mov     eax, ebx
	call    Call_BSHA_Process
	push    14h
	lea     eax, [edi+2Ch]
	call    Call_BSHA_Process
	push    ebx
	mov     eax, esi
	call    sub_18C2300
	and     dword ptr [edi], 0
	pop     esi
	pop     ebx
	leave
	retn
sub_18C1112 endp

BSHA_Init proc near                   ; CODE XREF: sub_18C1112+Bp
                                        ; sub_18C2383+9p ...
	and     dword ptr [eax], 0
	and     dword ptr [eax+4], 0
	mov     dword ptr [eax+8], 67452301h
	mov     dword ptr [eax+0Ch], 0EFCDAB89h
	mov     dword ptr [eax+10h], 98BADCFEh
	mov     dword ptr [eax+14h], 10325476h
	mov     dword ptr [eax+18h], 0C3D2E1F0h
	retn
BSHA_Init endp

Call_BSHA_Process proc near                   ; CODE XREF: sub_18C1112+18p
                                        ; sub_18C1112+24p ...

	var_8= dword ptr -8
	var_4= dword ptr -4
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	push    ecx
	push    ecx
	mov     ecx, [ebp+arg_0]
	push    ebx
	push    edi
	mov     edi, [esi]
	shr     edi, 3
	shl     ecx, 3
	and     edi, 3Fh
	add     [esi], ecx
	mov     [ebp+var_4], eax
	adc     dword ptr [esi+4], 0
	test    edi, edi
	jz      short loc_18C22C2
	add     [ebp+arg_0], edi
	sub     eax, edi
	cmp     [ebp+arg_0], 40h
	mov     [ebp+var_4], eax
	jb      short loc_18C22F5
	jmp     short loc_18C22A7
	; 哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪?
	
	loc_18C229C:                            ; CODE XREF: Call_BSHA_Process+40j
	mov     eax, [ebp+var_4]
	mov     al, [edi+eax]
	mov     [esi+edi+1Ch], al
	inc     edi
	
	loc_18C22A7:                            ; CODE XREF: Call_BSHA_Process+30j
	cmp     edi, 40h
	jb      short loc_18C229C
	lea     eax, [esi+1Ch]
	push    eax
	lea     ebx, [esi+8]
	call    BSHA_Process
	add     [ebp+var_4], 40h
	sub     [ebp+arg_0], 40h
	xor     edi, edi
	
	loc_18C22C2:                            ; CODE XREF: Call_BSHA_Process+20j
	mov     eax, [ebp+arg_0]
	cmp     eax, 40h
	jb      short loc_18C22F5
	shr     eax, 6
	lea     ebx, [esi+8]
	mov     [ebp+var_8], eax
	
	loc_18C22D3:                            ; CODE XREF: Call_BSHA_Process+7Cj
	push    [ebp+var_4]
	call    BSHA_Process
	add     [ebp+var_4], 40h
	sub     [ebp+arg_0], 40h
	dec     [ebp+var_8]
	jnz     short loc_18C22D3
	jmp     short loc_18C22F5
	; 哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪哪?
	
	loc_18C22EA:                            ; CODE XREF: Call_BSHA_Process+8Ej
	mov     eax, [ebp+var_4]
	mov     al, [edi+eax]
	mov     [esi+edi+1Ch], al
	inc     edi
	
	loc_18C22F5:                            ; CODE XREF: Call_BSHA_Process+2Ej
	                                        ; Call_BSHA_Process+5Ej ...
	cmp     edi, [ebp+arg_0]
	jb      short loc_18C22EA
	pop     edi
	pop     ebx
	leave
	retn    4
Call_BSHA_Process endp

sub_18C2300 proc near                   ; CODE XREF: sub_18C1112+36p
                                        ; sub_18C2383+2Fp ...

	var_8= dword ptr -8
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	push    ecx
	push    ecx
	push    esi
	mov     esi, eax
	push    edi
	lea     eax, [ebp+var_8]
	push    eax
	push    dword ptr [esi+4]
	push    dword ptr [esi]
	call    sub_18C135B
	mov     eax, [esi]
	push    0FFFFFFF7h
	shr     eax, 3
	pop     ecx
	sub     ecx, eax
	and     ecx, 3Fh
	inc     ecx
	push    ecx
	mov     eax, offset unk_18C3010
	call    Call_BSHA_Process
	push    8
	lea     eax, [ebp+var_8]
	call    Call_BSHA_Process
	mov     edi, [ebp+arg_0]
	push    5
	lea     eax, [esi+8]
	pop     esi
	
	loc_18C2343:                            ; CODE XREF: sub_18C2300+58j
	mov     ecx, [eax]
	push    3
	pop     edx
	
	loc_18C2348:                            ; CODE XREF: sub_18C2300+4Fj
	mov     [edi+edx], cl
	shr     ecx, 8
	dec     edx
	jns     short loc_18C2348
	add     eax, 4
	add     edi, 4
	dec     esi
	jnz     short loc_18C2343
	pop     edi
	pop     esi
	leave
	retn    4
sub_18C2300 endp

BSHA_Process proc near                   ; CODE XREF: Call_BSHA_Process+49p
                                        ; Call_BSHA_Process+6Cp

	var_15C= dword ptr -15Ch
	var_158= dword ptr -158h
	var_154= dword ptr -154h
	var_108= dword ptr -108h
	var_1C= dword ptr -1Ch
	var_18= dword ptr -18h
	var_14= dword ptr -14h
	var_10= dword ptr -10h
	var_C= dword ptr -0Ch
	var_8= dword ptr -8
	var_4= dword ptr -4
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	sub     esp, 15Ch
	push    esi
	push    edi
	push    10h
	pop     eax
	push    4
	lea     esi, [ebp+var_15C]
	mov     [ebp+var_14], eax
	mov     [ebp+var_18], eax
	pop     edi
	
	loc_18C1F6E:                            ; CODE XREF: BSHA_Process+2Ej
	mov     eax, [ebp+arg_0]
	push    esi
	call    sub_18C1339
	add     [ebp+arg_0], edi
	add     esi, edi
	dec     [ebp+var_14]
	jnz     short loc_18C1F6E
	lea     ecx, [ebp+var_154]
	
	loc_18C1F87:                            ; CODE XREF: BSHA_Process+56j
	mov     eax, [ecx+2Ch]
	xor     eax, [ecx+18h]
	xor     eax, [ecx-8]
	xor     eax, [ecx]
	mov     edx, eax
	shr     edx, 1Fh
	add     eax, eax
	or      edx, eax
	inc     [ebp+var_18]
	mov     [ecx+38h], edx
	add     ecx, edi
	cmp     [ebp+var_18], 50h
	jl      short loc_18C1F87
	mov     eax, [ebx]
	mov     [ebp+arg_0], eax
	mov     eax, [ebx+4]
	mov     [ebp+var_8], eax
	mov     eax, [ebx+8]
	mov     [ebp+var_4], eax
	mov     eax, [ebx+0Ch]
	mov     [ebp+var_C], eax
	mov     eax, [ebx+10h]
	mov     [ebp+var_10], eax
	lea     eax, [ebp+var_158]
	mov     [ebp+var_14], eax
	mov     [ebp+var_1C], edi
	mov     [ebp+var_18], 14h
	
	loc_18C1FD9:                            ; CODE XREF: BSHA_Process+10Bj
	push    [ebp+var_C]
	mov     esi, [ebp+var_14]
	push    [ebp+var_4]
	mov     edx, [ebp+arg_0]
	add     esi, 0FFFFFFFCh
	lea     edi, [ebp+var_10]
	lea     ecx, [ebp+var_8]
	call    BSHA_OP1
	push    [ebp+var_4]
	mov     esi, [ebp+var_14]
	push    [ebp+var_8]
	mov     edx, [ebp+var_10]
	lea     edi, [ebp+var_C]
	lea     ecx, [ebp+arg_0]
	call    BSHA_OP1
	push    [ebp+var_8]
	mov     edx, [ebp+var_C]
	push    [ebp+arg_0]
	add     esi, 4
	lea     edi, [ebp+var_4]
	lea     ecx, [ebp+var_10]
	call    BSHA_OP1
	push    [ebp+arg_0]
	mov     esi, [ebp+var_14]
	push    [ebp+var_10]
	mov     edx, [ebp+var_4]
	add     esi, 8
	lea     edi, [ebp+var_8]
	lea     ecx, [ebp+var_C]
	call    BSHA_OP1
	push    [ebp+var_10]
	mov     esi, [ebp+var_14]
	push    [ebp+var_C]
	mov     edx, [ebp+var_8]
	add     esi, 0Ch
	lea     edi, [ebp+arg_0]
	lea     ecx, [ebp+var_4]
	call    BSHA_OP1
	add     [ebp+var_14], 14h
	dec     [ebp+var_1C]
	jnz     loc_18C1FD9
	lea     esi, [ebp+var_108]
	
	loc_18C2068:                            ; CODE XREF: BSHA_Process+199j
	lea     eax, [ebp+var_10]
	push    eax
	push    [ebp+var_C]
	mov     eax, [ebp+arg_0]
	push    [ebp+var_4]
	lea     edx, [esi-4]
	lea     ecx, [ebp+var_8]
	call    BSHA_OP2
	lea     eax, [ebp+var_C]
	push    eax
	push    [ebp+var_4]
	mov     eax, [ebp+var_10]
	push    [ebp+var_8]
	mov     edx, esi
	lea     ecx, [ebp+arg_0]
	call    BSHA_OP2
	lea     eax, [ebp+var_4]
	push    eax
	push    [ebp+var_8]
	mov     eax, [ebp+var_C]
	push    [ebp+arg_0]
	lea     edx, [esi+4]
	lea     ecx, [ebp+var_10]
	call    BSHA_OP2
	lea     eax, [ebp+var_8]
	push    eax
	push    [ebp+arg_0]
	mov     eax, [ebp+var_4]
	push    [ebp+var_10]
	lea     edx, [esi+8]
	lea     ecx, [ebp+var_C]
	call    BSHA_OP2
	lea     eax, [ebp+arg_0]
	push    eax
	push    [ebp+var_10]
	mov     eax, [ebp+var_8]
	push    [ebp+var_C]
	lea     edx, [esi+0Ch]
	lea     ecx, [ebp+var_4]
	call    BSHA_OP2
	add     [ebp+var_18], 5
	add     esi, 14h
	cmp     [ebp+var_18], 28h
	jl      loc_18C2068
	mov     eax, [ebp+var_18]
	lea     eax, [ebp+eax*4+var_158]
	mov     [ebp+var_14], eax
	
	loc_18C20FD:                            ; CODE XREF: BSHA_Process+234j
	push    [ebp+var_C]
	mov     esi, [ebp+var_14]
	push    [ebp+arg_0]
	mov     eax, [ebp+var_4]
	add     esi, 0FFFFFFFCh
	lea     edi, [ebp+var_10]
	lea     ecx, [ebp+var_8]
	call    BSHA_OP3
	push    [ebp+var_4]
	mov     esi, [ebp+var_14]
	push    [ebp+var_10]
	mov     eax, [ebp+var_8]
	lea     edi, [ebp+var_C]
	lea     ecx, [ebp+arg_0]
	call    BSHA_OP3
	push    [ebp+var_8]
	mov     eax, [ebp+arg_0]
	push    [ebp+var_C]
	add     esi, 4
	lea     edi, [ebp+var_4]
	lea     ecx, [ebp+var_10]
	call    BSHA_OP3
	push    [ebp+arg_0]
	mov     esi, [ebp+var_14]
	push    [ebp+var_4]
	mov     eax, [ebp+var_10]
	add     esi, 8
	lea     edi, [ebp+var_8]
	lea     ecx, [ebp+var_C]
	call    BSHA_OP3
	push    [ebp+var_10]
	mov     esi, [ebp+var_14]
	push    [ebp+var_8]
	mov     eax, [ebp+var_C]
	add     esi, 0Ch
	lea     edi, [ebp+arg_0]
	lea     ecx, [ebp+var_4]
	call    BSHA_OP3
	add     [ebp+var_18], 5
	add     [ebp+var_14], 14h
	cmp     [ebp+var_18], 3Ch
	jl      loc_18C20FD
	mov     esi, [ebp+var_18]
	lea     esi, [ebp+esi*4+var_158]
	
	loc_18C2195:                            ; CODE XREF: BSHA_Process+2C5j
	lea     eax, [ebp+var_10]
	push    eax
	push    [ebp+var_C]
	mov     eax, [ebp+arg_0]
	push    [ebp+var_4]
	lea     edx, [esi-4]
	lea     ecx, [ebp+var_8]
	call    BSHA_OP4
	lea     eax, [ebp+var_C]
	push    eax
	push    [ebp+var_4]
	mov     eax, [ebp+var_10]
	push    [ebp+var_8]
	mov     edx, esi
	lea     ecx, [ebp+arg_0]
	call    BSHA_OP4
	lea     eax, [ebp+var_4]
	push    eax
	push    [ebp+var_8]
	mov     eax, [ebp+var_C]
	push    [ebp+arg_0]
	lea     edx, [esi+4]
	lea     ecx, [ebp+var_10]
	call    BSHA_OP4
	mov     edi, [ebp+var_10]
	lea     eax, [ebp+var_8]
	push    eax
	push    [ebp+arg_0]
	mov     eax, [ebp+var_4]
	lea     edx, [esi+8]
	push    edi
	lea     ecx, [ebp+var_C]
	call    BSHA_OP4
	lea     eax, [ebp+arg_0]
	push    eax
	mov     eax, [ebp+var_8]
	push    edi
	push    [ebp+var_C]
	lea     edx, [esi+0Ch]
	lea     ecx, [ebp+var_4]
	call    BSHA_OP4
	add     [ebp+var_18], 5
	add     esi, 14h
	cmp     [ebp+var_18], 50h
	jl      loc_18C2195
	mov     ecx, [ebp+arg_0]
	mov     eax, [ebx]
	add     eax, ecx
	mov     ecx, [ebp+var_8]
	mov     [ebx], eax
	mov     eax, [ebx+4]
	add     eax, ecx
	mov     ecx, [ebp+var_4]
	mov     [ebx+4], eax
	mov     eax, [ebx+8]
	add     eax, ecx
	mov     ecx, [ebp+var_C]
	mov     [ebx+8], eax
	mov     eax, [ebx+0Ch]
	add     eax, ecx
	mov     [ebx+0Ch], eax
	mov     eax, [ebx+10h]
	add     eax, edi
	pop     edi
	mov     [ebx+10h], eax
	pop     esi
	leave
	retn    4
BSHA_Process endp

sub_18C135B proc near                   ; CODE XREF: sub_18C2300+12p

	arg_0= dword ptr  4
	arg_4= dword ptr  8
	arg_8= dword ptr  0Ch
	
	mov     eax, [esp+arg_0]
	push    7
	pop     ecx
	
	loc_18C1362:                            ; CODE XREF: sub_18C135B+1Ej
	mov     edx, [esp+arg_8]
	mov     [ecx+edx], al
	mov     edx, [esp+arg_4]
	shrd    eax, edx, 8
	shr     edx, 8
	dec     ecx
	mov     [esp+arg_4], edx
	jns     short loc_18C1362
	retn    0Ch
sub_18C135B endp

sub_18C1339 proc near                   ; CODE XREF: BSHA_Process+21p

	arg_0= dword ptr  4
	
	movzx   edx, byte ptr [eax+2]
	xor     ecx, ecx
	mov     ch, [eax]
	mov     cl, [eax+1]
	movzx   eax, byte ptr [eax+3]
	shl     ecx, 8
	or      ecx, edx
	shl     ecx, 8
	or      ecx, eax
	mov     eax, [esp+arg_0]
	mov     [eax], ecx
	retn    4
sub_18C1339 endp

BSHA_OP1 proc near                   ; CODE XREF: BSHA_Process+9Dp
                                        ; BSHA_Process+B4p ...

	arg_0= dword ptr  4
	arg_4= dword ptr  8
	
	mov     eax, [ecx]
	push    ebx
	mov     ebx, eax
	and     eax, [esp+4+arg_0]
	not     ebx
	and     ebx, [esp+4+arg_4]
	or      ebx, eax
	mov     eax, edx
	shr     eax, 1Bh
	shl     edx, 5
	or      eax, edx
	add     ebx, eax
	mov     eax, [esi]
	lea     eax, [ebx+eax+5A827999h]
	add     [edi], eax
	mov     eax, [ecx]
	mov     edx, eax
	shl     edx, 1Eh
	shr     eax, 2
	or      edx, eax
	mov     [ecx], edx
	and     dword ptr [esi], 0
	pop     ebx
	retn    8
BSHA_OP1 endp

BSHA_OP2 proc near                   ; CODE XREF: BSHA_Process+12Ap
                                        ; BSHA_Process+141p ...

	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	arg_8= dword ptr  10h
	
	push    esi
	mov     esi, [esp+arg_8]
	push    edi
	mov     edi, eax
	shl     eax, 5
	shr     edi, 1Bh
	or      edi, eax
	mov     eax, [ecx]
	xor     eax, [esp+4+arg_0]
	xor     eax, [esp+4+arg_4]
	add     edi, eax
	mov     eax, [edx]
	lea     eax, [edi+eax+6ED9EBA1h]
	add     [esi], eax
	mov     eax, [ecx]
	mov     esi, eax
	shl     esi, 1Eh
	shr     eax, 2
	or      esi, eax
	mov     [ecx], esi
	and     dword ptr [edx], 0
	pop     edi
	pop     esi
	retn    0Ch
BSHA_OP2 endp

BSHA_OP3 proc near                   ; CODE XREF: BSHA_Process+1C1p
                                        ; BSHA_Process+1D8p ...

	arg_0= dword ptr  4
	arg_4= dword ptr  8
	
	mov     edx, [esp+arg_0]
	push    ebx
	mov     ebx, eax
	and     eax, [esp+4+arg_4]
	or      ebx, [esp+4+arg_4]
	and     ebx, [ecx]
	or      ebx, eax
	mov     eax, edx
	shr     eax, 1Bh
	shl     edx, 5
	or      eax, edx
	add     ebx, eax
	mov     eax, [esi]
	lea     eax, [ebx+eax-70E44324h]
	add     [edi], eax
	mov     eax, [ecx]
	mov     edx, eax
	shl     edx, 1Eh
	shr     eax, 2
	or      edx, eax
	mov     [ecx], edx
	and     dword ptr [esi], 0
	pop     ebx
	retn    8
BSHA_OP3 endp

BSHA_OP4 proc near                   ; CODE XREF: BSHA_Process+257p
                                        ; BSHA_Process+26Ep ...

	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	arg_8= dword ptr  10h
	
	push    esi
	mov     esi, [esp+arg_8]
	push    edi
	mov     edi, eax
	shl     eax, 5
	shr     edi, 1Bh
	or      edi, eax
	mov     eax, [ecx]
	xor     eax, [esp+4+arg_0]
	xor     eax, [esp+4+arg_4]
	add     edi, eax
	mov     eax, [edx]
	lea     eax, [edi+eax-359D3E2Ah]
	add     [esi], eax
	mov     eax, [ecx]
	mov     esi, eax
	shl     esi, 1Eh
	shr     eax, 2
	or      esi, eax
	mov     [ecx], esi
	and     dword ptr [edx], 0
	pop     edi
	pop     esi
	retn    0Ch
BSHA_OP4 endp

;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
	@Double_MD5@12 proc SYSCALL;	ecx=data, edx=unk, arg_0=Buffer[16]
	
	var_A= byte ptr -0Ah
	var_8= dword ptr -8
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	push    ecx
	push    ecx
	push    ebx
	push    esi
	mov     ebx, ecx		; eax
	xor     esi, esi
	
	loc_18C1E22:
	mov     eax, esi
	sar     eax, 2
	mov     eax, [ebx+eax*4]
	mov     ecx, esi
	and     ecx, 3
	shl     ecx, 3
	shr     eax, cl
	inc     esi
	cmp     esi, 8
	mov     [ebp+esi-9], al
	jl      short loc_18C1E22
	mov     eax, [ebx]
	push    0FFFFFFF7h
	pop     ecx
	shr     eax, 3							; 完毕后eax=mod长度
	sub     ecx, eax
	and     ecx, 3Fh
	inc     ecx
	push    ecx
	push    offset unk_18C3010
	call    Call_MD5_Process
	push    8
	lea     eax, [ebp+var_8]
	push    eax
	call    Call_MD5_Process
	xor     esi, esi
	
	loc_18C1E64:
	mov     eax, esi
	sar     eax, 2
	mov     eax, [ebx+eax*4+8]
	mov     ecx, esi
	and     ecx, 3
	shl     ecx, 3
	shr     eax, cl
	mov     ecx, [ebp+arg_0]
	mov     [esi+ecx], al
	inc     esi
	cmp     esi, 10h
	jl      short loc_18C1E64
	pop     esi
	pop     ebx
	leave
	retn    4
	@Double_MD5@12 endp

	Call_MD5_Process proc near
	
	var_4= dword ptr -4
	arg_0= dword ptr  8
	arg_4= dword ptr  0Ch
	
	push    ebp
	mov     ebp, esp
	push    ecx
	mov     ecx, [ebp+arg_4]
	mov     eax, [ebx]
	shr     eax, 3
	mov     edx, ecx
	and     eax, 3Fh
	shl     edx, 3
	test    ecx, ecx
	push    esi
	mov     esi, [ebp+arg_0]
	mov     [ebp+var_4], ecx
	jbe     loc_18C1E12
	add     [ebx], edx
	push    edi
	mov     edi, ecx
	shr     edi, 1Dh
	add     [ebx+4], edi
	cmp     [ebx], edx
	mov     edi, [ebx+4]
	jnb     short loc_18C1D8D
	inc     edi
	mov     [ebx+4], edi
	
	loc_18C1D8D:
	test    eax, eax
	jz      short loc_18C1DD6
	lea     edx, [eax+ecx]
	cmp     edx, 40h
	jbe     short loc_18C1D9E
	push    40h
	pop     ecx
	sub     ecx, eax
	
	loc_18C1D9E:
	mov     esi, [ebp+arg_0]
	mov     [ebp+var_4], ecx
	mov     edx, ecx
	shr     ecx, 2
	lea     edi, [eax+ebx+18h]
	rep movsd
	mov     ecx, edx
	and     ecx, 3
	rep movsb
	mov     ecx, edx
	add     eax, ecx
	cmp     eax, 40h
	jl      short loc_18C1E11
	mov     eax, [ebp+arg_0]
	lea     esi, [ecx+eax]
	mov     eax, [ebp+arg_4]
	sub     eax, ecx
	lea     ecx, [ebx+18h]
	push    ebx
	mov     [ebp+var_4], eax
	call    MD5_Process
	
	loc_18C1DD6:
	mov     edi, [ebp+var_4]
	cmp     edi, 40h
	jl      short loc_18C1DF9
	shr     edi, 6
	mov     eax, edi
	neg     eax
	shl     eax, 6
	add     [ebp+var_4], eax
	
	loc_18C1DEB:
	push    ebx
	mov     ecx, esi
	call    MD5_Process
	add     esi, 40h
	dec     edi
	jnz     short loc_18C1DEB
	
	loc_18C1DF9:
	mov     ecx, [ebp+var_4]
	test    ecx, ecx
	jz      short loc_18C1E11
	mov     eax, ecx
	shr     ecx, 2
	lea     edi, [ebx+18h]
	rep movsd
	mov     ecx, eax
	and     ecx, 3
	rep movsb
	
	loc_18C1E11:
	pop     edi
	
	loc_18C1E12:
	pop     esi
	leave
	retn    8
	Call_MD5_Process endp

	MD5_Process proc near
	
	var_48= dword ptr -48h
	var_44= dword ptr -44h
	var_40= dword ptr -40h
	var_3C= dword ptr -3Ch
	var_38= dword ptr -38h
	var_34= dword ptr -34h
	var_30= dword ptr -30h
	var_2C= dword ptr -2Ch
	var_28= dword ptr -28h
	var_24= dword ptr -24h
	var_20= dword ptr -20h
	var_1C= dword ptr -1Ch
	var_18= dword ptr -18h
	var_14= dword ptr -14h
	var_10= dword ptr -10h
	var_C= dword ptr -0Ch
	var_8= dword ptr -8
	var_4= dword ptr -4
	arg_0= dword ptr  8
	
	push    ebp
	mov     ebp, esp
	sub     esp, 48h
	mov     eax, [ebp+arg_0]
	push    ebx
	push    esi
	xor     esi, esi
	push    edi
	add     ecx, 2
	
	loc_18C150C:
	movzx   edi, byte ptr [ecx-1]
	xor     edx, edx
	mov     dh, [ecx+1]
	mov     dl, [ecx]
	add     ecx, 4
	shl     edx, 8
	or      edx, edi
	movzx   edi, byte ptr [ecx-6]
	shl     edx, 8
	or      edx, edi
	mov     [ebp+esi*4+var_48], edx
	inc     esi
	cmp     esi, 10h
	jl      short loc_18C150C
	mov     esi, [eax+0Ch]
	mov     edi, [eax+10h]
	mov     edx, [eax+14h]
	mov     eax, [eax+8]
	mov     ebx, edi
	and     ebx, esi
	mov     ecx, esi
	not     ecx
	and     ecx, edx
	or      ecx, ebx
	add     ecx, [ebp+var_48]
	mov     ebx, esi
	lea     ecx, [ecx+eax-28955B88h]
	mov     eax, ecx
	shr     eax, 19h
	shl     ecx, 7
	or      eax, ecx
	add     eax, esi
	and     ebx, eax
	mov     ecx, eax
	not     ecx
	and     ecx, edi
	or      ecx, ebx
	add     ecx, [ebp+var_44]
	lea     edx, [ecx+edx-173848AAh]
	mov     ecx, edx
	shl     edx, 0Ch
	shr     ecx, 14h
	or      ecx, edx
	add     ecx, eax
	mov     edx, ecx
	not     edx
	and     edx, esi
	mov     ebx, ecx
	and     ebx, eax
	or      edx, ebx
	add     edx, [ebp+var_40]
	mov     ebx, ecx
	lea     edi, [edx+edi+242070DBh]
	mov     edx, edi
	shl     edi, 11h
	shr     edx, 0Fh
	or      edx, edi
	add     edx, ecx
	mov     edi, edx
	not     edi
	and     edi, eax
	and     ebx, edx
	or      edi, ebx
	add     edi, [ebp+var_3C]
	lea     esi, [edi+esi-3E423112h]
	mov     edi, esi
	shl     edi, 16h
	shr     esi, 0Ah
	or      edi, esi
	add     edi, edx
	mov     [ebp+var_4], edi
	not     edi
	and     edi, ecx
	mov     esi, edx
	and     esi, [ebp+var_4]
	or      edi, esi
	add     edi, [ebp+var_38]
	lea     eax, [edi+eax-0A83F051h]
	mov     edi, [ebp+var_4]
	mov     esi, eax
	shl     eax, 7
	shr     esi, 19h
	or      esi, eax
	add     esi, edi
	mov     eax, esi
	not     eax
	and     eax, edx
	mov     ebx, edi
	and     ebx, esi
	or      eax, ebx
	add     eax, [ebp+var_34]
	lea     ecx, [eax+ecx+4787C62Ah]
	mov     eax, ecx
	shr     eax, 14h
	shl     ecx, 0Ch
	or      eax, ecx
	add     eax, esi
	mov     ecx, eax
	not     ecx
	and     ecx, edi
	mov     ebx, eax
	and     ebx, esi
	or      ecx, ebx
	add     ecx, [ebp+var_30]
	mov     ebx, eax
	lea     edx, [ecx+edx-57CFB9EDh]
	mov     ecx, edx
	shl     edx, 11h
	shr     ecx, 0Fh
	or      ecx, edx
	add     ecx, eax
	and     ebx, ecx
	mov     edx, ecx
	not     edx
	and     edx, esi
	or      edx, ebx
	add     edx, [ebp+var_2C]
	lea     edx, [edx+edi-2B96AFFh]
	mov     edi, edx
	shl     edi, 16h
	shr     edx, 0Ah
	or      edi, edx
	add     edi, ecx
	mov     [ebp+var_4], edi
	mov     edx, ecx
	and     edx, [ebp+var_4]
	not     edi
	and     edi, eax
	or      edi, edx
	add     edi, [ebp+var_28]
	lea     esi, [edi+esi+698098D8h]
	mov     edi, [ebp+var_4]
	mov     edx, esi
	shl     esi, 7
	shr     edx, 19h
	or      edx, esi
	add     edx, edi
	mov     ebx, edi
	and     ebx, edx
	mov     esi, edx
	not     esi
	and     esi, ecx
	or      esi, ebx
	add     esi, [ebp+var_24]
	lea     eax, [esi+eax-74BB0851h]
	mov     esi, eax
	shl     eax, 0Ch
	shr     esi, 14h
	or      esi, eax
	add     esi, edx
	mov     eax, esi
	not     eax
	and     eax, edi
	mov     ebx, esi
	and     ebx, edx
	or      eax, ebx
	add     eax, [ebp+var_20]
	mov     ebx, esi
	lea     ecx, [eax+ecx-0A44Fh]
	mov     eax, ecx
	shl     ecx, 11h
	shr     eax, 0Fh
	or      eax, ecx
	add     eax, esi
	mov     ecx, eax
	not     ecx
	and     ecx, edx
	and     ebx, eax
	or      ecx, ebx
	add     ecx, [ebp+var_1C]
	lea     ecx, [ecx+edi-76A32842h]
	mov     edi, ecx
	shl     edi, 16h
	shr     ecx, 0Ah
	or      edi, ecx
	add     edi, eax
	mov     [ebp+var_4], edi
	not     edi
	and     edi, esi
	mov     ecx, eax
	and     ecx, [ebp+var_4]
	or      edi, ecx
	add     edi, [ebp+var_18]
	lea     edx, [edi+edx+6B901122h]
	mov     edi, [ebp+var_4]
	mov     ecx, edx
	shl     edx, 7
	shr     ecx, 19h
	or      ecx, edx
	add     ecx, [ebp+var_4]
	and     edi, ecx
	mov     edx, ecx
	not     edx
	and     edx, eax
	or      edx, edi
	add     edx, [ebp+var_14]
	lea     esi, [edx+esi-2678E6Dh]
	mov     edx, esi
	shl     esi, 0Ch
	shr     edx, 14h
	or      edx, esi
	add     edx, ecx
	mov     edi, edx
	not     edi
	mov     esi, edi
	and     esi, [ebp+var_4]
	mov     ebx, edx
	and     ebx, ecx
	or      esi, ebx
	add     esi, [ebp+var_10]
	mov     ebx, edx
	lea     eax, [esi+eax-5986BC72h]
	mov     esi, eax
	shl     eax, 11h
	shr     esi, 0Fh
	or      esi, eax
	add     esi, edx
	and     ebx, esi
	mov     [ebp+var_8], esi
	not     [ebp+var_8]
	mov     eax, [ebp+var_8]
	and     eax, ecx
	or      eax, ebx
	add     eax, [ebp+var_C]
	mov     ebx, [ebp+var_4]
	lea     ebx, [eax+ebx+49B40821h]
	mov     eax, ebx
	shr     ebx, 0Ah
	shl     eax, 16h
	or      eax, ebx
	mov     ebx, edx
	add     eax, esi
	and     edi, esi
	and     ebx, eax
	or      edi, ebx
	add     edi, [ebp+var_44]
	mov     ebx, esi
	lea     ecx, [edi+ecx-9E1DA9Eh]
	mov     edi, ecx
	shl     ecx, 5
	shr     edi, 1Bh
	or      edi, ecx
	mov     ecx, [ebp+var_8]
	and     ecx, eax
	add     edi, eax
	and     ebx, edi
	or      ecx, ebx
	add     ecx, [ebp+var_30]
	lea     edx, [ecx+edx-3FBF4CC0h]
	mov     ecx, edx
	shl     edx, 9
	shr     ecx, 17h
	or      ecx, edx
	mov     edx, eax
	not     edx
	add     ecx, edi
	and     edx, edi
	mov     ebx, ecx
	and     ebx, eax
	or      edx, ebx
	add     edx, [ebp+var_1C]
	lea     esi, [edx+esi+265E5A51h]
	mov     edx, esi
	shl     esi, 0Eh
	shr     edx, 12h
	or      edx, esi
	add     edx, ecx
	mov     esi, edi
	not     esi
	and     esi, ecx
	mov     ebx, edx
	and     ebx, edi
	or      esi, ebx
	add     esi, [ebp+var_48]
	mov     ebx, ecx
	lea     eax, [esi+eax-16493856h]
	mov     esi, eax
	shr     eax, 0Ch
	shl     esi, 14h
	or      esi, eax
	add     esi, edx
	mov     eax, ecx
	and     ebx, esi
	not     eax
	and     eax, edx
	or      eax, ebx
	add     eax, [ebp+var_34]
	mov     ebx, edx
	lea     edi, [eax+edi-29D0EFA3h]
	mov     eax, edi
	shl     edi, 5
	shr     eax, 1Bh
	or      eax, edi
	mov     edi, edx
	add     eax, esi
	not     edi
	and     edi, esi
	and     ebx, eax
	or      edi, ebx
	add     edi, [ebp+var_20]
	lea     ecx, [edi+ecx+2441453h]
	mov     edi, ecx
	shl     ecx, 9
	shr     edi, 17h
	or      edi, ecx
	mov     ecx, esi
	add     edi, eax
	not     ecx
	and     ecx, eax
	mov     ebx, edi
	and     ebx, esi
	or      ecx, ebx
	add     ecx, [ebp+var_C]
	lea     edx, [ecx+edx-275E197Fh]
	mov     ecx, edx
	shl     edx, 0Eh
	shr     ecx, 12h
	or      ecx, edx
	add     ecx, edi
	mov     edx, eax
	not     edx
	and     edx, edi
	mov     ebx, ecx
	and     ebx, eax
	or      edx, ebx
	add     edx, [ebp+var_38]
	mov     ebx, edi
	lea     esi, [edx+esi-182C0438h]
	mov     edx, esi
	shr     esi, 0Ch
	shl     edx, 14h
	or      edx, esi
	mov     esi, edi
	add     edx, ecx
	not     esi
	and     esi, ecx
	and     ebx, edx
	or      esi, ebx
	add     esi, [ebp+var_24]
	mov     ebx, ecx
	lea     eax, [esi+eax+21E1CDE6h]
	mov     esi, eax
	shl     eax, 5
	shr     esi, 1Bh
	or      esi, eax
	add     esi, edx
	and     ebx, esi
	mov     eax, ecx
	not     eax
	and     eax, edx
	or      eax, ebx
	add     eax, [ebp+var_10]
	lea     edi, [eax+edi-3CC8F82Ah]
	mov     eax, edi
	shl     edi, 9
	shr     eax, 17h
	or      eax, edi
	add     eax, esi
	mov     edi, edx
	mov     ebx, eax
	and     ebx, edx
	not     edi
	and     edi, esi
	or      edi, ebx
	add     edi, [ebp+var_3C]
	lea     ecx, [edi+ecx-0B2AF279h]
	mov     edi, ecx
	shl     ecx, 0Eh
	shr     edi, 12h
	or      edi, ecx
	add     edi, eax
	mov     ecx, esi
	not     ecx
	and     ecx, eax
	mov     ebx, edi
	and     ebx, esi
	or      ecx, ebx
	add     ecx, [ebp+var_28]
	mov     ebx, eax
	lea     edx, [ecx+edx+455A14EDh]
	mov     ecx, edx
	shr     edx, 0Ch
	shl     ecx, 14h
	or      ecx, edx
	mov     edx, eax
	add     ecx, edi
	not     edx
	and     edx, edi
	and     ebx, ecx
	or      edx, ebx
	add     edx, [ebp+var_14]
	mov     ebx, edi
	lea     esi, [edx+esi-561C16FBh]
	mov     edx, esi
	shl     esi, 5
	shr     edx, 1Bh
	or      edx, esi
	add     edx, ecx
	mov     esi, edi
	not     esi
	and     esi, ecx
	and     ebx, edx
	or      esi, ebx
	add     esi, [ebp+var_40]
	lea     eax, [esi+eax-3105C08h]
	mov     esi, eax
	shl     eax, 9
	shr     esi, 17h
	or      esi, eax
	add     esi, edx
	mov     eax, ecx
	not     eax
	and     eax, edx
	mov     ebx, esi
	and     ebx, ecx
	or      eax, ebx
	add     eax, [ebp+var_2C]
	lea     edi, [eax+edi+676F02D9h]
	mov     eax, edi
	shl     edi, 0Eh
	shr     eax, 12h
	or      eax, edi
	add     eax, esi
	mov     edi, edx
	not     edi
	and     edi, esi
	mov     ebx, eax
	and     ebx, edx
	or      edi, ebx
	add     edi, [ebp+var_18]
	lea     ecx, [edi+ecx-72D5B376h]
	mov     edi, ecx
	shr     ecx, 0Ch
	shl     edi, 14h
	or      edi, ecx
	add     edi, eax
	mov     ecx, esi
	xor     ecx, eax
	xor     ecx, edi
	add     ecx, [ebp+var_34]
	lea     edx, [ecx+edx-5C6BEh]
	mov     ecx, edx
	shl     edx, 4
	shr     ecx, 1Ch
	or      ecx, edx
	add     ecx, edi
	mov     edx, eax
	xor     edx, edi
	xor     edx, ecx
	add     edx, [ebp+var_28]
	lea     esi, [edx+esi-788E097Fh]
	mov     ebx, esi
	shl     esi, 0Bh
	shr     ebx, 15h
	or      ebx, esi
	add     ebx, ecx
	mov     edx, ebx
	xor     edx, edi
	xor     edx, ecx
	add     edx, [ebp+var_1C]
	lea     eax, [edx+eax+6D9D6122h]
	mov     esi, eax
	shl     eax, 10h
	shr     esi, 10h
	or      esi, eax
	add     esi, ebx
	mov     edx, ebx
	xor     edx, esi
	mov     eax, edx
	xor     eax, ecx
	add     eax, [ebp+var_10]
	lea     edi, [eax+edi-21AC7F4h]
	mov     eax, edi
	shr     edi, 9
	shl     eax, 17h
	or      eax, edi
	add     eax, esi
	xor     edx, eax
	add     edx, [ebp+var_44]
	lea     ecx, [edx+ecx-5B4115BCh]
	mov     edx, ecx
	shl     ecx, 4
	shr     edx, 1Ch
	or      edx, ecx
	add     edx, eax
	mov     ecx, esi
	xor     ecx, eax
	xor     ecx, edx
	add     ecx, [ebp+var_38]
	lea     ebx, [ecx+ebx+4BDECFA9h]
	mov     edi, ebx
	shr     edi, 15h
	shl     ebx, 0Bh
	or      edi, ebx
	add     edi, edx
	mov     ecx, edi
	xor     ecx, eax
	xor     ecx, edx
	add     ecx, [ebp+var_2C]
	lea     ecx, [ecx+esi-944B4A0h]
	mov     esi, ecx
	shl     ecx, 10h
	shr     esi, 10h
	or      esi, ecx
	add     esi, edi
	mov     ecx, edi
	xor     ecx, esi
	mov     ebx, ecx
	xor     ebx, edx
	add     ebx, [ebp+var_20]
	lea     ebx, [ebx+eax-41404390h]
	mov     eax, ebx
	shl     eax, 17h
	shr     ebx, 9
	or      eax, ebx
	add     eax, esi
	xor     ecx, eax
	add     ecx, [ebp+var_14]
	lea     edx, [ecx+edx+289B7EC6h]
	mov     ecx, edx
	shl     edx, 4
	shr     ecx, 1Ch
	or      ecx, edx
	add     ecx, eax
	mov     edx, esi
	xor     edx, eax
	xor     edx, ecx
	add     edx, [ebp+var_48]
	lea     edi, [edx+edi-155ED806h]
	mov     edx, edi
	shl     edi, 0Bh
	shr     edx, 15h
	or      edx, edi
	add     edx, ecx
	mov     edi, edx
	xor     edi, eax
	xor     edi, ecx
	add     edi, [ebp+var_3C]
	lea     edi, [edi+esi-2B10CF7Bh]
	mov     esi, edi
	shl     edi, 10h
	shr     esi, 10h
	or      esi, edi
	add     esi, edx
	mov     edi, edx
	xor     edi, esi
	mov     ebx, edi
	xor     ebx, ecx
	add     ebx, [ebp+var_30]
	lea     ebx, [ebx+eax+4881D05h]
	mov     eax, ebx
	shl     eax, 17h
	shr     ebx, 9
	or      eax, ebx
	add     eax, esi
	xor     edi, eax
	add     edi, [ebp+var_24]
	lea     edi, [edi+ecx-262B2FC7h]
	mov     ecx, edi
	shl     edi, 4
	shr     ecx, 1Ch
	or      ecx, edi
	mov     edi, esi
	xor     edi, eax
	add     ecx, eax
	xor     edi, ecx
	add     edi, [ebp+var_18]
	lea     edi, [edi+edx-1924661Bh]
	mov     edx, edi
	shl     edi, 0Bh
	shr     edx, 15h
	or      edx, edi
	add     edx, ecx
	mov     edi, edx
	xor     edi, eax
	xor     edi, ecx
	add     edi, [ebp+var_C]
	lea     edi, [edi+esi+1FA27CF8h]
	mov     esi, edi
	shr     esi, 10h
	shl     edi, 10h
	or      esi, edi
	add     esi, edx
	mov     edi, edx
	xor     edi, esi
	xor     edi, ecx
	add     edi, [ebp+var_40]
	lea     eax, [edi+eax-3B53A99Bh]
	mov     edi, eax
	shl     edi, 17h
	shr     eax, 9
	or      edi, eax
	add     edi, esi
	mov     eax, edx
	not     eax
	or      eax, edi
	xor     eax, esi
	add     eax, [ebp+var_48]
	lea     ecx, [eax+ecx-0BD6DDBCh]
	mov     eax, ecx
	shl     ecx, 6
	shr     eax, 1Ah
	or      eax, ecx
	add     eax, edi
	mov     ecx, esi
	not     ecx
	or      ecx, eax
	xor     ecx, edi
	add     ecx, [ebp+var_2C]
	lea     edx, [ecx+edx+432AFF97h]
	mov     ecx, edx
	shl     edx, 0Ah
	shr     ecx, 16h
	or      ecx, edx
	mov     edx, edi
	add     ecx, eax
	not     edx
	or      edx, ecx
	xor     edx, eax
	add     edx, [ebp+var_10]
	lea     esi, [edx+esi-546BDC59h]
	mov     edx, esi
	shl     esi, 0Fh
	shr     edx, 11h
	or      edx, esi
	add     edx, ecx
	mov     esi, eax
	not     esi
	or      esi, edx
	xor     esi, ecx
	add     esi, [ebp+var_34]
	lea     edi, [esi+edi-36C5FC7h]
	mov     esi, edi
	shr     edi, 0Bh
	shl     esi, 15h
	or      esi, edi
	mov     edi, ecx
	not     edi
	add     esi, edx
	or      edi, esi
	xor     edi, edx
	add     edi, [ebp+var_18]
	lea     edi, [edi+eax+655B59C3h]
	mov     eax, edi
	shl     edi, 6
	shr     eax, 1Ah
	or      eax, edi
	add     eax, esi
	mov     edi, edx
	not     edi
	or      edi, eax
	xor     edi, esi
	add     edi, [ebp+var_3C]
	lea     edi, [edi+ecx-70F3336Eh]
	mov     ecx, edi
	shl     edi, 0Ah
	shr     ecx, 16h
	or      ecx, edi
	mov     edi, esi
	add     ecx, eax
	not     edi
	or      edi, ecx
	xor     edi, eax
	add     edi, [ebp+var_20]
	lea     edi, [edi+edx-100B83h]
	mov     edx, edi
	shl     edi, 0Fh
	shr     edx, 11h
	or      edx, edi
	add     edx, ecx
	mov     edi, eax
	not     edi
	or      edi, edx
	xor     edi, ecx
	add     edi, [ebp+var_44]
	lea     edi, [edi+esi-7A7BA22Fh]
	mov     esi, edi
	shr     edi, 0Bh
	shl     esi, 15h
	or      esi, edi
	add     esi, edx
	mov     edi, ecx
	not     edi
	or      edi, esi
	xor     edi, edx
	add     edi, [ebp+var_28]
	lea     edi, [edi+eax+6FA87E4Fh]
	mov     eax, edi
	shl     edi, 6
	shr     eax, 1Ah
	or      eax, edi
	add     eax, esi
	mov     edi, edx
	not     edi
	or      edi, eax
	xor     edi, esi
	add     edi, [ebp+var_C]
	lea     edi, [edi+ecx-1D31920h]
	mov     ecx, edi
	shl     edi, 0Ah
	shr     ecx, 16h
	or      ecx, edi
	add     ecx, eax
	mov     edi, esi
	not     edi
	or      edi, ecx
	xor     edi, eax
	add     edi, [ebp+var_30]
	lea     edi, [edi+edx-5CFEBCECh]
	mov     edx, edi
	shl     edi, 0Fh
	shr     edx, 11h
	or      edx, edi
	mov     edi, eax
	not     edi
	add     edx, ecx
	or      edi, edx
	xor     edi, ecx
	add     edi, [ebp+var_14]
	lea     edi, [edi+esi+4E0811A1h]
	mov     esi, edi
	shr     edi, 0Bh
	shl     esi, 15h
	or      esi, edi
	add     esi, edx
	mov     edi, ecx
	not     edi
	or      edi, esi
	xor     edi, edx
	add     edi, [ebp+var_38]
	lea     eax, [edi+eax-8AC817Eh]
	mov     edi, eax
	shl     eax, 6
	shr     edi, 1Ah
	or      edi, eax
	mov     eax, edx
	not     eax
	add     edi, esi
	or      eax, edi
	xor     eax, esi
	add     eax, [ebp+var_1C]
	lea     eax, [eax+ecx-42C50DCBh]
	mov     ecx, eax
	shl     eax, 0Ah
	shr     ecx, 16h
	or      ecx, eax
	mov     eax, esi
	not     eax
	add     ecx, edi
	or      eax, ecx
	xor     eax, edi
	add     eax, [ebp+var_40]
	lea     edx, [eax+edx+2AD7D2BBh]
	mov     ebx, edx
	shl     edx, 0Fh
	shr     ebx, 11h
	or      ebx, edx
	add     ebx, ecx
	mov     eax, edi
	not     eax
	or      eax, ebx
	xor     eax, ecx
	add     eax, [ebp+var_24]
	lea     esi, [eax+esi-14792C6Fh]
	mov     eax, [ebp+arg_0]
	mov     edx, [eax+8]
	add     edx, edi
	mov     [eax+8], edx
	mov     edx, esi
	shl     edx, 15h
	shr     esi, 0Bh
	or      edx, esi
	add     edx, [eax+0Ch]
	pop     edi
	add     edx, ebx
	mov     [eax+0Ch], edx
	mov     edx, [eax+10h]
	add     edx, ebx
	mov     [eax+10h], edx
	mov     edx, [eax+14h]
	add     edx, ecx
	pop     esi
	mov     [eax+14h], edx
	pop     ebx
	leave
	retn    4
	MD5_Process endp

end