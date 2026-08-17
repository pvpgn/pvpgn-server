//////////////////////////////////////////////////////////////////////
//
// marsgod - <marsgod@263.sina.com>
//
//////////////////////////////////////////////////////////////////////
/*
0x00
0x00 is a request sent from the server asking if you have the current warden module. It is in the following form: 

[1 byte] code (0) 
[16 bytes] name of the current module 
[16 bytes] decryption key for the current module 
[4 bytes] length of the current module 
A response of 0 will tell Battle.net to send you the current module, and you'll receive a slew of 0x01 packets. 1 will tell Battle.net you already have the module, and you'll receive a 0x02 packet. 

Example
Server 
00 a2 d4 d6 4c 46 8e 56 4f 42 c6 s4 68 e4 5d 6a 46 5f 46 b4 5c 24 d5 46 e4 56 a6 4d 75 2d 21 f8 79 05 0b 00 00 
Client 
00 
0x01
0x01 packets have the form: 

[1 byte] code (0x01) 
[2 bytes] length (without the 3-byte header) 
[array] data 
You will receive many 0x01 packets, until the total length of "data" received is equal to the length sent in packet 0x00. 

After the packet is received, it's validated (see Warden Modules). If it was received without error, a response of 1 (encrypted) is sent. If there's an error, 0 (encrypted) is sent. 

0x02
This packet is a request to validate the running game to verify it's legit. 

I haven't reversed it yet, and don't plan to in the near future. 

0x03
This packet is used to initialize functions to read information from the MPQ files. 

0x03 packets have the form: 

[1 byte] code (0x03) 
[2 bytes] length (without the 7-byte header) 
[4 bytes] checksum of the packet data (the checksum does not include the header) 
[1 byte] Unknown flag (this has to be 1 otherwise Warden will exit the initialize function) 
[1 byte] Unknown flag (usually 0) 
[1 byte] Unknown flag (usually 1) 
[nullstring] Library Name 
[4 bytes] function offset #1 
[4 bytes] function offset #2 
[4 bytes] function offset #3 
[4 bytes] function offset #4 
Packet Checksums
Warden uses a modified version of the SHA1 algo and Xor's the result of it. 

Warden_SHA1 can be found here: SHA1_in_C 

To detect RedVex like proxy hack, warden should check the BNClient.dll (6FF3D8D4) for a BNETIP IP address.
for example, the BNETIP=127.0.0.1, this should known as a Redvex hack.

6FF3D230: BNETIP string
6FF3D330: 域名解释后的地址：127.0.0.1
dword_6FF3D8D4 : inet_addr结果"127.0.0.1;"
*/

#define MAX66LEN 1024
#define MAX_WARDEN_CMD_RETRY 3

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "global.h"
#include "MD5.h"
#include "rc4.h"
#include "d2ptrs.h"
#include "d2callstub.h"

extern "C" void  __fastcall HashGameSeed(unsigned char *pt_0XAE_RC4_KEY, unsigned char *pt_0X66_RC4_KEY, unsigned char * TheGameSeed, unsigned int TheLength);
extern "C" void  __fastcall Double_MD5(DWORD *Mod_Length, DWORD unk, unsigned char *ptResult);

extern "C" DWORD  __fastcall d2warden_0X66Handler(DWORD ptGame, DWORD ptPlayer, unsigned char *ptPacket, DWORD PacketLen);
extern "C" DWORD  __fastcall d2warden_0X68Handler(unsigned char *ClientID_Packet);//,unsigned char *ptClient);

#define MAX_CHAT_MSG_BUF 10
void SendMsgToClient(DWORD ClientID,unsigned char *Msg);

#define UPDATE_WARDEN_RIGHTNOW(x) {							ptCurrentClient->WardenStatus = x; \
							ptCurrentClient->NextCheckTime = CurrentTick+100;	 \
							ptCurrentClient->NextTimeout = CurrentTick+15000; \
						}

#define UPDATE_WARDEN(x,y,z) {							ptCurrentClient->WardenStatus = x; \
							ptCurrentClient->NextCheckTime = y;	 \
							ptCurrentClient->NextTimeout = z; \
						}

int WardenHackCheckNumbers;
int WardenCheckInterv;
int ChatMsgUpdateInterv;
BYTE EnablePlayerMouseCheck;
BYTE EnableMMBotCheck;
BYTE SupportD2Version[10];

struct WardenHackCheckStruct
{
	unsigned char D2Version;		// 客户端版本
	unsigned char HackName[50];	// Hack的名称
	char Msg[256];
	unsigned char Log;		// 是否记录Hack,0=not log, 1=log, bit0=HackLog bit1=UnknownLog bit2=TimeoutLog
	unsigned char Action;			// 0=skip, 1=kick , bit0=HackAction  bit1=UnknownAction bit2=TimeoutAction
	unsigned char Debug;	// 记录下发送的AE报文以及收到的66报文
	int PacketAELen;	// 发送的PacketAE长度
	unsigned char PacketAE[512];	// 发送的PacketAE报文
	DWORD HackCHKSUM;	// 预期接收到的Hack Packet66报文
	DWORD CorrectCHKSUM;	// 预期接收到的正确Packet66报文
	int HackChatLen;
	int MsgOffset;
	unsigned char HackChat[65];
	int CheckTimes;
} *WardenHackCheckArray;

struct Client_Data_Struct
{
	DWORD ClientID;
	DWORD ErrorCount;
	unsigned char SessionKey[16];
	char CharName[20];
	char AccountName[20];
	DWORD ClientLogonTime;
	unsigned char D2Version;
	DWORD WardenStatus;			// 0-初始化	1-需要重新下载MOD 2-正在下载MOD 3-下载完毕（已经拥有MOD），需要发送01报文检查 4-01报文已经发送，等待66报文回应
	unsigned char CanRecv66Packet;
	int NextHackCheck;
	DWORD NextCheckTime;
	DWORD LastCheckTime;
	DWORD NextTimeout;
	DWORD Packet66_ReceiveTime;
	DWORD Packet66_PacketLen;
	unsigned char ThePacket[MAX66LEN];
	DWORD MOD_Position;		// 当前传送到该MOD的第几个字节	0-还没有传送  FFFFFFFF-已经传送完毕
	unsigned char RC4_KEY_0X66[258];
	unsigned char RC4_KEY_0XAE[258];
	unsigned char AE_Packet00[40];
	DWORD ServerReadyTime;
	
	unsigned char ErrorResponseReason;
	unsigned char TimeoutReason;
	int CheckTimes;
	DWORD LastChatUpdateTime;
	unsigned char NewMsg;
	int ChatMsgIndex;
	int CurrentCheckChatID;
	char LatestNumberOfMsgInBuffer;
	char NumberOfMsgInBuffer;
	DWORD NextMsgAddress;
	unsigned char ChatMsg[MAX_CHAT_MSG_BUF][65];
	
	/*
		如果服务器上玩家在Run或者Walk，但是客户端没有Emode动作，表示有问题！
		如果服务器上玩家正在Trade NPC，但是客户端没有打开TradeWindow，表示有问题！
	*/
	BYTE EnablePlayerMouseCheck;
	DWORD PlayerPtr;
	DWORD PlayerPathPtr;

	// 鼠标以及玩家的位置
//	DWORD PlayerXPosition;
//	DWORD PlayerYPosition;
//	DWORD PlayerXPosition_Modify;
	DWORD MouseXPosition;
	DWORD MouseYPosition;
	
	BYTE ClientTradeUIVar;
	
	BYTE MissMatchEModeCounter;
	
	BYTE PlayerZeroTargetCounter;
//	DWORD PlayerTargetX;
//	DWORD PlayerTargetY;
	
	DWORD ServerPlayerPtr;
	
	DWORD pMissilesTxt;
	
	struct Client_Data_Struct *pNext;

};

struct D2Warden_Parameter_Struct
{
	CRITICAL_SECTION WardenLock;	// 访问Client Logon队列的互斥锁

	struct Client_Data_Struct *Warden_Client_Data;
} D2Warden_Parameter;

bool Warden_Enable;
char Warden_MOD[256];
int Warden_Check_Interv;

unsigned char RC4_Key[]="WardenBy_Marsgod"; //3ea42f5ac80f0d2deb35d99b4e9a780b98ff
unsigned char Maiev[] = "MAIEV.MOD";
//unsigned char RC4_KeyEx[256];

void Warden_Init(void);

DWORD MOD_Length;
unsigned char *MOD_Enrypted;

unsigned char AE_Packet00[40];

#define WARDEN_START	0
#define WARDEN_START1	1
#define WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT	2
#define WARDEN_DOWNLOAD_MOD	3
#define WARDEN_CHECK_HACKS 4
#define WARDEN_WAIT_FOR_CHECK_RESULT	5
#define WARDEN_ERROR_RESPONSE	6
#define	WARDEN_TIMEOUT	7
#define WARDEN_WAITING_DOWNLOAD_END 8
#define WARDEN_KICK_PLAYER 9
#define WARDEN_UPDATE_CHAT_MSG 10
#define WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER 11
#define WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER2 12
#define WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER3 13
#define WARDEN_GET_CLIENT_STATUS 14
#define WARDEN_WAIT_FOR_CLIENT_STATUS0 15
#define WARDEN_WAIT_FOR_CLIENT_STATUS1 16
#define WARDEN_WAIT_FOR_CLIENT_STATUS2 17
#define WARDEN_UNSUPPORT_D2_VERSION	18

#define NEVER	0xFFFFFFFF

#define MAXERRORCOUNT 10

#define LOG_HACK 0
#define LOG_INFO 1
#define LOG_DEBUG 2

unsigned char log_level;

// WardenCMD2: 02 len string len string
// WardenCMD2: 02 00 subcmd0 subcmd1 (DWORD)memory_location len xx
//
// WardenCMD2: 02 00 ID_Mem_Check(1b) String_Index(01) Offset Len 
// WardenCMD2: 02 00 ID_PAGE_CHECK_A (DWORD)Unknown_Seed (DWORD[5])SHA1 (DWORD)Address (BYTE)Len BYTE=00
// WardenCMD2: 02 00 d8 [ce fd 4f 48] {[d4 96 3A 2E de][c5 c5 17 05 71][e1 24 cf ae f5][92 68 a4 34 0f]}4X5 [b8 fe][20] 这个命令用于检查NtQueryVirtualMemory函数是否被挂钩
// WardenCMD2: 02 00 0E [01]  对报文的第一个字符串所指明的文件，读取其SHA值
// WardenCMD2: 02 00 1b 02 [7b 33 06 00] [07] 
// WardenCMD2: 02 00 d8 [17 a0 c8 36] 4X5 [2a ff][20]
// WardenCMD2: 02 00 0e [03]
// WardenCMD2: 02 00 d8 [15 34 fb 1a] 4X5 [00 00][1f]

// WardenCMD2: 02 00 a2 [b8 08 d3 e1] 4X5 [58 a4 00 00][3c] 使用NtQueryVirtualMemory来穷举所有数据段，当大小>Offset的时候，读取数据14A458

// WardenCMD2: 02 00 51 {[d3 dc 0c ec]MD5} 4X5 枚举系统进程，获得进程的文件名，SHA，与预期的SHA值比较
// WardenCMD2: 02 00 26 16bytes
// WardenCMD2: 02 00 36 [xx] [xx xx xx xx] [xx] 
// WardenCMD2: 02 00 44 [xx]
// WardenCMD2: 02 00 6C {[xx xx xx xx]MD5}
// WardenCMD2: 02 00 87 {[xx xx xx xx]MD5} [xx xx xx xx] [xx] 
// WardenCMD2处理: d8进行判断（51 0E 1B 29 36 44 6C 87 A2 BD D8）


// +30 WardenCMD2: D8 [DWORD seed] [DWORD X 5  SHA][WORD Offset][BYTE LEN] 这个命令用于检查NtQueryVirtualMemory函数是否被挂钩
// +40 WardenCMD2: 0E [BYTE Index]  对Index字符串指明的文件，读取其SHA值
// +4 WardenCMD2: 1B [BYTE Index] [DWORD Offset] [BYTE len]  对Index字符串指明的Module，读取其Offset处长度为len的内容
// +20 WardenCMD2: A2 [DWORD seed] [DWORD X 5  SHA][DWORD Offset][BYTE LEN] 使用NtQueryVirtualMemory来穷举所有数据段，当大小>Offset+len的时候，读取数据14A458，并与预期的SHA比较
// +14 WardenCMD2: 51 [DWORD seed] [DWORD X 5  SHA] 枚举系统进程，获得进程的文件名，转成大写，SHA，与预期的SHA值比较

