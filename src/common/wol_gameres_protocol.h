/*
 * Copyright (C) 2008  Pelish (pelish@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef INCLUDED_WOL_GAMERES_PROTOCOL_TYPES
#define INCLUDED_WOL_GAMERES_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
# include "common/tag.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# include "common/tag.h"
# undef JUST_NEED_TYPES
#endif

/*
 * The "wol gamreres" protocol is used for sending Game reports to server
 * for ladders support.
 */


namespace pvpgn
{

	/******************************************************/
	typedef struct
	{
		bn_short size;
		bn_short rngd_size;
	} PACKED_ATTR() t_wolgameres_header;
	/******************************************************/

	/******************************************************/
	typedef struct
	{
		t_wolgameres_header h;
	} PACKED_ATTR() t_wolgameres_generic;
	/******************************************************/

	/* PELISH: not used now...
	typedef struct
	{
	bn_short type;
	bn_short length;
	//	data;
	} PACKED_ATTR() t_client_wolgameres_generic;
	*/

	/* DATA TYPES */
	const int DATA_TYPE_BYTE = 1;
	const int DATA_TYPE_BOOL = 2;
	const int DATA_TYPE_TIME = 5;
	const int DATA_TYPE_INT = 6;
	const int DATA_TYPE_STRING = 7;
	const int DATA_TYPE_BIGINT = 20;

	/* Game resolution tags */
	const t_tag CLIENT_SERN_UINT = 0x53455223;
	const t_tag CLIENT_SDFX_UINT = 0x53444658;
	const t_tag CLIENT_IDNO_UINT = 0x49444E4F;
	const t_tag CLIENT_GSKU_UINT = 0x47534B55;
	const t_tag CLIENT_DCON_UINT = 0x44434F4E;
	const t_tag CLIENT_LCON_UINT = 0x4C434F4E;
	const t_tag CLIENT_TYPE_UINT = 0x54595045;
	const t_tag CLIENT_TRNY_UINT = 0x54524E59;
	const t_tag CLIENT_OOSY_UINT = 0x4F4F5359;
	const t_tag CLIENT_FINI_UINT = 0x46494E49;
	const t_tag CLIENT_DURA_UINT = 0x44555241;
	const t_tag CLIENT_CRED_UINT = 0x43524544;
	const t_tag CLIENT_SHRT_UINT = 0x53485254;
	const t_tag CLIENT_SUPR_UINT = 0x53555052;
	const t_tag CLIENT_MODE_UINT = 0x4D4F4445;
	const t_tag CLIENT_BAMR_UINT = 0x42414D52;
	const t_tag CLIENT_CRAT_UINT = 0x43524154;
	const t_tag CLIENT_AIPL_UINT = 0x4149504C;
	const t_tag CLIENT_UNIT_UINT = 0x554E4954;
	const t_tag CLIENT_SCEN_UINT = 0x5343454E;
	const t_tag CLIENT_CMPL_UINT = 0x434D504C;
	const t_tag CLIENT_PNGS_UINT = 0x504E4753;
	const t_tag CLIENT_PNGR_UINT = 0x504E4752;
	const t_tag CLIENT_PLRS_UINT = 0x504C5253;
	const t_tag CLIENT_SPID_UINT = 0x53504944;
	const t_tag CLIENT_TIME_UINT = 0x54494D45;
	const t_tag CLIENT_AFPS_UINT = 0x41465053;
	const t_tag CLIENT_PROC_UINT = 0x50524F43;
	const t_tag CLIENT_MEMO_UINT = 0x4D454D4F;
	const t_tag CLIENT_VIDM_UINT = 0x5649444D;
	const t_tag CLIENT_SPED_UINT = 0x53504544;
	const t_tag CLIENT_VERS_UINT = 0x56455253;
	const t_tag CLIENT_DATE_UINT = 0x44415445;
	const t_tag CLIENT_BASE_UINT = 0x42415345;
	const t_tag CLIENT_TIBR_UINT = 0x54494252;
	const t_tag CLIENT_SHAD_UINT = 0x53484144;
	const t_tag CLIENT_FLAG_UINT = 0x464C4147;
	const t_tag CLIENT_TECH_UINT = 0x54454348;
	const t_tag CLIENT_BROK_UINT = 0x42524f4b;
	const t_tag CLIENT_ACCO_UINT = 0x4143434f;
	const t_tag CLIENT_ETIM_UINT = 0x4554494d;
	const t_tag CLIENT_PSPD_UINT = 0x50535044;
	const t_tag CLIENT_SMEM_UINT = 0x534d454d;
	const t_tag CLIENT_SVID_UINT = 0x53564944;
	const t_tag CLIENT_SNAM_UINT = 0x534e414d;
	const t_tag CLIENT_GMAP_UINT = 0x474d4150;
	const t_tag CLIENT_DSVR_UINT = 0x44535652;

