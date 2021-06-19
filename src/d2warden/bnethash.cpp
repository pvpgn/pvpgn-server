#include "introtate.h"
#include "bnethash.h"

static void hash_init(t_hash * hash);
static void do_hash(t_hash * hash, t_uint32 * tmp);
static void hash_set_16(t_uint32 * dst, unsigned char const * src, unsigned int count);


static void hash_init(t_hash * hash)
{
    (*hash)[0] = 0x67452301;
    (*hash)[1] = 0xefcdab89;
    (*hash)[2] = 0x98badcfe;
    (*hash)[3] = 0x10325476;
    (*hash)[4] = 0xc3d2e1f0;
}


static void do_hash(t_hash * hash, t_uint32 * tmp)
{
    unsigned int i;
    t_uint32     a,b,c,d,e,g;
    
    for (i=0; i<64; i++)
	tmp[i+16] = ROTL32(1,tmp[i] ^ tmp[i+8] ^ tmp[i+2] ^ tmp[i+13]);
    
    a = (*hash)[0];
    b = (*hash)[1];
    c = (*hash)[2];
    d = (*hash)[3];
    e = (*hash)[4];
    
    for (i=0; i<20*1; i++)
    {
    	// BSHA_OP1
	g = tmp[i] + ROTL32(a,5) + e + ((b & c) | (~b & d)) + 0x5a827999;
	e = d;
	d = c;
	c = ROTL32(b,30);
	b = a;
	a = g;
    }
    
    for (; i<20*2; i++)
    {
    	// BSHA_OP2
	g = (d ^ c ^ b) + e + ROTL32(g,5) + tmp[i] + 0x6ed9eba1;
	e = d;
	d = c;
	c = ROTL32(b,30);
	b = a;
	a = g;
    }
    
    for (; i<20*3; i++)
    {
    	// BSHA_OP3
	g = tmp[i] + ROTL32(g,5) + e + ((c & b) | (d & c) | (d & b)) - 0x70e44324;
	e = d;
	d = c;
	c = ROTL32(b,30);
	b = a;
	a = g;
    }
    
    for (; i<20*4; i++)
    {
    	// BSHA_OP4
	g = (d ^ c ^ b) + e + ROTL32(g,5) + tmp[i] - 0x359d3e2a;
	e = d;
	d = c;
	c = ROTL32(b,30);
	b = a;
	a = g;
    }
    
    (*hash)[0] += g;
    (*hash)[1] += b;
    (*hash)[2] += c;
    (*hash)[3] += d;
    (*hash)[4] += e;
}


/*
 * Fill 16 elements of the array of 32 bit values with the bytes from
 * dst up to count in little endian order. Fill left over space with
 * zeros
 */
static void hash_set_16(t_uint32 * dst, unsigned char const * src, unsigned int count)
{
    unsigned int i;
    unsigned int pos;
    
    for (pos=0,i=0; i<16; i++)
    {
			dst[i] = 0;
  		if (pos<count)  dst[i] |= ((t_uint32)src[pos]);
			pos++;
  		if (pos<count)  dst[i] |= ((t_uint32)src[pos])<<8;
			pos++;
  		if (pos<count)  dst[i] |= ((t_uint32)src[pos])<<16;
			pos++;
  		if (pos<count)  dst[i] |= ((t_uint32)src[pos])<<24;
			pos++;
    }
}


int bnet_hash(t_hash * hashout, unsigned int size, unsigned char * datain)
{
    t_uint32              tmp[64+16];
    unsigned char const * data;
    unsigned int          inc;
    
    hash_init(hashout);
    
    data = datain;
    while (size>0)
    {
			if (size>64)
	    	inc = 64;
			else
	    	inc = size;
	
			hash_set_16(tmp,data,inc);
			do_hash(hashout,tmp);
	
			data += inc;
			size -= inc;
    }
    
    return 0;
}