// +44 WardenCMD2: 29 16bytes
// +4 +c WardenCMD2: 36 [BYTE Index] [DWORD Offset] [BYTE Len]   对Index字符串指明的Module，读取其Offset处长度为4的内容A，然后寻址Base+A+4，将长度为Len的内容返回，主要用于检查Call指令是否被修改调用了其他程序
// ESI WardenCMD2: 44 [BYTE Index] 调用+9DC处的函数？！需要通过WardenCMD3设置？！
// +18 WardenCMD2: 6C [DWORD Seed] [DWORD X 5 SHA] 搜索所有可执行的代码段，获取该执行文件名，转成大写，SHA，与预期的SHA值比较
// +1c WardenCMD2: 87 [DWORD Seed] [DWORD X 5 SHA] [DWORD Offset][BYTE LEN] 使用snapshot来穷举所有代码段，当大小>Offset+len的时候，读取数据14A458，并与预期的SHA比较
// +2c WardenCMD2: BD [BYTE 堆栈上过程嵌套的级数] [BYTE Len]  获取当前堆栈顶部的返回地址，追踪指定级数（嵌套过程调用），然后将对应代码段所处位置的指定长度代码返回
// +34 WardenCMD2: F3 [BYTE*4][DWORD X 5 SHA][WORD][BYTE]

// 1.12a WardenMOD的子命令
// 76 [BYTE*4][DWORD X 5 SHA] 枚举系统进程，获得进程的文件名，转成大写，SHA，与预期的SHA值比较
// 00 [BYTE*4][DWORD X 5 SHA][DWORD][BYTE] 使用snapshot来穷举所有代码段，当大小>Offset+len的时候，读取数据14A458，并与预期的SHA比较
// 77 [BYTE*4][DWORD X 5 SHA][DWORD][BYTE] 使用NtQueryVirtualMemory来穷举所有数据段，当大小>Offset+len的时候，读取数据14A458，并与预期的SHA比较
// ED [BYTE*4][DWORD X 5 SHA] 枚使用snapshot来穷举所有代码段，获得进程的文件名，转成大写，SHA，与预期的SHA值比较
// EC [BYTE Index][DWORD Offset][BYTE Len] 对Index字符串指明的Module，读取其Offset处长度为len的内容
// D9 将[ebx]设置为1？？！！截断命令，D9后面的都是无效的报文
// 9E [BYTE*4][DWORD X 5 SHA][BYTE] 对Index字符串（必须是一个设备名，例如C:），使用QueryDosDevice获得该设备的详细信息，例如\Device\HarddiskVolume1，对此字符串进行SHA，并与预期的SHA比较
// 8B [BYTE*4][DWORD X 5 SHA][BYTE StrIndex1][BYTE StrIndex2][DWORD Offset][BYTE Len] 使用GetProcess获得StrIndex1模块中StrIndex2函数的地址（跳过起始的NOP指令），如果是CALL指令，则检查其指向地址Offset处长度为Len的SHA值。主要用于检测输出函数是否被挂钩
// 8A [BYTE 堆栈上过程嵌套的级数] [BYTE Len] 获取当前堆栈顶部的返回地址，追踪指定级数（嵌套过程调用 stack frame），然后将对应代码段所处位置的指定长度代码返回 01D37910
// 63 [BYTE] [DWORD Offset][BYTE Len] 对Index字符串指明的Module，读取其Offset处长度为4的内容A，然后寻址Base+A+4，将长度为Len的内容返回，主要用于检查Call指令是否被修改调用了其他程序
// 28 获取客户端的TickCount，时间？或者获取[ESI+7E0]函数的返回值
// 15 [16Bytes] 调用Warden函数（空函数）理16字节数据，返回5个字节的结果
// 14 [BYTE Index] 对Index字符串（文件），使用ESI+7DC函数进行处理，并返回结果（成功返回0，失败错误返回1）
// 01 [BYTE Index] 对Index字符串文件，执行SHA操作，貌似必须是mpq文件？！必须先用Warden_CMD3配置加载

unsigned char WardenCMD2[] = { // load d2client.dll and check memory.... response:02 07 00 00 40 50 0f 00 40 ae 06 03 1e 00 
0x02, 0x0c, 0x44, 0x32, 0x43, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x2e, 0x64, 0x6c, 0x6c, 0x00, 0x1b, 	
0x01, 0x45, 0x23, 0x01, 0x00, 0x06, 0x00};

unsigned char GetChatLocationCMD2_0[] = {0x02,0x0c,0x44,0x32,0x43,0x6c,0x69,0x65,0x6e,0x74,0x2e,0x64,0x6c,0x6c,0x00,0xec,0x01,0x30,0xb8,0x11,0x00,0x06,0xd9};
unsigned char GetChatLocationCMD2_1[] = {0x02,0x00,0xec,0x00,0xff,0xff,0xff,0xff,0x26,0xd9 };
unsigned char GetChatLocationCMD2_2[] = {0x02,0x00,0xec,0x00,0xff,0xff,0xff,0xff,0x40,0xd9 };

unsigned char GetPlayerMouseLocationCMD2_V1[] = {0x02,0x0c,0x44,0x32,0x43,0x6c,0x69,0x65,0x6e,0x74,0x2e,0x64,0x6c,0x6c,0x00,0xec,0x01,0x74,0xb6,0x11,0x00,0x08,0xec,0x01,0xE8,0xC3,0x11,0x00,0x04,0xec,0x01,0x14,0xB4,0x11,0x00,0x08,0xd9};
unsigned char Get2DWORDCMD2[] = {0x02,0x00,0xec,0x00,0xff,0xff,0xff,0xff,0x04,0xec,0x00,0xff,0xff,0xff,0xff,0x04,0xd9 };

unsigned char WardenCMD0_local[38];
unsigned char WardenCMD2_local[1024];

unsigned char GetPtrLocationCMD2_0[] = {0x02,0x0C,0x44,0x32,0x43,0x6c,0x69,0x65,0x6e,0x74,0x2e,0x64,0x6c,0x6c,/*0x00,*/0x0c,0x44,0x32,0x43,0x6F,0x6D,0x6D,0x6F,0x6E,0x2E,0x64,0x6c,0x6c,0x00,0xec,0x01,0x30,0xb8,0x11,0x00,0x04,0xec,0x02,0xF4,0x12,0x0A,0x00,0x04,0xd9};

void WardenLoop(void);
void Log(unsigned char log_level_in,char *format,...)
{
	char text[4096];
	
	if (log_level_in > log_level) return;
	
	va_list arguments;
	va_start(arguments, format);
	vsprintf_s(text,sizeof(text),format, arguments);
	va_end(arguments);

	SYSTEMTIME t;
	char timebuf[256];
	GetLocalTime(&t);
	sprintf_s(timebuf,sizeof(timebuf),"[%04d-%02d-%02d %02d:%02d:%02d]",t.wYear,t.wMonth,t.wDay,t.wHour, t.wMinute, t.wSecond );

	errno_t err;
	FILE *fp;
	err = fopen_s(&fp,"d2warden.log","a+");
	if (err==0)
	{
		fseek(fp,0,SEEK_END);
		fwrite(timebuf,strlen(timebuf),1,fp);
		fwrite(text,strlen(text),1,fp);
		fclose(fp);
	}
};

int GetHexValue(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}

	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}

	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}

	return -1;
}


unsigned char Convert2HEX(unsigned char *tt)
{
	unsigned char tt0;
	
	tt0 = 0;
	tt0 = GetHexValue(tt[0]) * 16 + GetHexValue(tt[1]);
	return tt0;
}

DWORD Convert2DWORD(char *tt)
{
	DWORD ret=0;
	
	sscanf((const char *)tt,"%08X",&ret);
	return ret;
}

int GetHexSpan(const char* string)
{
	int i = 0;
	for (; string[i] != '\0' && GetHexValue(string[i]) >= 0; i++);
	return i;
}

int ConvertHexStringToBytes(const char* string, unsigned char* bytes, int length)
{

	int span = GetHexSpan(string); 
	int offset = span / 2 + (span & 1) - 1;
	int end = span - 1;

	if (offset >= length)
	{
		offset = length - 1;
		end = length * 2 - 1;
	}

	int count = offset + 1;

	for (int i = end, j = 0; i >= 0 && offset < length; i--, j++)
	{
		int value = GetHexValue(string[i]);

		if ((j & 1) != 0)
		{
			bytes[offset--] |= (value << 4);
		}

		else
		{
			bytes[offset] = value;
		}
	}

	return count;
}

int ConvertBytesToHexString(const unsigned char* bytes, int dataSize, char* string, int stringSize, char delimiter)
{
	int count = 0;

	for (int i = 0; i < dataSize && stringSize - count > 2; i++)
	{
		unsigned char byte = bytes[i];
		
		if (delimiter != 0 && stringSize - count > 3 && i < dataSize - 1)
		{
			count += sprintf(string + count, "%.2x%c", byte, delimiter);
		}

		else
		{
			count += sprintf(string + count, "%.2x", byte);
		}
	}

	return count;
}

int CalculateTextDumpSize(int size)
{
	int rows = size / 16;

	if (size % 16 > 0)
		rows++;

	return 16 * rows * 4 + rows * 3 + 1;
}

int DumpDataAsText(unsigned char* bytesIn, int sizeIn, char* dataOut, int sizeOut)
{
	if (sizeOut < CalculateTextDumpSize(sizeIn))
		return -1;

	// zero out the destination data to be safe
	memset(dataOut, 0, sizeOut);

	int offset = 0;

	for (int i = 0; i < sizeIn; i += 16)
	{
		// print out hexadecimal representation for the current row
		for (int j = 0; j < 16; j++)
		{
			if (i + j < sizeIn)
			{
				sprintf(dataOut + offset, "%.2x ", bytesIn[i + j]);
			}

			else
			{
				strcat(dataOut + offset, "   ");
			}

			offset += 3;
		}

		// seperate hexadecimal representation from ascii representation with a tab
		dataOut[offset++] = '\t';
		// print out ascii representation for the current row
		for (int j = 0; j < 16; j++)
		{
			unsigned char temp = ' ';
			
			if (i + j < sizeIn)
			{
				if (isprint(bytesIn[i + j]))
				{
					temp = bytesIn[i + j];
				}

				else
				{
					temp = '.';
				}
			}
			
			dataOut[offset++] = temp;
		}	
		// insert line break at the end of the row
		dataOut[offset++] = '\n';
	}

	// terminate the output string
	dataOut[offset++] = 0;

	return offset;
}

int removespace(char *str,int len)
{
	int i,j;
	char buf[1024];
	
	j = 0;
	for (i=0;i<len;i++)
	if (str[i] == ' ') continue;
	else
	{
		buf[j] = str[i];
		j++;
	}
	buf[j] = 0;
	if (j>0) strcpy(str,buf);
	return j;
}

void trimspaces(char *str)
{
	char *p1 = str, *p2 = str+strlen(str);
	while (isspace(*p1)) p1++;
	do p2--; while ((p2 >= p1) && isspace(*p2));
	*++p2 = 0;
	memmove(str, p1, (p2+1-p1)*sizeof(char));
}

int MyRand()
{
	return 1+(rand() % 8);
}


void Remove0X66Packet(struct Client_Data_Struct *ptCurrentClient)
{
	if (ptCurrentClient==NULL) return;
	ptCurrentClient->Packet66_ReceiveTime=0;
	ptCurrentClient->CanRecv66Packet = 1;
	ptCurrentClient->Packet66_PacketLen=0;
};

/*
void MY_SendPacket(DWORD unk,DWORD ClientID,unsigned char *ThePacket,DWORD Length)
{
	char *ptClientList;
	char *ptConnection;
	
	ptClientList = (char *)D2GAME_GetClient(ClientID);
	if (ptClientList==NULL) return;
	
	ptConnection = (char *)*(DWORD *)(ptClientList+0x88);
	if (ptConnection==NULL)
	{
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return;
	}
	
	while(ptConnection)
	{
		if (*(DWORD *)(ptConnection)==ClientID) break;
		
		if (IsBadReadPtr(ptConnection+0x4A8,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)ptClientList);
			return;
		}

		ptConnection=(char *)*(DWORD *)(ptConnection+0x4A8);
	}
	
	if (ptConnection==NULL)
	{
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return;
	}

	D2GAME_SendPacket(ptConnection,ThePacket,Length);
	D2GAME_LeaveCriticalSection((DWORD)ptClientList);
	return;
}
*/
/*
	AE xxl xxh 1 packl packh ....
	
	AE F7 01 01 F3 01 ...
*/
DWORD SendPartOfMOD2Client(DWORD ClientID,unsigned char *RC4_KEY,DWORD MOD_Position)
{
	unsigned char MyMod[0x200];
	int ModLen;
	
	if (MOD_Length - 0x1f4 > MOD_Position) ModLen = 0x1f4;
	else
		ModLen = MOD_Length - MOD_Position;
	MyMod[0] = 0xAE;
	MyMod[1] = (unsigned char)((ModLen+3)%256);
	MyMod[2] = (unsigned char)((ModLen+3)>>8);
	MyMod[3] = 1;
	MyMod[4] = (unsigned char)(ModLen%256);
	MyMod[5] = (unsigned char)(ModLen>>8);
	memcpy(&MyMod[6],MOD_Enrypted+MOD_Position,ModLen);
	rc4_crypt(RC4_KEY,&MyMod[3],ModLen+3);
	D2NET_SendPacket(0,ClientID,MyMod,ModLen+6);
	return ModLen+MOD_Position;
};