	/* RNDG Player tags */
	const t_tag CLIENT_PNAM_UINT = 0x504e414d;
	const t_tag CLIENT_PLOC_UINT = 0x504c4f43;
	const t_tag CLIENT_TEAM_UINT = 0x5445414d;
	const t_tag CLIENT_PSCR_UINT = 0x50534352;
	const t_tag CLIENT_PPTS_UINT = 0x50505453;
	const t_tag CLIENT_PTIM_UINT = 0x5054494d;
	const t_tag CLIENT_PHLT_UINT = 0x50484c54;
	const t_tag CLIENT_PKIL_UINT = 0x504b494c;
	const t_tag CLIENT_EKIL_UINT = 0x454b494c;
	const t_tag CLIENT_AKIL_UINT = 0x414b494c;
	const t_tag CLIENT_SHOT_UINT = 0x53484f54;
	const t_tag CLIENT_HEDF_UINT = 0x48454446;
	const t_tag CLIENT_TORF_UINT = 0x544f5246;
	const t_tag CLIENT_ARMF_UINT = 0x41524d46;
	const t_tag CLIENT_LEGF_UINT = 0x4c454746;
	const t_tag CLIENT_CRTF_UINT = 0x43525446;
	const t_tag CLIENT_PUPS_UINT = 0x50555053;
	const t_tag CLIENT_VKIL_UINT = 0x564b494c;
	const t_tag CLIENT_VTIM_UINT = 0x5654494d;
	const t_tag CLIENT_NKFV_UINT = 0x4e4b4656;
	const t_tag CLIENT_SQUI_UINT = 0x53515549;
	const t_tag CLIENT_PCRD_UINT = 0x50435244;
	const t_tag CLIENT_BKIL_UINT = 0x424b494c;
	const t_tag CLIENT_HEDR_UINT = 0x48454452;
	const t_tag CLIENT_TORR_UINT = 0x544f5252;
	const t_tag CLIENT_ARMR_UINT = 0x41524d52;
	const t_tag CLIENT_LEGR_UINT = 0x4c454752;
	const t_tag CLIENT_CRTR_UINT = 0x43525452;
	const t_tag CLIENT_FLGC_UINT = 0x464c4743;

	/* Player tags */
	const t_tag CLIENT_NAM0_UINT = 0x4E414D30;
	const t_tag CLIENT_NAM1_UINT = 0x4E414D31;
	const t_tag CLIENT_NAM2_UINT = 0x4E414D32;
	const t_tag CLIENT_NAM3_UINT = 0x4E414D33;
	const t_tag CLIENT_NAM4_UINT = 0x4E414D34;
	const t_tag CLIENT_NAM5_UINT = 0x4E414D35;
	const t_tag CLIENT_NAM6_UINT = 0x4E414D36;
	const t_tag CLIENT_NAM7_UINT = 0x4E414D37;

