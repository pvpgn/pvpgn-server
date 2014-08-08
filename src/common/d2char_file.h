#ifndef INCLUDED_COMMON_D2CHAR_FILE
#define INCLUDED_COMMON_D2CHAR_FILE

#define D2CHARFILE_PADBYTE              0xff

namespace pvpgn
{

	typedef struct {
		/* Finish copying from bnet_protocol.h/t_d2_char_info */

		bn_long experience;
		bn_byte reservedxp[30];
		bn_long invgold;
		bn_long bankgold;
		bn_long stashgold;
		bn_long reservedgold[8];
		bn_byte skilllvls[30];
		bn_byte reservedskills[30];

		bn_short strength;
		bn_short vitaility;
		bn_short dexterity;
		bn_short energy;
		bn_short reservedattr[4];

		bn_byte questflags[4];           /* 1 byte/8 bits each act */
		bn_byte reservedquestflags[16];
		bn_byte waypoints[4][3];         /* 3 bytes/24 bits each act */
		bn_byte reservedwaypoints[4][3];

		/* stuff like HP, Mana are calculated dynamically... */

		bn_byte inventory[80];         /* backpack got 40 spaces, double that to be safe */
		bn_byte reservedinv[80];       /* space for expanding */
		bn_byte belt[32];              /* largest belt holds 16, double that to be safe */
		bn_byte reservedbelt[32];      /* space for expanding */

		bn_int deathcount;

		bn_byte body1unknownb1;             /* For dead body... */
		bn_byte body1unknownb2;
		bn_byte body1helmgfx;
		bn_byte body1bodygfx;
		bn_byte body1leggfx;
		bn_byte body1lhandweapon;
		bn_byte body1lhandgfx;
		bn_byte body1rhandweapon;
		bn_byte body1rhandgfx;
		bn_byte body1unknownb3;
		bn_byte body1unknownb4;
		bn_byte body1unknownb5;
		bn_byte body1unknownb6;
		bn_byte body1unknownb7;
		bn_byte body1unknownb8;
		bn_byte body1unknownb9;
		bn_byte body1unknownb10;
		bn_byte body1unknownb11;
		bn_byte body1inventory[80];         /* backpack got 40 spaces, double that to be safe */
		bn_byte body1reservedinv[80];       /* space for expanding */
		bn_byte body1belt[32];              /* largest belt holds 16, double that to be safe */
		bn_byte body1reservedbelt[32];      /* space for expanding */

		bn_byte stashinv[100];        /* Forgot how big the stash is.... */
		bn_byte reservedstash[200];
	} t_d2char_record;

}

/* item
description
base type (1 byte)
item quality (unknown size)
Magic (blue name)
modifer level
magic dword 1
magic dword 2
*/

#endif