void SendMsgToClient(DWORD ClientID,char *Msg)
{
	// 26 04 00 02 00 00 00 00 01 00 (Character Name) 00 (FF c8 Message FF c4) 00
	unsigned char buffer[1024]={0x26,0x04,0x00,0x02,0x00,0x00,0x00,0x00,0x01,0x00,'[','a','d','m','i','n','i','s','t','r','a','t','o','r',']',0x00,0xFF,0x63,0x38};

	int str_len = strlen(Msg);
	memcpy(&buffer[29],Msg,str_len);
	buffer[29+str_len] = 0xFF;
	buffer[30+str_len] = 0x63;
	buffer[31+str_len] = 0x34;
	buffer[32+str_len] = 0x00;

	D2NET_SendPacket(0,ClientID,buffer,33+str_len);

}

bool CheckValidClient(DWORD ClientID, DWORD ClientLogonTime)
{
	DWORD tt=true;
	tt = D2NET_GetClient(ClientID);
	if (tt)
		return true;
	return false;
}

int UpdateClientInfo(struct Client_Data_Struct *ptCurrentClient)
{
	char *ptClientList;
	char *pt1,*pt2;
	char *ptConnection;
	DWORD InitStaus,InitStaus2;
	DWORD ptGame,dwUnitType,dwUnitId,ptPlayer;
	DWORD last_recv_pack_time;

	
	ptClientList = (char *)D2GAME_GetClient(ptCurrentClient->ClientID);
	if (ptClientList==NULL) return 0;
	
	ptConnection = (char *)*(DWORD *)(ptClientList+0x88);
	if (ptConnection==NULL)
	{
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return -1;
	}
	
	while(ptConnection)
	{
		if (*(DWORD *)(ptConnection)==ptCurrentClient->ClientID) break;
		
		if (IsBadReadPtr(ptConnection+0x4A8,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)ptClientList);
			return -1;
		}

		ptConnection=(char *)*(DWORD *)(ptConnection+0x4A8);
	}
	
	if (ptConnection==NULL)
	{
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return -4;
	}

	// ptConnection+4
	InitStaus = (DWORD)*(DWORD*)(ptConnection+0x4);
	
	if (InitStaus<4)
	{
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return -2;
	}
	
	// ptConnection+0x0D
	pt2 = (char *)(ptConnection+0x0D);
//	memcpy(ptCurrentClient->CharName,pt2,16);
	if (strcmpi(pt2,ptCurrentClient->CharName) != 0)
	{
		Log(LOG_HACK,"Player CharName: \"%s\"!=\"%s\"[ClientID=%d]\n",pt2,ptCurrentClient->CharName,ptCurrentClient->ClientID);
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return -3;
	}

	// ptConnection+1D
	pt1 = (char *)(ptConnection+0x1D);
	memcpy(ptCurrentClient->AccountName,pt1,20);
		
	// ptConnection+0x174
	pt2 = (char *)(ptConnection+0x174);
	ptCurrentClient->ServerPlayerPtr = (DWORD)*((DWORD *)pt2);
	
	if (ptCurrentClient->ServerReadyTime==0)
	{
		ptCurrentClient->ServerReadyTime=GetTickCount();
		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
		return -3;
	}
	
	last_recv_pack_time = (DWORD)*(DWORD*)(ptConnection+0x3D8);

	// ptConnection+500h
	//InitStaus2 = (DWORD)*(DWORD*)(ptConnection+0x500);
//	ptGame=(DWORD)*(DWORD*)(ptConnection+0x1A8);	//ecx
//	dwUnitType=(DWORD)*(DWORD*)(ptConnection+0x16C);	//eax
//	dwUnitId=(DWORD)*(DWORD*)(ptConnection+0x170);	// edx
//	ptPlayer=D2GAME_GameFindUnitFunc(ptGame,dwUnitId,dwUnitType);
	
//	if (ptPlayer==NULL)
//	{
//		D2GAME_LeaveCriticalSection((DWORD)ptClientList);
//		return -5;
//	}
	
	D2GAME_LeaveCriticalSection((DWORD)ptClientList);
	return (last_recv_pack_time-ptCurrentClient->ServerReadyTime)>4000?1:0;
}
/*
int UpdateClientInfo(struct Client_Data_Struct *ptCurrentClient)
{
	char *pt;
	char *pt1,*pt2;
	char *ptConnection;
	DWORD ptPlayer;
	pt = (char *)D2GAME_GetClient(ptCurrentClient->ClientID);
	if (IsBadReadPtr(pt,4)==0)
	{
		if (IsBadReadPtr(pt+0x88,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -1;
		}
		
		ptConnection = (char *)*(DWORD *)(pt+0x88);
		if (IsBadReadPtr(ptConnection,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -7;
		}
		if ((DWORD)*(DWORD *)ptConnection != ptCurrentClient->ClientID)
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -2;
		}
		// ptConnection+1D
		pt1 = (char *)(ptConnection+0x1D);
		if (pt1) memcpy(ptCurrentClient->AccountName,pt1,20);
		else
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -3;
		}
		
		// ptConnection+0x0D
		pt2 = (char *)(ptConnection+0x0D);
		if (pt2)
		{
			if (strcmpi(pt2,ptCurrentClient->CharName) != 0)
			{
				D2GAME_LeaveCriticalSection((DWORD)pt);
				return -4;
			}
		}
		else
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -5;
		}

		// ptConnection+0x174
		pt2 = (char *)(ptConnection+0x174);
		if (IsBadReadPtr(pt2,4)==0)
		{
			ptPlayer = (DWORD)*((DWORD *)pt2);
			if (IsBadReadPtr((char *)ptPlayer,4))
			{
				D2GAME_LeaveCriticalSection((DWORD)pt);
				return -8;
			}
			ptCurrentClient->ServerPlayerPtr = ptPlayer;
		}
		else
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -6;
		}

		D2GAME_LeaveCriticalSection((DWORD)pt);
		return 1;
	}
	return 0;
}
struct ClientData2_t { 
   DWORD ClientID;               //0x00 
   BYTE _1[0x09];               //0x04 
   char charname[0x10];         //0x0D 
   char account[0x10];            //0x1D 
   BYTE _2[0x3B];               //0x2D 
   LPCLIENTINFO pClientInfo;      //0x68 (seems to be added by d2gs, not by original blizz game) 
   BYTE __2[0x108];            //0x6C 
   LPPLAYERUNIT pPlayerUnit;      //0x174, see remarks 
   BYTE _3[0x30];               //0x178 
   LPGAMEOBJECT pGameObject;      //0x1A8 
   BYTE _4[0x22C];               //0x1AC 
   DWORD last_recv_pack_time;      //0x3D8 
   BYTE _5[0xCC];               //0x3DC 
   LPCLIENTDATA2 PrevCharInGame;   //0x4A8 
   LPCLIENTDATA2 NextCd2InDbByClientId;   //0x4AC next cd2 in D2Game+1115E0 database 
   LPCLIENTDATA2 NextCd2InDbByNamehash;   //0x4B0 next cd2 in D2Game+1111E0 database 
// 3D8 - last recv packet time ? 
// 174 - ptr to char object, use: push 0,mov esi, cdata2,call 1045600,mov charobj,eax 
// 1A8 - LPGAMEOBJECT 
// 4A8 - next char in game 
// ... 
}; 
*/