	const t_tag CLIENT_IPA0_UINT = 0x49504130;
	const t_tag CLIENT_IPA1_UINT = 0x49504131;
	const t_tag CLIENT_IPA2_UINT = 0x49504132;
	const t_tag CLIENT_IPA3_UINT = 0x49504133;
	const t_tag CLIENT_IPA4_UINT = 0x49504134;
	const t_tag CLIENT_IPA5_UINT = 0x49504135;
	const t_tag CLIENT_IPA6_UINT = 0x49504136;
	const t_tag CLIENT_IPA7_UINT = 0x49504137;

	const t_tag CLIENT_CID0_UINT = 0x43494430;
	const t_tag CLIENT_CID1_UINT = 0x43494431;
	const t_tag CLIENT_CID2_UINT = 0x43494432;
	const t_tag CLIENT_CID3_UINT = 0x43494433;
	const t_tag CLIENT_CID4_UINT = 0x43494434;
	const t_tag CLIENT_CID5_UINT = 0x43494435;
	const t_tag CLIENT_CID6_UINT = 0x43494436;
	const t_tag CLIENT_CID7_UINT = 0x43494437;

	const t_tag CLIENT_SID0_UINT = 0x53494430;
	const t_tag CLIENT_SID1_UINT = 0x53494431;
	const t_tag CLIENT_SID2_UINT = 0x53494432;
	const t_tag CLIENT_SID3_UINT = 0x53494433;
	const t_tag CLIENT_SID4_UINT = 0x53494434;
	const t_tag CLIENT_SID5_UINT = 0x53494435;
	const t_tag CLIENT_SID6_UINT = 0x53494436;
	const t_tag CLIENT_SID7_UINT = 0x53494437;

	const t_tag CLIENT_TID0_UINT = 0x54494430;
	const t_tag CLIENT_TID1_UINT = 0x54494431;
	const t_tag CLIENT_TID2_UINT = 0x54494432;
	const t_tag CLIENT_TID3_UINT = 0x54494433;
	const t_tag CLIENT_TID4_UINT = 0x54494434;
	const t_tag CLIENT_TID5_UINT = 0x54494435;
	const t_tag CLIENT_TID6_UINT = 0x54494436;
	const t_tag CLIENT_TID7_UINT = 0x54494437;

	const t_tag CLIENT_CMP0_UINT = 0x434D5030;
	const t_tag CLIENT_CMP1_UINT = 0x434D5031;
	const t_tag CLIENT_CMP2_UINT = 0x434D5032;
	const t_tag CLIENT_CMP3_UINT = 0x434D5033;
	const t_tag CLIENT_CMP4_UINT = 0x434D5034;
	const t_tag CLIENT_CMP5_UINT = 0x434D5035;
	const t_tag CLIENT_CMP6_UINT = 0x434D5036;
	const t_tag CLIENT_CMP7_UINT = 0x434D5037;

	const t_tag CLIENT_COL0_UINT = 0x434f4c30;
	const t_tag CLIENT_COL1_UINT = 0x434f4c31;
	const t_tag CLIENT_COL2_UINT = 0x434f4c32;
	const t_tag CLIENT_COL3_UINT = 0x434f4c33;
	const t_tag CLIENT_COL4_UINT = 0x434f4c34;
	const t_tag CLIENT_COL5_UINT = 0x434f4c35;
	const t_tag CLIENT_COL6_UINT = 0x434f4c36;
	const t_tag CLIENT_COL7_UINT = 0x434f4c37;

	const t_tag CLIENT_CRD0_UINT = 0x43524430;
	const t_tag CLIENT_CRD1_UINT = 0x43524431;
	const t_tag CLIENT_CRD2_UINT = 0x43524432;
	const t_tag CLIENT_CRD3_UINT = 0x43524433;
	const t_tag CLIENT_CRD4_UINT = 0x43524434;
	const t_tag CLIENT_CRD5_UINT = 0x43524435;
	const t_tag CLIENT_CRD6_UINT = 0x43524436;
	const t_tag CLIENT_CRD7_UINT = 0x43524437;

