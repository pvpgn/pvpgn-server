;
;Warden Stuff
;

D2Warden_ThreadHandle dd 0
D2Warden_ThreadReturnVal dd 0
D2Warden_StartAddress dd 0
D2Warden_0X66Handler	dd 0
D2Warden_0X68Handler	dd 0
aD2Warden db 'd2warden.dll',0
aD2Warden_ThreadStr db 'd2warden_thread',0
aD2Warden_0X66Handler	db 'd2warden_0X66Handler',0
aD2Warden_0X68Handler	db 'd2warden_0X68Handler',0
D2Warden_Handler dd 0

InitWardenThread proc
	pushad
	
  mov eax,D2Warden_ThreadHandle
  test eax,eax
  jnz over	;已经创建了warden 线程
	mov eax,EnableWarden
	test eax,eax
	jz over
	
	push offset aD2Warden	; d2warden.dll
	mov eax,LoadLibraryA
	mov eax,[eax]
	call eax	; LoadLibraryA
	mov D2Warden_Handler,eax
	test eax,eax
	jz over
	
	push offset aD2Warden_ThreadStr
	push eax
	mov eax,GetProcAddr
	mov eax,[eax]
	call eax ; GetProcAddr
	mov D2Warden_StartAddress,eax
	test eax,eax
	jnz cont1

fail_init:
	mov eax,FreeLibrary
	mov eax,[eax]
	push D2Warden_Handler
	call eax ; FreeLibrary
	
	; Disable Warden if init fail
	xor eax,eax
	mov EnableWarden,eax
	jmp over

cont1:
	push offset aD2Warden_0X66Handler
	push D2Warden_Handler
	mov eax,GetProcAddr
	mov eax,[eax]
	call eax ; GetProcAddr
	mov D2Warden_0X66Handler,eax
	test eax,eax
	jnz cont2
	jmp fail_init
	
cont2:
	push offset aD2Warden_0X68Handler
	push D2Warden_Handler
	mov eax,GetProcAddr
	mov eax,[eax]
	call eax ; GetProcAddr
	mov D2Warden_0X68Handler,eax
	test eax,eax
	jnz cont3
	jmp fail_init

cont3:
	push 0	; lpThreadId
	push 0	; dwCreationFlags
	push 0
	push D2Warden_StartAddress ; lpStartAddress
	push 0	; dwStackSize
	push 0	; lpThreadAttributes
	mov	eax,CreateThread
	mov eax,[eax]
	call eax	; CreateThread
	
	test eax,eax
	jnz	over
fail_over:	
	mov EnableWarden,eax	
	
over:
  mov D2Warden_ThreadHandle,eax
	popad
	retn
InitWardenThread endp


;d2game.dll 6FC4BE20
;ecx
;edx
;arg_0
;arg_4
;
;ret eax=3 非法报文
MyPacket0X66Handler proc 
	mov			eax,EnableWarden
	test		eax,eax
	jnz			SendInfoToWarden
	; 奇怪！Warden并没有Enable，但是客户端却回送了0x66报文？！
	retn 8
SendInfoToWarden:
	; ecx=ptGame
	; edx=ptPlayer
	; arg_0=ptPacket
	; arg_4=len
	jmp D2Warden_0X66Handler
MyPacket0X66Handler	endp

; Client Logon into game
MyPacket0X68Handler proc 
	xor			eax,eax
	mov     ax, [edx+5]
	cmp     ax, 1
	jb      loc_68003AD5
	cmp     ax, 400h
	ja      loc_68003AD5
	
	push		eax
	movzx		eax,byte ptr[edx+7]
	cmp			eax,5
	jz			check_asn_dru
	cmp			eax,6
	jz			check_asn_dru
	pop			eax
	jmp			check_over
check_asn_dru:
	pop			eax

check_dru:
	; 检查D2C是否是DRU或者ASN角色
	pushad
	push		eax
	call		GetGameInfo
	add			esp,4
	test		eax,eax
	jz			fail_get_game
	mov			eax,[eax+4]
	test		eax,eax
	jz			fail_get_game
	mov			eax,[eax+ExtendGameInfoStruct.LoD_Game]
	test		eax,eax
	jnz			LOD_Game
	; DRU or ASN in D2C, invalid!
	popad
	jmp			loc_68003AD5
LOD_Game:
	popad
	jmp			check_over
fail_get_game:	
	popad

check_over:
	retn 4
loc_68003AD5:
	mov     eax, 3
	retn 4
MyPacket0X68Handler	endp

MyPacket0X68Handler_Post proc 
	;eax=ptClient
	;ecx=ptPacket
	push		eax
	mov			eax,EnableWarden
	test		eax,eax
	jnz			SendInfoToWarden
	pop			eax
	ret
SendInfoToWarden:
	pop eax	; restore the return val
	pushad
	; ecx=ClientID
	; edx=ptPacket
	call D2Warden_0X68Handler
	popad
	xor     eax, eax
	ret
fail_over:
	pop edx
	pop ecx
	popad
loc_68003AD5:
	mov     eax, 3
	ret
MyPacket0X68Handler_Post	endp