int CheckClientReady(DWORD ClientID)
{
	char *pt;
	char *ptConnection;
	DWORD InitStaus1,InitStaus2;
	pt = (char *)D2GAME_GetClient(ClientID);
	if (IsBadReadPtr(pt,4)==0)
	{
		if (IsBadReadPtr(pt+0x88,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -1;
		}
		
		ptConnection = (char *)*(DWORD *)(pt+0x88);
		if (IsBadReadPtr(ptConnection,4))
		{
			D2GAME_LeaveCriticalSection((DWORD)pt);
			return -2;
		}
		// ptConnection+4
		InitStaus1 = (DWORD)*(DWORD*)(ptConnection+0x4);
		// ptConnection+500h
		InitStaus2 = (DWORD)*(DWORD*)(ptConnection+0x500);
		D2GAME_LeaveCriticalSection((DWORD)pt);
		
		if (InitStaus2)
			return InitStaus1;
		else
			return 0;
	}
	return 0;
}

//main

extern "C" DWORD __stdcall d2warden_thread(struct D2Warden_Parameter_Struct *lpParameter)
{
	if (!RelocD2Ptrs((DWORD *)&pD2PtrsListStart, (DWORD *)&pD2PtrsListEnd))
	{
		ExitThread(0xff);
		return 0;
	}
	
	srand( (unsigned)time( NULL ) );
	memset(&D2Warden_Parameter,0,sizeof(D2Warden_Parameter));
	D2Warden_Parameter.Warden_Client_Data = NULL;
	InitializeCriticalSection(&D2Warden_Parameter.WardenLock);

	Warden_Init();
	if (Warden_Enable == false)
	{
		DeleteCriticalSection(&D2Warden_Parameter.WardenLock);
		Log(LOG_INFO,"d2warden_thread: Warden disable for this server.\n");
		ExitThread(0xff);
		return 0;
	}
	
	Log(LOG_INFO,"d2warden_thread: Starting Warden Loop on this server.\n");
	while (1)
	{
		// 扫描所有Client_Data，进行Warden处理
		WardenLoop();
		Sleep(1);
	}
	
	if (WardenHackCheckArray) free(WardenHackCheckArray);
	DeleteCriticalSection(&D2Warden_Parameter.WardenLock);
	Log(LOG_INFO,"d2warden_thread: Warden Loop stop on this server.\n");
	return 1;
}

int RealHackCheckNumbers;

int Reading_HackCheck(char *filename,char *HackName)
{
	char buf[1024];
	char buf2[512];
	int count;

	count = GetPrivateProfileSection(HackName,buf,100,filename);
	if ( count == 0)
	{
		Log(LOG_INFO,"Section %s can't be found in d2warden.ini.\n",HackName);
		return -1;
	};

	WardenHackCheckArray[RealHackCheckNumbers].HackCHKSUM = 0;
	WardenHackCheckArray[RealHackCheckNumbers].CorrectCHKSUM = 0;
	memcpy(WardenHackCheckArray[RealHackCheckNumbers].HackName,HackName,50);
	WardenHackCheckArray[RealHackCheckNumbers].D2Version = (unsigned char)GetPrivateProfileInt(HackName,"D2Version",0,filename);
	WardenHackCheckArray[RealHackCheckNumbers].Log = (unsigned char)GetPrivateProfileInt(HackName,"Log",0,filename);
	WardenHackCheckArray[RealHackCheckNumbers].Action = (unsigned char)GetPrivateProfileInt(HackName,"Action",0,filename);
	WardenHackCheckArray[RealHackCheckNumbers].Debug = (unsigned char)GetPrivateProfileInt(HackName,"Debug",0,filename);
	WardenHackCheckArray[RealHackCheckNumbers].CheckTimes = GetPrivateProfileInt(HackName,"CheckTimes",0,filename);
	GetPrivateProfileString(HackName,"HackCHKSUM",NULL,buf,1024,filename);
	if (strlen(buf))
	{
		removespace(buf,strlen(buf));
		count = ConvertHexStringToBytes(buf,(unsigned char *)buf2,512);
		memcpy(&WardenHackCheckArray[RealHackCheckNumbers].HackCHKSUM,buf2,4);
	}
	GetPrivateProfileString(HackName,"CorrectCHKSUM",NULL,buf,1024,filename);
	if (strlen(buf))
	{
		removespace(buf,strlen(buf));
		count = ConvertHexStringToBytes(buf,(unsigned char *)buf2,512);
		memcpy(&WardenHackCheckArray[RealHackCheckNumbers].CorrectCHKSUM,buf2,4);
	}

	WardenHackCheckArray[RealHackCheckNumbers].MsgOffset = GetPrivateProfileInt(HackName,"Offset",0,filename);
	if (WardenHackCheckArray[RealHackCheckNumbers].MsgOffset > 64)
	{
		Log(LOG_INFO,"%s can't set Offset > 64.\n",HackName);
		return -1;
	}
	GetPrivateProfileString(HackName,"HackVALUE",NULL,buf,1024,filename);
	WardenHackCheckArray[RealHackCheckNumbers].HackChatLen = 0;
	if (strlen(buf))
	{
		removespace(buf,strlen(buf));
		count = ConvertHexStringToBytes(buf,(unsigned char *)buf2,512);
		if (count>64) count = 64;
		if (WardenHackCheckArray[RealHackCheckNumbers].MsgOffset>=0)
			count = count -(int)WardenHackCheckArray[RealHackCheckNumbers].MsgOffset;
		WardenHackCheckArray[RealHackCheckNumbers].HackChatLen = count;
		memcpy(&WardenHackCheckArray[RealHackCheckNumbers].HackChat,buf2,count);
	}

	if (WardenHackCheckArray[RealHackCheckNumbers].HackCHKSUM == 0 && WardenHackCheckArray[RealHackCheckNumbers].CorrectCHKSUM == 0 && WardenHackCheckArray[RealHackCheckNumbers].HackChatLen == 0)
	{
		Log(LOG_INFO,"Reading_HackCheck: HackCHKSUM && CorrectCHKSUM && HackVALUE not set in %s\n",HackName);
		return -1;
	}

	GetPrivateProfileString(HackName,"Msg",NULL,buf,256,filename);
	trimspaces(buf);
	strcpy(WardenHackCheckArray[RealHackCheckNumbers].Msg,buf);

	GetPrivateProfileString(HackName,"Check",NULL,buf,1024,filename);
	if (strlen(buf) == 0)
	{
		Log(LOG_INFO,"Reading_HackCheck: Check not set in %s\n",HackName);
		return -1;
	}
	removespace(buf,strlen(buf));
	if (strcmpi(buf,"SS"))
	{
		count = ConvertHexStringToBytes(buf,(unsigned char *)buf2,512);
		WardenHackCheckArray[RealHackCheckNumbers].PacketAELen=count;
		memcpy(WardenHackCheckArray[RealHackCheckNumbers].PacketAE,buf2,count);
		if (count < 2)
		{
			Log(LOG_INFO,"Reading_HackCheck: Check should be hex string in %s\n",HackName);
			return -1;
		}
	}
	else
	{
		WardenHackCheckArray[RealHackCheckNumbers].PacketAELen = 0;
	}
	
	RealHackCheckNumbers++;
	
	return 0;
}

void Warden_Init(void)
{
	// 初始化Warden模块，从d2warden.ini中读取Warden配置参数
	char filename[MAX_PATH];
	FILE *fp;
	int i;
	char buf[1024];
	
	Log(LOG_HACK,"d2warden version v2.06 by marsgod started.Every client use %d bytes memory.\n",sizeof(struct Client_Data_Struct));

	log_level = 0;
	GetModuleFileName(NULL,filename,MAX_PATH);
	strcpy(strrchr(filename,'\\')+1,"d2warden.ini");

	log_level = (unsigned char)GetPrivateProfileInt("Global","Log_level",0,filename);
	WardenCheckInterv = GetPrivateProfileInt("Global","Check_Interv",10,filename);
	ChatMsgUpdateInterv = GetPrivateProfileInt("Global","UpdateInterv",10,filename);
	EnablePlayerMouseCheck = (BYTE)GetPrivateProfileInt("Global","EnablePlayerMouseCheck",0,filename);
	EnableMMBotCheck = (BYTE)GetPrivateProfileInt("Global","EnableMMBotCheck",0,filename);
	
	if (WardenCheckInterv < 1) WardenCheckInterv = 1;

	GetPrivateProfileString("Global","EnableHackChecks",NULL,buf,1024,filename);

	if (strlen(buf)==0)
	{
		Log(LOG_INFO,"Warden_Init: In [Global], EnableHackChecks not set.\n");
		Warden_Enable = false;
		return;
	}
	WardenHackCheckNumbers = 1;
	for (i=0;i<(int)strlen(buf);i++)
	{
		if (buf[i] == ',' || buf[i] == ';') WardenHackCheckNumbers++;
	}
	
	Log(LOG_INFO,"Warden_Init: Alloc memory for %d WardenHackCheck entries.\n",WardenHackCheckNumbers);
	WardenHackCheckArray = (struct WardenHackCheckStruct *)malloc(WardenHackCheckNumbers*sizeof(struct WardenHackCheckStruct));
	if (WardenHackCheckArray==NULL)
	{
		Log(LOG_INFO,"Warden_Init: Can't alloc memory for WardenHackCheckArray(%d entries)\n",WardenHackCheckNumbers);
		Warden_Enable = false;
		return;
	}
	
	RealHackCheckNumbers = 0;
	char *token;
	char temp_str[512];
	token = strtok(buf,",;\n");
	while(token != NULL)
	{
		strcpy(temp_str,token);
		trimspaces(temp_str);
		if (strlen(temp_str))
			Reading_HackCheck(filename,temp_str);
		token = strtok(NULL,",;\n");
	}
	
	Log(LOG_HACK,"Warden_Init: %d WardenHackCheck entries read from d2warden.ini\n",RealHackCheckNumbers);
	WardenHackCheckNumbers = RealHackCheckNumbers;
	
	GetPrivateProfileString("Global","SupportD2ClientVersion",NULL,buf,1024,filename);

	if (strlen(buf)==0)
	{
		for (i=0;i<sizeof(SupportD2Version);i++)
			SupportD2Version[i]=i;
	}
	else
	{
		memset(SupportD2Version,0xff,sizeof(SupportD2Version));
		char *token;
		char temp_str[512];
		char ss=0;
		token = strtok(buf,",;\n");
		while(token != NULL && ss<sizeof(SupportD2Version))
		{
			strcpy(temp_str,token);
			trimspaces(temp_str);
			if (strlen(temp_str))
			{
				i=atoi(temp_str);
				SupportD2Version[ss]=i;
				ss++;
			}
			token = strtok(NULL,",;\n");
		}
	}

	GetPrivateProfileString("Global","MOD",NULL,Warden_MOD,256,filename);
	if (strlen(Warden_MOD) != 36)
	{
		Log(LOG_INFO,"Warden_Init: Invalid MOD filename = %s(len!=36)\n",Warden_MOD);
		Warden_Enable = false;
		return;
	}

	fp = fopen(Warden_MOD,"rb");
	if (fp==NULL)
	{
		Log(LOG_INFO,"Warden_Init: Can't open MOD file = %s!\n",Warden_MOD);
		Warden_Enable = false;
		return;
	}
	fseek(fp,0,SEEK_END);
	MOD_Length = ftell(fp);
	
	if (MOD_Length <= 260 || MOD_Length > 100000)
	{
		Log(LOG_INFO,"Warden_Init: Invalid MOD Length = %d\n",MOD_Length);
		Warden_Enable = false;
		fclose(fp);
		return;
	}
	
	MOD_Enrypted = (unsigned char *)malloc(MOD_Length);
	if (MOD_Enrypted == NULL)
	{
		Log(LOG_INFO,"Warden_Init: Unable to alloc memory for MOD file(%d bytes).\n",MOD_Length);
		Warden_Enable = false;
		fclose(fp);
		return;
	}
	
	fseek(fp,0,SEEK_SET);
	fread(MOD_Enrypted,1,MOD_Length,fp);
	fclose(fp);
	
	if (MOD_Enrypted[MOD_Length-260] != 'N' || MOD_Enrypted[MOD_Length-259] != 'G' || MOD_Enrypted[MOD_Length-258] != 'I' || MOD_Enrypted[MOD_Length-257] != 'S')
	{
		Log(LOG_INFO,"Warden_Init: Invalid SIGN of MOD file..corrupt?\n");
		Warden_Enable = false;
		free(MOD_Enrypted);
		return;
	}
	
	GetPrivateProfileString("Global","MOD_RC4_Key",NULL,buf,256,filename);
	if (strlen(buf) == 0)
	{
		Log(LOG_INFO,"Warden_Init: Unable to read MOD_RC4_Key from d2warden.ini!\n");
		Warden_Enable = false;
		free(MOD_Enrypted);
		return;
	}
	
	removespace(buf,strlen(buf));
	char buf2[512];
	int count;
	count = ConvertHexStringToBytes(buf,(unsigned char *)buf2,512);
	if (count!=16)
	{
		Log(LOG_INFO,"Warden_Init: MOD_RC4_Key should be 16 hex bytes!\n");
		Warden_Enable = false;
		free(MOD_Enrypted);
		return;
	}
//	memcpy(RC4_Key,buf2,16);

	unsigned char s[258];
	rc4_setup(s,RC4_Key,16);
	rc4_crypt(s,MOD_Enrypted,MOD_Length);
/*
	MD5_CTX context;
	unsigned char digest[16];
	unsigned char digest1[16];
	
	MD5Init (&context);
	MD5Update (&context, MOD_Enrypted, MOD_Length);
	MD5Final (digest, &context);


	MD5Init (&context);
	MD5Update (&context, MOD_Enrypted, MOD_Length);
	MD5Update (&context, Maiev, sizeof(Maiev));
	MD5Final (digest1, &context);
	
	for (i=0;i<16;i++)
	{
		// 检查mod的名字是否等于MD5的校验和
	}

	// MOD_Length*8调用Double_MD5
	// D9DF8ADAD5959F307FD000AC06F577E598008D01
	
	unsigned char ptResult[16];
	DWORD Length_MD5[16];

	for (i=0;i<16;i++) Length_MD5[i] = 0;

	Length_MD5[0] = MOD_Length*8;
	
	Double_MD5(Length_MD5, 0, ptResult);
*/	
	AE_Packet00[00] = 0xAE;
	AE_Packet00[01] = 0x25;
	AE_Packet00[02] = 0x00;
	AE_Packet00[03] = 0x00;
	
	unsigned char tt[2];
	
	for (i=0;i<16;i++)
	{
		tt[0] = Warden_MOD[2*i];
		tt[1] = Warden_MOD[2*i+1];
		AE_Packet00[4+i] = Convert2HEX(tt);
		AE_Packet00[20+i] = RC4_Key[i];
	}
		
	AE_Packet00[36] = (unsigned char)(MOD_Length % 256);
	AE_Packet00[37] = (unsigned char)((MOD_Length / 256) % 256);
	AE_Packet00[38] = (unsigned char)((MOD_Length / 65536) % 256);
	AE_Packet00[39] = (unsigned char)((MOD_Length / 16777216) % 256);
	
	WardenCMD2_local[0] = 0xAE;
	
	Warden_Enable = true;
	Log(LOG_INFO,"Warden_Init: MOD %s loaded succssfully.\n",Warden_MOD);
	Log(LOG_INFO,"Warden_Init: Init done.\n");
	
	return;
}

void WardenLoop(void)
{
	
	struct Client_Data_Struct *ptCurrentClient,*ptNext;
	DWORD CurrentTick,SlowTick;
	DWORD TheCheckSum;
	char buf[1024];
	int i;
	DWORD Packet66_ReceiveTime;
	DWORD Packet66_PacketLen;
	unsigned char ThePacket[MAX66LEN];
	
	
	ptCurrentClient = D2Warden_Parameter.Warden_Client_Data;
	while(ptCurrentClient)
	{
		CurrentTick = GetTickCount();
		if (ptCurrentClient->NextCheckTime > CurrentTick && ptCurrentClient->NextCheckTime != NEVER)
		{
			ptCurrentClient = ptCurrentClient->pNext;
			continue;
		}
		
		if ((CheckValidClient(ptCurrentClient->ClientID,ptCurrentClient->ClientLogonTime) == false) || ptCurrentClient->NextCheckTime == NEVER)
		{
			unsigned char cause;
			// 当前客户端已经是无效客户端，或者永远不会检查，去掉
			if (CheckValidClient(ptCurrentClient->ClientID,ptCurrentClient->ClientLogonTime) == false) cause=0;
			else
			if (ptCurrentClient->NextCheckTime == NEVER) cause=1;
			else
				cause=0xff;
			Log(LOG_DEBUG,"WardenLoop: RemoveClientData ClientID=%04d WardenStatus=%d cause=%d \n",ptCurrentClient->ClientID,ptCurrentClient->WardenStatus,cause);

			EnterCriticalSection(&D2Warden_Parameter.WardenLock);
			if (D2Warden_Parameter.Warden_Client_Data == ptCurrentClient)
			{
				D2Warden_Parameter.Warden_Client_Data = ptCurrentClient->pNext;
				free(ptCurrentClient);
				ptCurrentClient = D2Warden_Parameter.Warden_Client_Data;
			}
			else
			{
				ptNext = D2Warden_Parameter.Warden_Client_Data;
				while (ptNext->pNext != ptCurrentClient) ptNext=ptNext->pNext;
					
				ptNext->pNext = ptCurrentClient->pNext;
				free(ptCurrentClient);
				ptCurrentClient = ptNext->pNext;
			}
			LeaveCriticalSection(&D2Warden_Parameter.WardenLock);
			continue;
		}
		
		if (1)//ptCurrentClient->NextCheckTime < CurrentTick)
		{
			// 已经到时候检查客户端了
			if (ptCurrentClient->NextTimeout < CurrentTick)
			{
				// 接收超时
				if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Log & 0x04)
				{
					Log(LOG_HACK,"Warden_Loop: Player %s(*%s) time out[HackCheck=%s reason=%d]..\n",ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,ptCurrentClient->TimeoutReason);
				}
				if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Action & 0x04)
					ptCurrentClient->WardenStatus = WARDEN_TIMEOUT;
				else
				if (ptCurrentClient->WardenStatus==WARDEN_WAIT_FOR_CHECK_RESULT)
					{
							ptCurrentClient->NextHackCheck++;
							if (ptCurrentClient->NextHackCheck >= WardenHackCheckNumbers)
							{
								ptCurrentClient->NextHackCheck = 0;
								ptCurrentClient->CheckTimes++;
								ptCurrentClient->NextCheckTime = SlowTick;	// 下一轮检查
								ptCurrentClient->NextTimeout = SlowTick+15000;
								if (ptCurrentClient->EnablePlayerMouseCheck)
								{
//									ptCurrentClient->NextCheckTime = CurrentTick;	// 下一轮检查
//									ptCurrentClient->NextTimeout = CurrentTick+15000;
									goto try_update_targetxy;
								}
								ptCurrentClient->WardenStatus = WARDEN_CHECK_HACKS;
								break;
							}
							/*
							if (ptCurrentClient->NextHackCheck == i)
							{
								// OK, no hack check found... skip
								ptCurrentClient->NextCheckTime = NEVER;
								ptCurrentClient->NextTimeout = NEVER;
								Log(LOG_DEBUG,"WardenLoop: No hack check for D2Version=%d\n",ptCurrentClient->D2Version);
								break;
							}
							*/
					}
				else
					{
						ptCurrentClient->WardenStatus = WARDEN_TIMEOUT;
					}
			}
			
			// 保证能够在下次不会超时
			SlowTick = CurrentTick + WardenCheckInterv;
			
			Packet66_ReceiveTime=ptCurrentClient->Packet66_ReceiveTime;
			Packet66_PacketLen=ptCurrentClient->Packet66_PacketLen;
			if (Packet66_PacketLen>0)
			{
				memcpy(ThePacket,ptCurrentClient->ThePacket,Packet66_PacketLen);
			}
			
			if (ptCurrentClient->ErrorCount > MAXERRORCOUNT)
			{
				ptCurrentClient->WardenStatus = WARDEN_ERROR_RESPONSE;
				//ptCurrentClient->ErrorResponseReason = 0;
			}
			
			switch (ptCurrentClient->WardenStatus)
			{
				case WARDEN_START:
					for (i=0;i<sizeof(SupportD2Version);i++)
					{
						if (SupportD2Version[i]!=0xff && SupportD2Version[i]==ptCurrentClient->D2Version)
							break;
					}
					if (i==sizeof(SupportD2Version))
					{
						UPDATE_WARDEN_RIGHTNOW(WARDEN_UNSUPPORT_D2_VERSION);
						break;
					}					
					int retval;
					ptCurrentClient->TimeoutReason=WARDEN_START;
					if ((retval = UpdateClientInfo(ptCurrentClient)) < 1)
					{
						ptCurrentClient->TimeoutReason=100+retval;
						ptCurrentClient->NextCheckTime = CurrentTick+500;	// 立刻检查
						//ptCurrentClient->NextTimeout = CurrentTick+15000;
						//Log(LOG_HACK,"ERROR: WARDEN_START ClientID=%04d Invalid[%d]\n",ptCurrentClient->ClientID,retval);
						break;
					}
					
					ptCurrentClient->ErrorCount = 0;						
					UPDATE_WARDEN(WARDEN_START1,CurrentTick+1000,CurrentTick+25000);// 10秒后再检查
					break;
				case WARDEN_START1:
					// 首先查询客户端是否存在指定的MOD，发送AE00报文
					ptCurrentClient->LastCheckTime=CurrentTick;
					memcpy(WardenCMD0_local,&AE_Packet00[3],37);
					rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&AE_Packet00[3],37);
					memcpy(ptCurrentClient->AE_Packet00,AE_Packet00,sizeof(AE_Packet00));
Send_AE00_Again:
					ptCurrentClient->TimeoutReason=WARDEN_START1;
					Log(LOG_DEBUG,"WardenLoop: WARDEN_START1 ClientID=%04d Send AE Packet\n",ptCurrentClient->ClientID);
					Remove0X66Packet(ptCurrentClient);
					D2NET_SendPacket(0,ptCurrentClient->ClientID,AE_Packet00,sizeof(AE_Packet00));
					memcpy(&AE_Packet00[3],WardenCMD0_local,37);	// restore AE Packet
					UPDATE_WARDEN(WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT,CurrentTick+1000,CurrentTick+15000);
					break;
				case WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT:
					ptCurrentClient->TimeoutReason=WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT;
					// 检查客户端返回AE00报文的结果
					Log(LOG_DEBUG,"WardenLoop: WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT  ClientID=%04d\n",ptCurrentClient->ClientID);
					if (Packet66_PacketLen)
					{
						Log(LOG_DEBUG,"WardenLoop: WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT  Got Packet ClientID=%04d\n",ptCurrentClient->ClientID);
						// 接收到AE 00的应答报文
						Remove0X66Packet(ptCurrentClient);
						if (Packet66_PacketLen != 1)
						{
							ptCurrentClient->ErrorResponseReason = 1;
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							Log(LOG_DEBUG,"WardenLoop: WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT  Error Response ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
						
						if (ThePacket[0] == 0)
						{
							UPDATE_WARDEN_RIGHTNOW(WARDEN_DOWNLOAD_MOD);
							Log(LOG_DEBUG,"WardenLoop: WARDEN_NeedDownload ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
						else
						if (ThePacket[0] == 1)
						{
							if (ptCurrentClient->EnablePlayerMouseCheck)
								UPDATE_WARDEN_RIGHTNOW(WARDEN_GET_CLIENT_STATUS)
							else
								UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS)
							Log(LOG_DEBUG,"WardenLoop: WARDEN_AlreadyDownloaded ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
						else
						{
							ptCurrentClient->ErrorResponseReason = 2;
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							Log(LOG_DEBUG,"WardenLoop: WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT  Error Response ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
					}
					else
					{
						/*
						if (CurrentTick>ptCurrentClient->LastCheckTime+2000)
						{
							Log(LOG_DEBUG,"WardenLoop: WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT Try Resend AE Packet ClientID=%04d\n",ptCurrentClient->ClientID);
							ptCurrentClient->ErrorCount++;
							ptCurrentClient->ErrorResponseReason=19;
							ptCurrentClient->LastCheckTime=CurrentTick;
							goto Send_AE00_Again;
						}
						*/
						ptCurrentClient->NextCheckTime = CurrentTick + 1000;
					}
					break;
				case WARDEN_DOWNLOAD_MOD:
					ptCurrentClient->TimeoutReason=WARDEN_DOWNLOAD_MOD;
					Remove0X66Packet(ptCurrentClient);
					ptCurrentClient->MOD_Position = SendPartOfMOD2Client(ptCurrentClient->ClientID,ptCurrentClient->RC4_KEY_0XAE,ptCurrentClient->MOD_Position);
					if (ptCurrentClient->MOD_Position >= MOD_Length)
					{
						UPDATE_WARDEN_RIGHTNOW(WARDEN_WAITING_DOWNLOAD_END);
						Log(LOG_DEBUG,"WardenLoop: WARDEN_DownloadCompelete ClientID=%04d\n",ptCurrentClient->ClientID);
						break;
					}
					else
						UPDATE_WARDEN(WARDEN_DOWNLOAD_MOD,CurrentTick+200,CurrentTick+15000);
					break;
				case WARDEN_WAITING_DOWNLOAD_END:
					ptCurrentClient->TimeoutReason=WARDEN_WAITING_DOWNLOAD_END;
					//Log(LOG_DEBUG,"WardenLoop: WARDEN_WAITING_DOWNLOAD_END ClientID=%04d[%08X,%08X]\n",ptCurrentClient->ClientID,CurrentTick,ptCurrentClient->NextTimeout);
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						if (Packet66_PacketLen != 1)
						{
							ptCurrentClient->ErrorResponseReason = 3;
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							break;
						}
						
						if (ThePacket[0] == 0)
						{
							ptCurrentClient->ErrorCount++;
							UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_CLIENT_HAS_MOD_OR_NOT);
							Log(LOG_DEBUG,"WardenLoop: WARDEN_DownloadNotOK ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
						else
						if (ThePacket[0] == 1)
						{
							if (ptCurrentClient->EnablePlayerMouseCheck)
								UPDATE_WARDEN_RIGHTNOW(WARDEN_GET_CLIENT_STATUS)
							else
								UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS)
							ptCurrentClient->ErrorCount=0;
							Log(LOG_DEBUG,"WardenLoop: WARDEN_DownloadedOK ClientID=%04d\n",ptCurrentClient->ClientID);
							break;
						}
						else
						{
							ptCurrentClient->ErrorResponseReason = 4;
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							break;
						}
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;

				case WARDEN_GET_CLIENT_STATUS:
					ptCurrentClient->TimeoutReason=WARDEN_GET_CLIENT_STATUS;
					if (ptCurrentClient->D2Version == 1)
					{
						((DWORD*)(&GetPtrLocationCMD2_0[30]))[0] =0x0011C1E0; // ptPlayerUnit
						((DWORD*)(&GetPtrLocationCMD2_0[37]))[0] =0x000A12F4; // ptMissilesTxt
					}
					else
					if (ptCurrentClient->D2Version == 2)
					{
						((DWORD*)(&GetPtrLocationCMD2_0[30]))[0] =0x0011C3D0;	// ptPlayerUnit
						((DWORD*)(&GetPtrLocationCMD2_0[37]))[0] =0x000A12BC; // ptMissilesTxt
					}
					else
					if (ptCurrentClient->D2Version == 3)
					{
						((DWORD*)(&GetPtrLocationCMD2_0[30]))[0] =0x0011D344; // ptPlayerUnit
						((DWORD*)(&GetPtrLocationCMD2_0[37]))[0] =0x000A05DC; // ptMissilesTxt
					}
					else
					{
						Remove0X66Packet(ptCurrentClient);
						UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
						break;
					}
					
					Remove0X66Packet(ptCurrentClient);
					WardenCMD2_local[1] = sizeof(GetPtrLocationCMD2_0);
					WardenCMD2_local[2] = 0;
					memcpy(&WardenCMD2_local[3],GetPtrLocationCMD2_0,sizeof(GetPtrLocationCMD2_0));
					rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(GetPtrLocationCMD2_0));
					D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(GetPtrLocationCMD2_0)+3);
					
					UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_CLIENT_STATUS0);
					break;
				case WARDEN_WAIT_FOR_CLIENT_STATUS0:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_CLIENT_STATUS0;
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Get Client ptPlayerUnit:\n%s\n",ptCurrentClient->ClientID,buf);
						}

						if (Packet66_PacketLen == 17)
						{
							unsigned char tt[102];
							memcpy(tt,ThePacket,20);
							memcpy(&(ptCurrentClient->PlayerPtr),&(ThePacket[8]),4);
							memcpy(&(ptCurrentClient->pMissilesTxt),&(ThePacket[13]),4);
							if (ptCurrentClient->PlayerPtr != 0 && ptCurrentClient->pMissilesTxt != 0)
							{
								DWORD tt1,tt2;
								tt1 = ptCurrentClient->PlayerPtr+0x2C;
								tt2 = ptCurrentClient->pMissilesTxt + 0x9828;
								memcpy(&Get2DWORDCMD2[4],&tt1,4);
								memcpy(&Get2DWORDCMD2[11],&tt2,4);

								WardenCMD2_local[1] = sizeof(Get2DWORDCMD2);
								WardenCMD2_local[2] = 0;
								memcpy(&WardenCMD2_local[3],Get2DWORDCMD2,sizeof(Get2DWORDCMD2));
								rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(Get2DWORDCMD2));
								D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(Get2DWORDCMD2)+3);
					
								UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_CLIENT_STATUS1);
								ptCurrentClient->ErrorCount=0;
							}
							else
							{
								ptCurrentClient->ErrorCount++;
								UPDATE_WARDEN_RIGHTNOW(WARDEN_GET_CLIENT_STATUS);
							}
						}
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
					
				case WARDEN_WAIT_FOR_CLIENT_STATUS1:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_CLIENT_STATUS1;
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Get Client ptPlayerPath:\n%s\n",ptCurrentClient->ClientID,buf);
						}

						if (Packet66_PacketLen == 17)
						{
							DWORD tt,tt2;
							memcpy(&tt,&(ThePacket[8]),4);
							memcpy(&tt2,&(ThePacket[13]),4);
							if (tt2==0x6C6C756E && EnableMMBotCheck)
							{
								Log(LOG_HACK,"Hack: Player %s(*%s) use hack MM.bot[Wrong Missiles.txt]\n",ptCurrentClient->CharName,ptCurrentClient->AccountName);
								if (EnableMMBotCheck>=3)
								{
									// Kick Player
									SendMsgToClient(ptCurrentClient->ClientID,"You are killed by Warden because use MM.bot!");
									D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
									ptCurrentClient->NextTimeout = NEVER;
									ptCurrentClient->NextCheckTime = NEVER;
									break;
								}
							}
							if (tt != 0)
							{
								ptCurrentClient->PlayerPathPtr = tt+0x10;
								ptCurrentClient->NextCheckTime = CurrentTick;	// 下一次检查
								ptCurrentClient->NextTimeout = CurrentTick+15000;

try_update_targetxy:
								DWORD eModeLocation;
								eModeLocation = ptCurrentClient->PlayerPtr + 0x10;
								memcpy(&Get2DWORDCMD2[4],&ptCurrentClient->PlayerPathPtr,4);
								memcpy(&Get2DWORDCMD2[11],&eModeLocation,4);
								WardenCMD2_local[1] = sizeof(Get2DWORDCMD2);
								WardenCMD2_local[2] = 0;
								memcpy(&WardenCMD2_local[3],Get2DWORDCMD2,sizeof(Get2DWORDCMD2));
								rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(Get2DWORDCMD2));
								D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(Get2DWORDCMD2)+3);

								ptCurrentClient->WardenStatus = WARDEN_WAIT_FOR_CLIENT_STATUS2;
							}
						}
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;

				case WARDEN_WAIT_FOR_CLIENT_STATUS2:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_CLIENT_STATUS2;
					DWORD ServerPlayerTargetXY,ServerPlayerTargetUnit;
					DWORD ServerPlayerXY;
					DWORD ServerPlayerInteractiveUnitID,ServerPlayerInteractiveUnitType,ServerPlayerInteractiving;
					DWORD ServerPlayerEMode;
					DWORD ptServerPath;
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						if (IsBadReadPtr((DWORD *)(ptCurrentClient->ServerPlayerPtr+0x2C),4))
						{
							ptCurrentClient->NextCheckTime = CurrentTick + 50;
							ptCurrentClient->ErrorCount++;
							break;
						}

						ptServerPath = (DWORD)*(DWORD *)(ptCurrentClient->ServerPlayerPtr+0x2C);
						if (IsBadReadPtr((DWORD *)ptServerPath,4))
						{
							ptCurrentClient->NextCheckTime = CurrentTick + 50;
							break;
						}
						
						ptCurrentClient->ErrorCount = 0;

						ServerPlayerTargetUnit = *(DWORD*)(ptServerPath+0x58);
						ServerPlayerTargetXY = *(DWORD*)(ptServerPath+0x10);
						ServerPlayerXY = *(DWORD*)(ptServerPath+0x00);
						ServerPlayerInteractiveUnitID = *(DWORD*)(ptCurrentClient->ServerPlayerPtr+0x64);
						ServerPlayerInteractiveUnitType = *(DWORD*)(ptCurrentClient->ServerPlayerPtr+0x68);
						ServerPlayerInteractiving = *(DWORD*)(ptCurrentClient->ServerPlayerPtr+0x6C);
						ServerPlayerEMode = *(DWORD*)(ptCurrentClient->ServerPlayerPtr+0x10);
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Get Client ptPlayerTargetXY:\n%s\n",ptCurrentClient->ClientID,buf);
							Log(LOG_HACK,"Position=%08X TargetXY=[%08X,%04X] Interactive=[%08X,%02X,%02X] EMode=[%02X,%02X]\n",ServerPlayerXY,ServerPlayerTargetXY,ServerPlayerTargetUnit,ServerPlayerInteractiveUnitID,ServerPlayerInteractiveUnitType,ServerPlayerInteractiving,ServerPlayerEMode,ptCurrentClient->MissMatchEModeCounter);
						}

						if (Packet66_PacketLen == 17)
						{
								DWORD ClientPlayerTargetXY;
								DWORD ClientEMode;
								memcpy(&ClientPlayerTargetXY,&(ThePacket[8]),4);
								memcpy(&ClientEMode,&(ThePacket[13]),4);
								if (ClientEMode != 1 && ClientEMode != 5)
									ptCurrentClient->MissMatchEModeCounter = 0xFF;
									
								if (ptCurrentClient->MissMatchEModeCounter != 0xFF)
								{
									// NEC套装BUG，在城内，server端是5，而客户端是1....
									if (ServerPlayerEMode != ClientEMode && ServerPlayerEMode != 5 && ServerPlayerEMode != 1)
										ptCurrentClient->MissMatchEModeCounter++;
								}
								
								if (ptCurrentClient->MissMatchEModeCounter > 10 && ptCurrentClient->MissMatchEModeCounter != 0xFF)
								{
										Log(LOG_HACK,"Hack: Player %s(*%s) use hack non-mouse bot[eMode]\n",ptCurrentClient->CharName,ptCurrentClient->AccountName);
										// Kick Player
										SendMsgToClient(ptCurrentClient->ClientID,"You are killed by Warden because no Animation on your screen!");
										D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
										ptCurrentClient->NextCheckTime = NEVER;
										break;
								}
								
								if (ClientPlayerTargetXY == 0 && ServerPlayerTargetXY != 0)
								{
									// 服务器的TargetXY已经更新，而客户端还没有更新。玩家用了非鼠标操作的Hack或者Bot
									ptCurrentClient->PlayerZeroTargetCounter++;
									if (0)//ptCurrentClient->PlayerZeroTargetCounter > 1)
									{
										Log(LOG_HACK,"Hack: Player %s(*%s) use hack non-mouse bot[TargetXY]\n",ptCurrentClient->CharName,ptCurrentClient->AccountName);
										// Kick Player
										SendMsgToClient(ptCurrentClient->ClientID,"You are killed by Warden because TP in town!");
										D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
										ptCurrentClient->NextCheckTime = NEVER;
										break;
									}
									else
									if (ptCurrentClient->PlayerZeroTargetCounter == 1)
									{
										// 如果有一次同步失效，立刻进行第二次检测
										ptCurrentClient->NextCheckTime = CurrentTick+50;	// 下一次检查
										ptCurrentClient->NextTimeout = CurrentTick + 15000;
										goto try_update_targetxy;
									}
									else
									{
										UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
									}
								}
								else
								{
									if (ClientPlayerTargetXY != 0 || ServerPlayerTargetXY == 0)
										ptCurrentClient->PlayerZeroTargetCounter = 0;
									UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
								}
				
								ptCurrentClient->NextCheckTime = CurrentTick;	// 立刻检查
								ptCurrentClient->NextTimeout = CurrentTick + 15000;
						}
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
				case WARDEN_CHECK_HACKS:
					ptCurrentClient->TimeoutReason=WARDEN_CHECK_HACKS;
					if (CurrentTick - ptCurrentClient->LastChatUpdateTime>ChatMsgUpdateInterv && ChatMsgUpdateInterv != 0)
					{
							Remove0X66Packet(ptCurrentClient);
							UPDATE_WARDEN_RIGHTNOW(WARDEN_UPDATE_CHAT_MSG);
							break;
					}
					i = ptCurrentClient->NextHackCheck;
					while (1)
					{
						if ((WardenHackCheckArray[ptCurrentClient->NextHackCheck].D2Version == 0 || WardenHackCheckArray[ptCurrentClient->NextHackCheck].D2Version == ptCurrentClient->D2Version) && (ptCurrentClient->CheckTimes < WardenHackCheckArray[ptCurrentClient->NextHackCheck].CheckTimes || WardenHackCheckArray[ptCurrentClient->NextHackCheck].CheckTimes == 0))
						{
							if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen)
							{
								WardenCMD2_local[1] = WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen % 256;
								WardenCMD2_local[2] = WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen / 256;
								memcpy(&WardenCMD2_local[3],WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAE,WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen);
								rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen);
								D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,WardenHackCheckArray[ptCurrentClient->NextHackCheck].PacketAELen+3);
						
								UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_CHECK_RESULT);
								Log(LOG_DEBUG,"WardenLoop: WARDEN_CheckingHacks ClientID=%04d HackName=%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName);
							
								break;
							}
							else
							if (ptCurrentClient->NewMsg)
							{
								ptCurrentClient->NewMsg = 0;
								// 检查Chat信息
								for (i=0;i<MAX_CHAT_MSG_BUF;i++)
								{
									if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].MsgOffset>=0)
									{
										if (memcmp(&(ptCurrentClient->ChatMsg[i][WardenHackCheckArray[ptCurrentClient->NextHackCheck].MsgOffset]),WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackChat,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackChatLen) == 0)
										{
											if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Debug)
											{
												DumpDataAsText(ptCurrentClient->ChatMsg[i], 64, buf, 1024);
												Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
											}
											goto HackChatDetected;
										}
									}
									else
									{
										// 跳到分号之后
										int j;
										for (j=0;j<40;j++)
										{
											if (ptCurrentClient->ChatMsg[i][j] != 0x1A)
												continue;
											if (memcmp(&(ptCurrentClient->ChatMsg[i][j+2]),WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackChat,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackChatLen) == 0)
											{
												if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Debug)
												{
													DumpDataAsText(ptCurrentClient->ChatMsg[i], 64, buf, 1024);
													Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
												}
												goto HackChatDetected;
											}
										}
									}
								}
							}
							goto NextCheck;
						}
						else
						{
							ptCurrentClient->NextHackCheck++;
							if (ptCurrentClient->NextHackCheck >= WardenHackCheckNumbers)
							{
								ptCurrentClient->NextHackCheck = 0;
								ptCurrentClient->CheckTimes++;
								ptCurrentClient->NextCheckTime = SlowTick;	// 下一轮检查
								ptCurrentClient->NextTimeout = SlowTick+15000;
								if (ptCurrentClient->EnablePlayerMouseCheck)
								{
//									ptCurrentClient->NextCheckTime = CurrentTick;	// 下一轮检查
//									ptCurrentClient->NextTimeout = CurrentTick+15000;
									goto try_update_targetxy;
								}
								ptCurrentClient->WardenStatus = WARDEN_CHECK_HACKS;
								break;
							}
							if (ptCurrentClient->NextHackCheck == i)
							{
								// OK, no hack check found... skip
								ptCurrentClient->NextCheckTime = NEVER;
								ptCurrentClient->NextTimeout = NEVER;
								Log(LOG_DEBUG,"WardenLoop: No hack check for D2Version=%d\n",ptCurrentClient->D2Version);
								break;
							}
						}
					}
					break;
				case WARDEN_WAIT_FOR_CHECK_RESULT:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_CHECK_RESULT;
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Debug)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
						}

						if (Packet66_PacketLen >= 7)
						{
							memcpy(&TheCheckSum,&(ThePacket[3]),4);
							if (TheCheckSum == WardenHackCheckArray[ptCurrentClient->NextHackCheck].CorrectCHKSUM && TheCheckSum != 0)
							{
								// Correct response.
NextCheck:
								ptCurrentClient->NextHackCheck++;
								if (ptCurrentClient->NextHackCheck >= WardenHackCheckNumbers)
								{
									ptCurrentClient->NextHackCheck = 0;
									ptCurrentClient->CheckTimes++;
									ptCurrentClient->NextCheckTime = SlowTick;	// 下一轮检查
									ptCurrentClient->NextTimeout = SlowTick+15000;
									if (ptCurrentClient->EnablePlayerMouseCheck)
									{
//										ptCurrentClient->NextCheckTime = CurrentTick;	// 下一轮检查
//										ptCurrentClient->NextTimeout = CurrentTick+15000;
										goto try_update_targetxy;
									}
								}
								ptCurrentClient->WardenStatus = WARDEN_CHECK_HACKS;
							}
							else
							if (TheCheckSum == WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackCHKSUM && TheCheckSum != 0)
							{

HackChatDetected:
								// Hack detected.
								if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Log & 0x01)
								{
									// Log enable
									Log(LOG_HACK,"Hack: Player %s(*%s) use hack %s\n",ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName);
								}
								if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Action & 0x01)
								{
									// Kick Player
									UPDATE_WARDEN_RIGHTNOW(WARDEN_KICK_PLAYER);
								}
								else
								{
									UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
									goto NextCheck;
								}

							}
							else
							{
								// Unknown response.
								if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Log & 0x02)
								{
									// Log enable
									Log(LOG_HACK,"Hack: Player %s(*%s) may use hack %s(Unknown response)\n",ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName);
								}
								if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Action & 0x02)
								{
									// Kick Player
									UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
									ptCurrentClient->ErrorResponseReason = 5;
								}
								else
								{
									UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
									goto NextCheck;
								}
							}
						}
						else
						{
							// Unknown response.
							if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Log & 0x02)
							{
								// Log enable
								Log(LOG_HACK,"Hack: Player %s(*%s) may use hack %s(Unknown response)\n",ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName);
							}
							if (WardenHackCheckArray[ptCurrentClient->NextHackCheck].Action & 0x02)
							{
								// Kick Player
								UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
								ptCurrentClient->ErrorResponseReason = 6;
							}
							else
							{
								UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
								goto NextCheck;
							}
						}
						if (ptCurrentClient->WardenStatus == WARDEN_CHECK_HACKS && ptCurrentClient->NextHackCheck == 0)
						{
							ptCurrentClient->NextCheckTime = SlowTick;	// 下一轮检查
							ptCurrentClient->NextTimeout = SlowTick+15000;
						}
						else
						{
							ptCurrentClient->NextCheckTime = CurrentTick;	// 下一次检查
							ptCurrentClient->NextTimeout = CurrentTick+15000;
						}
						Log(LOG_DEBUG,"WardenLoop: WARDEN_ClientReturnHackCheck ClientID=%04d\n",ptCurrentClient->ClientID);
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
				case WARDEN_UPDATE_CHAT_MSG:
					ptCurrentClient->TimeoutReason=WARDEN_UPDATE_CHAT_MSG;
					Log(LOG_DEBUG,"WardenLoop: Updating Player Chat Msg 1, chatid=%d\n",ptCurrentClient->ChatMsgIndex);
					if (ptCurrentClient->D2Version == 1)
						((DWORD*)(&GetChatLocationCMD2_0[17]))[0] =0x0011b830;
					else
					if (ptCurrentClient->D2Version == 2)
						((DWORD*)(&GetChatLocationCMD2_0[17]))[0] =0x001054D8;
					else
					if (ptCurrentClient->D2Version == 3)
						((DWORD*)(&GetChatLocationCMD2_0[17]))[0] =0x00108240;
					else
					{
						Remove0X66Packet(ptCurrentClient);
						UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
						break;
					}
					
					Remove0X66Packet(ptCurrentClient);
					WardenCMD2_local[1] = sizeof(GetChatLocationCMD2_0);
					WardenCMD2_local[2] = 0;
					memcpy(&WardenCMD2_local[3],GetChatLocationCMD2_0,sizeof(GetChatLocationCMD2_0));
					rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(GetChatLocationCMD2_0));
					D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(GetChatLocationCMD2_0)+3);
					
					UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER);
					break;
				case WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER;
					//NextMsgRecord MsgNum
					int CurrentNumberOfMsgInBuffer;
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						ptCurrentClient->LastChatUpdateTime = CurrentTick;
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
						}

						if (Packet66_PacketLen == 14)
						{
							CurrentNumberOfMsgInBuffer = 0;
							memcpy(&CurrentNumberOfMsgInBuffer,&(ThePacket[12]),2);
							memcpy(&(ptCurrentClient->NextMsgAddress),&(ThePacket[8]),4);
							/*
							if (CurrentNumberOfMsgInBuffer<2) 
							{
								// 不可能一条消息都没有，肯定是Anti Warden的保护
								ptCurrentClient->WardenStatus = WARDEN_ERROR_RESPONSE;
								ptCurrentClient->NextCheckTime = CurrentTick+50;	// 下一次检查
								ptCurrentClient->NextTimeout = CurrentTick + 15000;
								ptCurrentClient->ErrorResponseReason = 7;
								Log(LOG_HACK,"Hack: Player %s(*%s) use anti warden hack,maybe cGuard?(%d chat)\n",ptCurrentClient->CharName,ptCurrentClient->AccountName,CurrentNumberOfMsgInBuffer);
								break;
							}
							*/
							if (CurrentNumberOfMsgInBuffer>0) CurrentNumberOfMsgInBuffer --;

							if (ptCurrentClient->LatestNumberOfMsgInBuffer < CurrentNumberOfMsgInBuffer)
							{
								ptCurrentClient->NumberOfMsgInBuffer = CurrentNumberOfMsgInBuffer - ptCurrentClient->LatestNumberOfMsgInBuffer;
								if (ptCurrentClient->NumberOfMsgInBuffer > MAX_CHAT_MSG_BUF)
								{
									ptCurrentClient->NumberOfMsgInBuffer = MAX_CHAT_MSG_BUF;
									ptCurrentClient->LatestNumberOfMsgInBuffer = ptCurrentClient->LatestNumberOfMsgInBuffer + ptCurrentClient->NumberOfMsgInBuffer;
								}
								else
								{
//									ptCurrentClient->NumberOfMsgInBuffer = MAX_CHAT_MSG_BUF;
									ptCurrentClient->LatestNumberOfMsgInBuffer = CurrentNumberOfMsgInBuffer;
								}
								// Player have msg
								Log(LOG_DEBUG,"WardenLoop: Updating Player Chat Msg 2, chatid=%d Total %d msg in buffer\n",ptCurrentClient->ChatMsgIndex,ptCurrentClient->NumberOfMsgInBuffer);
//								ptCurrentClient->NumberOfMsgInBuffer --;
								memcpy(&GetChatLocationCMD2_1[4],&(ptCurrentClient->NextMsgAddress),4);
								WardenCMD2_local[1] = sizeof(GetChatLocationCMD2_1);
								WardenCMD2_local[2] = 0;
								memcpy(&WardenCMD2_local[3],GetChatLocationCMD2_1,sizeof(GetChatLocationCMD2_1));
								rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(GetChatLocationCMD2_1));
								D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(GetChatLocationCMD2_1)+3);
					
								UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER2);
							}
							else
							{
								UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
							}
						}
						else
						{
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							ptCurrentClient->ErrorResponseReason = 8;
						}
						ptCurrentClient->NextCheckTime = CurrentTick;	// 下一次检查
						ptCurrentClient->NextTimeout = CurrentTick+15000;
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
					
				case WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER2:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER2;
					// MsgAddress NextMsgRecord
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						ptCurrentClient->LastChatUpdateTime = CurrentTick;
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
						}

						if (Packet66_PacketLen == 46)
						{
							memcpy(&(ptCurrentClient->NextMsgAddress),&(ThePacket[8]),4);
								// Player have msg
							Log(LOG_DEBUG,"WardenLoop: Updating Player Chat Msg 3, chatid=%d\n",ptCurrentClient->ChatMsgIndex);
							memcpy(&GetChatLocationCMD2_2[4],&(ptCurrentClient->NextMsgAddress),4);
							WardenCMD2_local[1] = sizeof(GetChatLocationCMD2_2);
							WardenCMD2_local[2] = 0;
							memcpy(&WardenCMD2_local[3],GetChatLocationCMD2_2,sizeof(GetChatLocationCMD2_2));
							rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(GetChatLocationCMD2_2));
							D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(GetChatLocationCMD2_2)+3);
								
							memcpy(&(ptCurrentClient->NextMsgAddress),&(ThePacket[42]),4);
							UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER3);
						}
						else
						{
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							ptCurrentClient->ErrorResponseReason = 9;
						}
						ptCurrentClient->NextCheckTime = CurrentTick;	// 下一次检查
						ptCurrentClient->NextTimeout = CurrentTick+15000;
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
				case WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER3:
					ptCurrentClient->TimeoutReason=WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER3;
					// Msg
					if (Packet66_PacketLen)
					{
						Remove0X66Packet(ptCurrentClient);
						ptCurrentClient->LastChatUpdateTime = CurrentTick;
						if (log_level>3)
						{
							DumpDataAsText(ThePacket, Packet66_PacketLen, buf, 1024);
							Log(LOG_HACK,"WardenLoop: ClientID=%d Hack=%s Response:\n%s\n",ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,buf);
						}

						if (Packet66_PacketLen == 72)
						{
							memcpy(ptCurrentClient->ChatMsg[ptCurrentClient->ChatMsgIndex],&(ThePacket[8]),64);
							ptCurrentClient->NewMsg=1;
							ptCurrentClient->NumberOfMsgInBuffer --;
							if (ptCurrentClient->NumberOfMsgInBuffer > 0)
							{
								// Player have msg
								Log(LOG_DEBUG,"WardenLoop: Updating Player Chat Msg 4, chatid=%d\n",ptCurrentClient->ChatMsgIndex);
								memcpy(&GetChatLocationCMD2_1[4],&(ptCurrentClient->NextMsgAddress),4);
								WardenCMD2_local[1] = sizeof(GetChatLocationCMD2_1);
								WardenCMD2_local[2] = 0;
								memcpy(&WardenCMD2_local[3],GetChatLocationCMD2_1,sizeof(GetChatLocationCMD2_1));
								rc4_crypt(ptCurrentClient->RC4_KEY_0XAE,&WardenCMD2_local[3],sizeof(GetChatLocationCMD2_1));
								D2NET_SendPacket(0,ptCurrentClient->ClientID,WardenCMD2_local,sizeof(GetChatLocationCMD2_1)+3);
					
								UPDATE_WARDEN_RIGHTNOW(WARDEN_WAIT_FOR_PLAYER_CHAT_MSG_POINTER2);
							}
							else
							{
								UPDATE_WARDEN_RIGHTNOW(WARDEN_CHECK_HACKS);
							}
						}
						else
						{
							UPDATE_WARDEN_RIGHTNOW(WARDEN_ERROR_RESPONSE);
							ptCurrentClient->ErrorResponseReason = 10;
						}
						ptCurrentClient->ChatMsgIndex++;
						if (ptCurrentClient->ChatMsgIndex == MAX_CHAT_MSG_BUF)
							ptCurrentClient->ChatMsgIndex = 0;
						ptCurrentClient->NextCheckTime = CurrentTick;	// 下一次检查
						ptCurrentClient->NextTimeout = CurrentTick+15000;
					}
					else
						ptCurrentClient->NextCheckTime = CurrentTick + 50;
					break;
				case WARDEN_KICK_PLAYER:
					Log(LOG_DEBUG,"WardenLoop: WARDEN_KickCharFromGame ClientID=%04d Player=%s(*%s) HackName=%s\n",ptCurrentClient->ClientID,ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName);
					ptCurrentClient->NextCheckTime = NEVER;
					ptCurrentClient->NextTimeout = NEVER;
					ptCurrentClient->WardenStatus = WARDEN_WAIT_FOR_CHECK_RESULT;
					if (strlen(WardenHackCheckArray[ptCurrentClient->NextHackCheck].Msg))
						SendMsgToClient(ptCurrentClient->ClientID,WardenHackCheckArray[ptCurrentClient->NextHackCheck].Msg);
					D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
					break;
				case WARDEN_ERROR_RESPONSE:
					Log(LOG_DEBUG,"WardenLoop: WARDEN_KickCharFromGame ClientID=%04d Player=%s(*%s) HackName=%s Error response[reason=%d]\n",ptCurrentClient->ClientID,ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,ptCurrentClient->ErrorResponseReason);
					ptCurrentClient->NextCheckTime = NEVER;
					ptCurrentClient->NextTimeout = NEVER;
					ptCurrentClient->WardenStatus = WARDEN_WAIT_FOR_CHECK_RESULT;
					sprintf(buf,"You are killed by Warden because error response.[reason=%d]",ptCurrentClient->ErrorResponseReason);
					SendMsgToClient(ptCurrentClient->ClientID,buf);
					D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
					break;
				case WARDEN_TIMEOUT:
					Log(LOG_DEBUG,"WardenLoop: WARDEN_KickCharFromGame ClientID=%04d Player=%s(*%s) HackName=%s timeout[reason=%d]\n",ptCurrentClient->ClientID,ptCurrentClient->CharName,ptCurrentClient->AccountName,WardenHackCheckArray[ptCurrentClient->NextHackCheck].HackName,ptCurrentClient->TimeoutReason);
					ptCurrentClient->NextCheckTime = NEVER;
					ptCurrentClient->NextTimeout = NEVER;
					ptCurrentClient->WardenStatus = WARDEN_WAIT_FOR_CHECK_RESULT;
					sprintf(buf,"You are killed by Warden because timeout.[reason=%d]",ptCurrentClient->TimeoutReason);
					SendMsgToClient(ptCurrentClient->ClientID,buf);
					D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
					break;
				case WARDEN_UNSUPPORT_D2_VERSION:
					Log(LOG_DEBUG,"WardenLoop: WARDEN_KickCharFromGame ClientID=%04d Player=%s(*%s) unsupport d2 version[version=%d]\n",ptCurrentClient->ClientID,ptCurrentClient->CharName,ptCurrentClient->AccountName,ptCurrentClient->D2Version);
					ptCurrentClient->NextCheckTime = NEVER;
					ptCurrentClient->NextTimeout = NEVER;
					ptCurrentClient->WardenStatus = WARDEN_WAIT_FOR_CHECK_RESULT;
					sprintf(buf,"You are killed by Warden because using unsupport d2 client[version=%d].",ptCurrentClient->D2Version);
					SendMsgToClient(ptCurrentClient->ClientID,buf);
					D2GAME_KickCharFromGame(ptCurrentClient->ClientID);
					break;
			}
		}
		
		ptCurrentClient = ptCurrentClient->pNext;
	}
}

