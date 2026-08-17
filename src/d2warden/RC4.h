#ifndef _RC4_H 
#define _RC4_H 

/*  
struct rc4_state 
{ 
    unsigned char x, y, m[256]; 
}; 
*/
  
void rc4_setup( unsigned char *s, unsigned char *key,  int length ); 
void rc4_crypt( unsigned char *s, unsigned char *data, int length ); 
  
#endif