	const t_tag CLIENT_INB0_UINT = 0x494e4230;
	const t_tag CLIENT_INB1_UINT = 0x494e4231;
	const t_tag CLIENT_INB2_UINT = 0x494e4232;
	const t_tag CLIENT_INB3_UINT = 0x494e4233;
	const t_tag CLIENT_INB4_UINT = 0x494e4234;
	const t_tag CLIENT_INB5_UINT = 0x494e4235;
	const t_tag CLIENT_INB6_UINT = 0x494e4236;
	const t_tag CLIENT_INB7_UINT = 0x494e4237;

	const t_tag CLIENT_UNB0_UINT = 0x554e4230;
	const t_tag CLIENT_UNB1_UINT = 0x554e4231;
	const t_tag CLIENT_UNB2_UINT = 0x554e4232;
	const t_tag CLIENT_UNB3_UINT = 0x554e4233;
	const t_tag CLIENT_UNB4_UINT = 0x554e4234;
	const t_tag CLIENT_UNB5_UINT = 0x554e4235;
	const t_tag CLIENT_UNB6_UINT = 0x554e4236;
	const t_tag CLIENT_UNB7_UINT = 0x554e4237;

	const t_tag CLIENT_PLB0_UINT = 0x504c4230;
	const t_tag CLIENT_PLB1_UINT = 0x504c4231;
	const t_tag CLIENT_PLB2_UINT = 0x504c4232;
	const t_tag CLIENT_PLB3_UINT = 0x504c4233;
	const t_tag CLIENT_PLB4_UINT = 0x504c4234;
	const t_tag CLIENT_PLB5_UINT = 0x504c4235;
	const t_tag CLIENT_PLB6_UINT = 0x504c4236;
	const t_tag CLIENT_PLB7_UINT = 0x504c4237;

	const t_tag CLIENT_BLB0_UINT = 0x424c4230;
	const t_tag CLIENT_BLB1_UINT = 0x424c4231;
	const t_tag CLIENT_BLB2_UINT = 0x424c4232;
	const t_tag CLIENT_BLB3_UINT = 0x424c4233;
	const t_tag CLIENT_BLB4_UINT = 0x424c4234;
	const t_tag CLIENT_BLB5_UINT = 0x424c4235;
	const t_tag CLIENT_BLB6_UINT = 0x424c4236;
	const t_tag CLIENT_BLB7_UINT = 0x424c4237;

	const t_tag CLIENT_INL0_UINT = 0x494e4c30;
	const t_tag CLIENT_INL1_UINT = 0x494e4c31;
	const t_tag CLIENT_INL2_UINT = 0x494e4c32;
	const t_tag CLIENT_INL3_UINT = 0x494e4c33;
	const t_tag CLIENT_INL4_UINT = 0x494e4c34;
	const t_tag CLIENT_INL5_UINT = 0x494e4c35;
	const t_tag CLIENT_INL6_UINT = 0x494e4c36;
	const t_tag CLIENT_INL7_UINT = 0x494e4c37;

	const t_tag CLIENT_UNL0_UINT = 0x554e4c30;
	const t_tag CLIENT_UNL1_UINT = 0x554e4c31;
	const t_tag CLIENT_UNL2_UINT = 0x554e4c32;
	const t_tag CLIENT_UNL3_UINT = 0x554e4c33;
	const t_tag CLIENT_UNL4_UINT = 0x554e4c34;
	const t_tag CLIENT_UNL5_UINT = 0x554e4c35;
	const t_tag CLIENT_UNL6_UINT = 0x554e4c36;
	const t_tag CLIENT_UNL7_UINT = 0x554e4c37;

	const t_tag CLIENT_PLL0_UINT = 0x504c4c30;
	const t_tag CLIENT_PLL1_UINT = 0x504c4c31;
	const t_tag CLIENT_PLL2_UINT = 0x504c4c32;
	const t_tag CLIENT_PLL3_UINT = 0x504c4c33;
	const t_tag CLIENT_PLL4_UINT = 0x504c4c34;
	const t_tag CLIENT_PLL5_UINT = 0x504c4c35;
	const t_tag CLIENT_PLL6_UINT = 0x504c4c36;
	const t_tag CLIENT_PLL7_UINT = 0x504c4c37;