extern "C" DWORD  __fastcall d2warden_0X66Handler(DWORD ptGame, DWORD ptPlayer, unsigned char *ptPacket, DWORD PacketLen)
{
	// 66 xxl xxh ....
	// xxh:xxl = 剩余Packet长度
	
	struct Client_Data_Struct *ptWarden_Client_Data;
	DWORD ClientID;
	DWORD InternalLen;
	
	if (PacketLen < 3)
		return 3;
	// ptPlayer->14h->9Ch

	ClientID = *((DWORD *)(*(((DWORD*)*((DWORD *)ptPlayer+0x5))+0x27)));
	InternalLen = ptPacket[2]*256 + ptPacket[1];
	
	if (InternalLen <= 0 || InternalLen >= MAX66LEN)
	{
		// 错误的66报文长度！
		return 3;
	}

	Log(LOG_DEBUG,"0X66Handler: Got Client 0x66 Packet ClientID=%04d\n",ClientID);
	ptWarden_Client_Data = D2Warden_Parameter.Warden_Client_Data;
	while (ptWarden_Client_Data)
	{
		if (ptWarden_Client_Data->ClientID == ClientID)
		{
			if (ptWarden_Client_Data->CanRecv66Packet == 1)
			{
				// 使用RC4解密数据
				memcpy(ptWarden_Client_Data->ThePacket,ptPacket+3,InternalLen);
				rc4_crypt(ptWarden_Client_Data->RC4_KEY_0X66,ptWarden_Client_Data->ThePacket,InternalLen);
				ptWarden_Client_Data->Packet66_ReceiveTime = GetTickCount();
				ptWarden_Client_Data->CanRecv66Packet = 0;	// 一次只能接收一个0x66报文
				ptWarden_Client_Data->Packet66_PacketLen=InternalLen;
//				Log(LOG_DEBUG,"0X66Handler: Got Client 0x66 Packet ClientID=%04d\n",ClientID);
			}
			else
				ptWarden_Client_Data->ErrorCount++;
			break;
		}
		ptWarden_Client_Data = ptWarden_Client_Data->pNext;
	}
	
	return 0;
}


