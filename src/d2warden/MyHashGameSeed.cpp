/*
sub_18C1098(unsigned char *result=esi,int length=eax,unsigned char *GameSeed=arg_0)
{
	int var_4;
	var_4 = length/2;
	sub_18C23BC(result+18,GameSeed,var_4);
	result[4] = 0;
	result[5] = 0;
	result[6] = 0;
	result[7] = 0;
	result[8] = 0;
	...
	result[23] = 0;
	
	length = length - var_4;
	sub_18C23BC(result+2C,GameSeed+var_4,length);
	sub_18C1112(result=edi);
}

sub_18C23BC(unsigned char *result,unsigned char *in,int length)
{
	unsigned char buff[0x60];
	BSHA_Init(buff);
	Call_BSHA_Process(buff=esi,in=eax,length=arg_0);
	sub_18C2300(buff=eax,result=arg_0);
}


sub_18C1112(unsigned char *in)
{
	unsigned char buff[0x60];
	BSHA_Init(buff);
	Call_BSHA_Process(buff=esi,in+18=eax,20);
	Call_BSHA_Process(buff=esi,in+04=eax,20);
	Call_BSHA_Process(buff=esi,in+2C=eax,20);
	sub_18C2300(buff=eax,in+4=arg_0);
	in[0~3] = 0;
}

sub_18C2300(buff=eax,result=arg_0)
{
	unsigned char var_8[8];
	sub_18C135B(*buff,*(buff+4),var_8);
	tt = (*buff)/8;
	ecx=(-9-tt) & 0x3F + 1;
	Call_BSHA_Process(buff=esi,unk_18C3010=eax,ecx=arg_0);
	Call_BSHA_Process(buff=esi,var_8,8);
	
	edi=result;
	eax = &(buff+8);
	esi = 5;

loc_18C2343:	
	ecx = *(eax);
	edx = 3;

loc_18C2348:	
	*(result + edx) = cl;
	ecx = ecx / 256;
	edx -- ;
	if (edx >= 0) goto loc_18C2348;
		
	eax + = 4;
	result + = 4;
	esi --;
	
	if (esi != 0) goto loc_18C2343
}

Call_BSHA_Process(buff=esi,in=eax,length=arg_0)
{
	ecx=length*8;
	edi=(*(buff)/8) & 0x3F;
	*(buff) += ecx;
	var_4 = in;
	*(buff+4) += 0;
	if (edi==0) goto loc_18C22C2
	length += edi;
	in -= edi;
	if (length <= 0x40) goto loc_18C22F5
	goto loc_18C22A7

loc_18C229C:
	eax = var_4;
	[buff+edi+1C] = [edi+eax];
	edi ++;
	
	if (edi<=0x40) goto loc_18C229C
	
	BSHA_Process(esi+8=ebx,esi+1Ch=arg_0);
	
	var_4 += 0x40;
	length -= 0x40;
	edi = 0;
	
loc_18C22C2:
	if (length <=0x40) goto loc_18C22F5;
	var_8 = length / 64;

loc_18C22D3:
	BSHA_Process(esi+8=ebx,var_4);
	var_4 += 0x40;
	length -= 0x40;
	var_8 --;
	if (var_8 !=0) goto loc_18C22D3
	goto loc_18C22F5

loc_18C22EA:
	eax = var_4;
	[buff+edi+1C] = [edi+eax];
	edi ++;

	if (edi <= length) goto loc_18C22EA
		
}
*/