	const t_tag CLIENT_BLL0_UINT = 0x424c4c30;
	const t_tag CLIENT_BLL1_UINT = 0x424c4c31;
	const t_tag CLIENT_BLL2_UINT = 0x424c4c32;
	const t_tag CLIENT_BLL3_UINT = 0x424c4c33;
	const t_tag CLIENT_BLL4_UINT = 0x424c4c34;
	const t_tag CLIENT_BLL5_UINT = 0x424c4c35;
	const t_tag CLIENT_BLL6_UINT = 0x424c4c36;
	const t_tag CLIENT_BLL7_UINT = 0x424c4c37;

	const t_tag CLIENT_INK0_UINT = 0x494e4b30;
	const t_tag CLIENT_INK1_UINT = 0x494e4b31;
	const t_tag CLIENT_INK2_UINT = 0x494e4b32;
	const t_tag CLIENT_INK3_UINT = 0x494e4b33;
	const t_tag CLIENT_INK4_UINT = 0x494e4b34;
	const t_tag CLIENT_INK5_UINT = 0x494e4b35;
	const t_tag CLIENT_INK6_UINT = 0x494e4b36;
	const t_tag CLIENT_INK7_UINT = 0x494e4b37;

	const t_tag CLIENT_UNK0_UINT = 0x554e4b30;
	const t_tag CLIENT_UNK1_UINT = 0x554e4b31;
	const t_tag CLIENT_UNK2_UINT = 0x554e4b32;
	const t_tag CLIENT_UNK3_UINT = 0x554e4b33;
	const t_tag CLIENT_UNK4_UINT = 0x554e4b34;
	const t_tag CLIENT_UNK5_UINT = 0x554e4b35;
	const t_tag CLIENT_UNK6_UINT = 0x554e4b36;
	const t_tag CLIENT_UNK7_UINT = 0x554e4b37;

	const t_tag CLIENT_PLK0_UINT = 0x504c4b30;
	const t_tag CLIENT_PLK1_UINT = 0x504c4b31;
	const t_tag CLIENT_PLK2_UINT = 0x504c4b32;
	const t_tag CLIENT_PLK3_UINT = 0x504c4b33;
	const t_tag CLIENT_PLK4_UINT = 0x504c4b34;
	const t_tag CLIENT_PLK5_UINT = 0x504c4b35;
	const t_tag CLIENT_PLK6_UINT = 0x504c4b36;
	const t_tag CLIENT_PLK7_UINT = 0x504c4b37;

	const t_tag CLIENT_BLK0_UINT = 0x424c4b30;
	const t_tag CLIENT_BLK1_UINT = 0x424c4b31;
	const t_tag CLIENT_BLK2_UINT = 0x424c4b32;
	const t_tag CLIENT_BLK3_UINT = 0x424c4b33;
	const t_tag CLIENT_BLK4_UINT = 0x424c4b34;
	const t_tag CLIENT_BLK5_UINT = 0x424c4b35;
	const t_tag CLIENT_BLK6_UINT = 0x424c4b36;
	const t_tag CLIENT_BLK7_UINT = 0x424c4b37;

	const t_tag CLIENT_BLC0_UINT = 0x424c4330;
	const t_tag CLIENT_BLC1_UINT = 0x424c4331;
	const t_tag CLIENT_BLC2_UINT = 0x424c4332;
	const t_tag CLIENT_BLC3_UINT = 0x424c4333;
	const t_tag CLIENT_BLC4_UINT = 0x424c4334;
	const t_tag CLIENT_BLC5_UINT = 0x424c4335;
	const t_tag CLIENT_BLC6_UINT = 0x424c4336;
	const t_tag CLIENT_BLC7_UINT = 0x424c4337;

}

#endif  /* INCLUDED_WOL_GAMERES_PROTOCOL_TYPES */