/*
.text:6FD05AF7 mov     eax, esi
.text:6FD05AF9 call    sub_6FCE49A0  mov     dword ptr [eax+178h], 1
esi=ptClient
*/

extern "C" DWORD  __fastcall d2warden_0X68Handler(unsigned char *ClientID_Packet)//,unsigned char *ptClient)
{
	struct Client_Data_Struct *NewClientData;
	struct Client_Data_Struct *ptWarden_Client_Data;
	DWORD ClientID;
	char *ptConnection,*pt2;
	unsigned char *ptPacket;
	
	ptPacket=ClientID_Packet+4;

	unsigned char RC4_KEY_0X66[16],RC4_KEY_0XAE[16];
	DWORD D2Version;

	if (ptPacket == NULL || IsBadReadPtr(ClientID_Packet,4)) return 0;
	
	memcpy(&D2Version,&ptPacket[8],4);
	
//	if (D2Version != 0x0b && D2Version != 0x0c && D2Version != 0x0d)
//	{
//		Log(LOG_DEBUG,"0X68Handler: Unknown D2Version=%X Player=%s\n",D2Version,&ptPacket[21]);
//		return 0;
//	}

	ClientID=*((DWORD *)ClientID_Packet);
	ptWarden_Client_Data = D2Warden_Parameter.Warden_Client_Data;
	while (ptWarden_Client_Data)
	{
		if (ptWarden_Client_Data->ClientID == ClientID)
		{
			// 非法68报文
			ptWarden_Client_Data->ErrorCount++;
			Log(LOG_DEBUG,"0X68Handler: Hack D2Client D2Version=%X Player=%s\n",D2Version,&ptPacket[21]);
			return 0;
		}
		ptWarden_Client_Data = ptWarden_Client_Data->pNext;
	}

	Log(LOG_DEBUG,"0X68Handler: Login D2Version=%X Player=%s ClientID=%04d\n",D2Version,&ptPacket[21],ClientID);
	NewClientData = (struct Client_Data_Struct*)malloc(sizeof(struct Client_Data_Struct));
	if (NewClientData == NULL)
	{
//		Log(LOG_DEBUG,"0X68Handler: Can't alloc memory for NewClient ClientID=%04d len=%d\n",ClientID,sizeof(struct Client_Data_Struct));
		return 0;
	}
	
	memset(NewClientData->CharName,0,sizeof(NewClientData->CharName));
	memset(NewClientData->AccountName,0,sizeof(NewClientData->AccountName));
	memcpy(NewClientData->CharName,&ptPacket[21],16);

	NewClientData->ClientID = ClientID;
	memcpy(NewClientData->SessionKey,&ptPacket[1],4);
	NewClientData->ClientLogonTime = GetTickCount();
	NewClientData->CanRecv66Packet = 1;
	NewClientData->NextCheckTime = NewClientData->ClientLogonTime+MyRand()*1000;
	NewClientData->LastCheckTime = NewClientData->NextCheckTime;
	NewClientData->WardenStatus = WARDEN_START;
	NewClientData->Packet66_ReceiveTime = 0;
	NewClientData->Packet66_PacketLen = 0;
	memset(NewClientData->ThePacket,0,MAX66LEN);
	NewClientData->MOD_Position = 0;
	NewClientData->NextHackCheck = 0;
	NewClientData->D2Version = D2Version-0x0a;//(D2Version == 0x0b)?1:(D2Version == 0x0c)?2:(D2Version == 0x0d)?3:4;
	NewClientData->ErrorCount = 0;
	NewClientData->CheckTimes = 0;
	NewClientData->LastChatUpdateTime = GetTickCount();
	NewClientData->ChatMsgIndex = 0;
	NewClientData->NewMsg = 0;
	NewClientData->NextMsgAddress = 0;
	NewClientData->NumberOfMsgInBuffer = 0;
	NewClientData->LatestNumberOfMsgInBuffer = 0;
	NewClientData->CurrentCheckChatID = 0;
	memset(NewClientData->ChatMsg,0,sizeof(NewClientData->ChatMsg));
	NewClientData->ErrorResponseReason = 0xff;
	NewClientData->TimeoutReason = 0xff;
	NewClientData->ServerReadyTime=0;
	
	
//	NewClientData->PlayerXPosition = 0;
//	NewClientData->PlayerYPosition = 0;
//	NewClientData->PlayerXPosition_Modify = 0;
//	NewClientData->MouseXPosition = 0;
//	NewClientData->MouseYPosition = 0;

	NewClientData->PlayerPtr = 0;
	NewClientData->PlayerPathPtr = 0;
	NewClientData->PlayerZeroTargetCounter = 0;
//	NewClientData->PlayerTargetX = 0;
//	NewClientData->PlayerTargetY = 0;
	NewClientData->ServerPlayerPtr = 0;
	NewClientData->EnablePlayerMouseCheck = EnablePlayerMouseCheck;
	NewClientData->MissMatchEModeCounter = 0;
	
	NewClientData->pMissilesTxt = 0;

	HashGameSeed(RC4_KEY_0XAE, RC4_KEY_0X66, NewClientData->SessionKey, 4);
	rc4_setup(NewClientData->RC4_KEY_0XAE,RC4_KEY_0XAE,16);
	rc4_setup(NewClientData->RC4_KEY_0X66,RC4_KEY_0X66,16);
	NewClientData->pNext = NULL;
	
	EnterCriticalSection(&D2Warden_Parameter.WardenLock);
	NewClientData->NextTimeout = GetTickCount()+20000;
	NewClientData->pNext = D2Warden_Parameter.Warden_Client_Data;
	D2Warden_Parameter.Warden_Client_Data = NewClientData;
	LeaveCriticalSection(&D2Warden_Parameter.WardenLock);

//	Log(LOG_DEBUG,"0X68Handler: NewClientData ClientID=%04d Player=%s\n",NewClientData->ClientID,NewClientData->CharName);
	return 1;
}
/*
extern "C" struct TreasureClassNoDropTbl* __fastcall d2warden_PreCalculateTreasureClassNoDropTbl()
{
	int NoDrop_Chance;
	int Total_Chance;
	int D2C_Drop_Chance,D2X_Drop_Chance;
	float tt1,ff1,tt2,ff2;
	int i,j,NumberOfRecords;
	int DiffCounter;
	struct TreasureClassNoDropTbl*pt;

	DWORD *ptTCRecordsList,*ptTCRecords,*CurrentRecord;
	
	ptTCRecordsList = (DWORD *)0x6FDF114C;
	ptTCRecords = (DWORD *)*(ptTCRecordsList);
	NumberOfRecords = *(ptTCRecordsList+1);

	if (ptTCRecords==NULL || NumberOfRecords ==0) return NULL;
	
	
	//1. 根据TCRecords的基地址以及当前TC的偏移，计算出当前TC的序号
	//2. 使用序号来查找TreasureClassNoDropTbl
	//
	//在6FDF114C处存放了ptTCRecords的基地址，每2C一条记录 +14=NoDrop +0C=Drop(D2X) +08=Drop(D2C)
	//在6FDF1150处存放了TC的总数目
	
	
	pt = (struct TreasureClassNoDropTbl*)malloc(NumberOfRecords*sizeof(struct TreasureClassNoDropTbl));
	if (pt==NULL) return NULL;
	
	DiffCounter = 0;
	for (i=0;i<NumberOfRecords;i++)
	{
		CurrentRecord = (DWORD *)(ptTCRecords+i*0x0B);	// +2C
		if (CurrentRecord == 0) break;
		NoDrop_Chance = (int)*(CurrentRecord+5);	// +14
		D2C_Drop_Chance = (int)*(CurrentRecord+2);	// +08
		D2X_Drop_Chance = (int)*(CurrentRecord+3);	// +0C

		if (NoDrop_Chance==0)
		{
			for (j=0;j<7;j++) pt[i].D2C_NoDrop[j]=pt[i].D2X_NoDrop[j]=0;
			continue;
		}
		ff1 = tt1 = (float)NoDrop_Chance/(float)(NoDrop_Chance+D2C_Drop_Chance);
		ff2 = tt2 = (float)NoDrop_Chance/(float)(NoDrop_Chance+D2X_Drop_Chance);
		for (j=0;j<7;j++)
		{
			ff1 = tt1*ff1;
			pt[i].D2C_NoDrop[j] = (int)((D2C_Drop_Chance /(1.0 - ff1)) * ff1 );
			ff2 = tt2*ff2;
			pt[i].D2X_NoDrop[j] = (int)((D2X_Drop_Chance /(1.0 - ff2)) * ff2 );
		}
	}
	
	return pt;
}


extern LPDEBUG_CLIENT_INFO DebugInitClientInfo(DWORD dwClientId)
{
	LPDEBUG_CLIENT_INFO lpClientInfo;

	lpClientInfo = DEBUG_ALLOC(sizeof(*lpClientInfo));
	if (!lpClientInfo) return NULL;
	memset(lpClientInfo, 0, sizeof(*lpClientInfo));
	lpClientInfo->dwMagic = DEBUG_CLIENT_MAGIC;
	lpClientInfo->ptSelf = lpClientInfo;
	lpClientInfo->dwClientId = dwClientId;
	lpClientInfo->dwLastPingTime = GetTickCount();
	lpClientInfo->dwLastACTime = GetTickCount();
#ifdef DEBUG_D2GS
	DebugLog("DebugInitClientInfo", "allocated client info %p for client %d (%d bytes)", 
		lpClientInfo, dwClientId, sizeof(*lpClientInfo));
#endif
	return lpClientInfo;
}

extern void DebugFreeClientInfo(LPDEBUG_CLIENT_INFO lpClientInfo)
{
	if (lpClientInfo) {
		if (lpClientInfo->dwMagic != DEBUG_CLIENT_MAGIC) {
			DebugLog("DebugFreeClientInfo", "bad client magic 0x%08X", lpClientInfo->dwMagic);
			return;
		}
		if (lpClientInfo->ptSelf != lpClientInfo) {
			DebugLog("DebugFreeClientInfo", "clientInfo ptr mismatch %p/%p", 
				lpClientInfo->ptSelf, lpClientInfo);
			return;
		}
#ifdef DEBUG_D2GS
		DebugLog("DebugFreeClientInfo", "free client info %p for client %d", 
			lpClientInfo, lpClientInfo->dwClientId);
#endif
		DEBUG_FREE(lpClientInfo);
	}
	return;
}

这是申请和释放的地方
#define DEBUG_ALLOC(size)	VirtualAlloc(NULL, (size), MEM_COMMIT, PAGE_READWRITE)
#define DEBUG_FREE(ptr)		VirtualFree(ptr, 0, MEM_RELEASE)


*/