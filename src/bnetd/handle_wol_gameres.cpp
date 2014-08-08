/*
 * Copyright (C) 2008,2009  Pelish (pelish@gmail.com)
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

#include "common/setup_before.h"
#include "handle_wol_gameres.h"

#include <cstring>

#include "common/wol_gameres_protocol.h"
#include "common/packet.h"
#include "common/eventlog.h"
#include "common/tag.h"
#include "common/bn_type.h"

#include "compat/snprintf.h"

#include "connection.h"
#include "game.h"
#include "account.h"
#ifdef WIN32_GUI
#include <win32/winmain.h>
#endif
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		/* handlers prototypes */
		static int _client_sern(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sdfx(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_idno(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_gsku(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_dcon(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_lcon(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_type(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_trny(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_oosy(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_fini(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_dura(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cred(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_shrt(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_supr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_mode(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bamr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crat(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_aipl(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unit(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_scen(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmpl(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pngs(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pngr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plrs(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_spid(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_time(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_afps(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_proc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_memo(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_vidm(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sped(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_vers(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_date(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_base(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tibr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_shad(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_flag(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tech(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_acco(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_brok(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_etim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pspd(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_smem(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_svid(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_snam(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_gmap(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_dsvr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		/* Renegade Player related functions */
		static int _client_pnam(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ploc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_team(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pscr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ppts(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ptim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_phlt(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ekil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_akil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_shot(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_hedf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_torf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_armf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_legf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crtf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pups(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_vkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_vtim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nkfv(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_squi(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pcrd(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_hedr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_torr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_armr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_legr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crtr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_flgc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		/* Player related functions */
		static int _client_nam0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_nam7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_ipa0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ipa7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_cid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_sid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_sid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_tid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_tid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_cmp0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_cmp7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_col0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_col7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_crd0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_crd7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_inb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_unb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_plb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_blb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_inl0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_inl7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_unl0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unl7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_pll0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_pll7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_bll0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_bll7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_ink0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_ink7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_unk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_unk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_plk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_plk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_blk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		static int _client_blc0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);
		static int _client_blc7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		typedef int(*t_wol_gamerestag)(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data);

		typedef struct {
			t_tag               wol_gamerestag_uint;
			t_wol_gamerestag    wol_gamerestag_handler;
		} t_wol_gamerestag_table_row;

		/* handler table */
		static const t_wol_gamerestag_table_row wol_gamreres_htable[] = {
			{ CLIENT_SERN_UINT, _client_sern },
			{ CLIENT_SDFX_UINT, _client_sdfx },
			{ CLIENT_IDNO_UINT, _client_idno },
			{ CLIENT_GSKU_UINT, _client_gsku },
			{ CLIENT_DCON_UINT, _client_dcon },
			{ CLIENT_LCON_UINT, _client_lcon },
			{ CLIENT_TYPE_UINT, _client_type },
			{ CLIENT_TRNY_UINT, _client_trny },
			{ CLIENT_OOSY_UINT, _client_oosy },
			{ CLIENT_FINI_UINT, _client_fini },
			{ CLIENT_DURA_UINT, _client_dura },
			{ CLIENT_CRED_UINT, _client_cred },
			{ CLIENT_SHRT_UINT, _client_shrt },
			{ CLIENT_SUPR_UINT, _client_supr },
			{ CLIENT_MODE_UINT, _client_mode },
			{ CLIENT_BAMR_UINT, _client_bamr },
			{ CLIENT_CRAT_UINT, _client_crat },
			{ CLIENT_AIPL_UINT, _client_aipl },
			{ CLIENT_UNIT_UINT, _client_unit },
			{ CLIENT_SCEN_UINT, _client_scen },
			{ CLIENT_CMPL_UINT, _client_cmpl },
			{ CLIENT_PNGS_UINT, _client_pngs },
			{ CLIENT_PNGR_UINT, _client_pngr },
			{ CLIENT_PLRS_UINT, _client_plrs },
			{ CLIENT_SPID_UINT, _client_spid },
			{ CLIENT_TIME_UINT, _client_time },
			{ CLIENT_AFPS_UINT, _client_afps },
			{ CLIENT_PROC_UINT, _client_proc },
			{ CLIENT_MEMO_UINT, _client_memo },
			{ CLIENT_VIDM_UINT, _client_vidm },
			{ CLIENT_SPED_UINT, _client_sped },
			{ CLIENT_VERS_UINT, _client_vers },
			{ CLIENT_DATE_UINT, _client_date },
			{ CLIENT_BASE_UINT, _client_base },
			{ CLIENT_TIBR_UINT, _client_tibr },
			{ CLIENT_SHAD_UINT, _client_shad },
			{ CLIENT_FLAG_UINT, _client_flag },
			{ CLIENT_TECH_UINT, _client_tech },
			{ CLIENT_BROK_UINT, _client_brok },
			{ CLIENT_ACCO_UINT, _client_acco },
			{ CLIENT_ETIM_UINT, _client_etim },
			{ CLIENT_PSPD_UINT, _client_pspd },
			{ CLIENT_SMEM_UINT, _client_smem },
			{ CLIENT_SVID_UINT, _client_svid },
			{ CLIENT_SNAM_UINT, _client_snam },
			{ CLIENT_GMAP_UINT, _client_gmap },
			{ CLIENT_DSVR_UINT, _client_dsvr },

			/* Renegade Player related tags */
			{ CLIENT_PNAM_UINT, _client_pnam },
			{ CLIENT_PLOC_UINT, _client_ploc },
			{ CLIENT_TEAM_UINT, _client_team },
			{ CLIENT_PSCR_UINT, _client_pscr },
			{ CLIENT_PPTS_UINT, _client_ppts },
			{ CLIENT_PTIM_UINT, _client_ptim },
			{ CLIENT_PHLT_UINT, _client_phlt },
			{ CLIENT_PKIL_UINT, _client_pkil },
			{ CLIENT_EKIL_UINT, _client_ekil },
			{ CLIENT_AKIL_UINT, _client_akil },
			{ CLIENT_SHOT_UINT, _client_shot },
			{ CLIENT_HEDF_UINT, _client_hedf },
			{ CLIENT_TORF_UINT, _client_torf },
			{ CLIENT_ARMF_UINT, _client_armf },
			{ CLIENT_LEGF_UINT, _client_legf },
			{ CLIENT_CRTF_UINT, _client_crtf },
			{ CLIENT_PUPS_UINT, _client_pups },
			{ CLIENT_VKIL_UINT, _client_vkil },
			{ CLIENT_VTIM_UINT, _client_vtim },
			{ CLIENT_NKFV_UINT, _client_nkfv },
			{ CLIENT_SQUI_UINT, _client_squi },
			{ CLIENT_PCRD_UINT, _client_pcrd },
			{ CLIENT_BKIL_UINT, _client_bkil },
			{ CLIENT_HEDR_UINT, _client_hedr },
			{ CLIENT_TORR_UINT, _client_torr },
			{ CLIENT_ARMR_UINT, _client_armr },
			{ CLIENT_LEGR_UINT, _client_legr },
			{ CLIENT_CRTR_UINT, _client_crtr },
			{ CLIENT_FLGC_UINT, _client_flgc },

			/* Player related tags */
			{ CLIENT_NAM0_UINT, _client_nam0 },
			{ CLIENT_NAM1_UINT, _client_nam1 },
			{ CLIENT_NAM2_UINT, _client_nam2 },
			{ CLIENT_NAM3_UINT, _client_nam3 },
			{ CLIENT_NAM4_UINT, _client_nam4 },
			{ CLIENT_NAM5_UINT, _client_nam5 },
			{ CLIENT_NAM6_UINT, _client_nam6 },
			{ CLIENT_NAM7_UINT, _client_nam7 },

			{ CLIENT_IPA0_UINT, _client_ipa0 },
			{ CLIENT_IPA1_UINT, _client_ipa1 },
			{ CLIENT_IPA2_UINT, _client_ipa2 },
			{ CLIENT_IPA3_UINT, _client_ipa3 },
			{ CLIENT_IPA4_UINT, _client_ipa4 },
			{ CLIENT_IPA5_UINT, _client_ipa5 },
			{ CLIENT_IPA6_UINT, _client_ipa6 },
			{ CLIENT_IPA7_UINT, _client_ipa7 },

			{ CLIENT_CID0_UINT, _client_cid0 },
			{ CLIENT_CID1_UINT, _client_cid1 },
			{ CLIENT_CID2_UINT, _client_cid2 },
			{ CLIENT_CID3_UINT, _client_cid3 },
			{ CLIENT_CID4_UINT, _client_cid4 },
			{ CLIENT_CID5_UINT, _client_cid5 },
			{ CLIENT_CID6_UINT, _client_cid6 },
			{ CLIENT_CID7_UINT, _client_cid7 },

			{ CLIENT_SID0_UINT, _client_sid0 },
			{ CLIENT_SID1_UINT, _client_sid1 },
			{ CLIENT_SID2_UINT, _client_sid2 },
			{ CLIENT_SID3_UINT, _client_sid3 },
			{ CLIENT_SID4_UINT, _client_sid4 },
			{ CLIENT_SID5_UINT, _client_sid5 },
			{ CLIENT_SID6_UINT, _client_sid6 },
			{ CLIENT_SID7_UINT, _client_sid7 },

			{ CLIENT_TID0_UINT, _client_tid0 },
			{ CLIENT_TID1_UINT, _client_tid1 },
			{ CLIENT_TID2_UINT, _client_tid2 },
			{ CLIENT_TID3_UINT, _client_tid3 },
			{ CLIENT_TID4_UINT, _client_tid4 },
			{ CLIENT_TID5_UINT, _client_tid5 },
			{ CLIENT_TID6_UINT, _client_tid6 },
			{ CLIENT_TID7_UINT, _client_tid7 },

			{ CLIENT_CMP0_UINT, _client_cmp0 },
			{ CLIENT_CMP1_UINT, _client_cmp1 },
			{ CLIENT_CMP2_UINT, _client_cmp2 },
			{ CLIENT_CMP3_UINT, _client_cmp3 },
			{ CLIENT_CMP4_UINT, _client_cmp4 },
			{ CLIENT_CMP5_UINT, _client_cmp5 },
			{ CLIENT_CMP6_UINT, _client_cmp6 },
			{ CLIENT_CMP7_UINT, _client_cmp7 },

			{ CLIENT_COL0_UINT, _client_col0 },
			{ CLIENT_COL1_UINT, _client_col1 },
			{ CLIENT_COL2_UINT, _client_col2 },
			{ CLIENT_COL3_UINT, _client_col3 },
			{ CLIENT_COL4_UINT, _client_col4 },
			{ CLIENT_COL5_UINT, _client_col5 },
			{ CLIENT_COL6_UINT, _client_col6 },
			{ CLIENT_COL7_UINT, _client_col7 },

			{ CLIENT_CRD0_UINT, _client_crd0 },
			{ CLIENT_CRD1_UINT, _client_crd1 },
			{ CLIENT_CRD2_UINT, _client_crd2 },
			{ CLIENT_CRD3_UINT, _client_crd3 },
			{ CLIENT_CRD4_UINT, _client_crd4 },
			{ CLIENT_CRD5_UINT, _client_crd5 },
			{ CLIENT_CRD6_UINT, _client_crd6 },
			{ CLIENT_CRD7_UINT, _client_crd7 },

			{ CLIENT_INB0_UINT, _client_inb0 },
			{ CLIENT_INB1_UINT, _client_inb1 },
			{ CLIENT_INB2_UINT, _client_inb2 },
			{ CLIENT_INB3_UINT, _client_inb3 },
			{ CLIENT_INB4_UINT, _client_inb4 },
			{ CLIENT_INB5_UINT, _client_inb5 },
			{ CLIENT_INB6_UINT, _client_inb6 },
			{ CLIENT_INB7_UINT, _client_inb7 },

			{ CLIENT_UNB0_UINT, _client_unb0 },
			{ CLIENT_UNB1_UINT, _client_unb1 },
			{ CLIENT_UNB2_UINT, _client_unb2 },
			{ CLIENT_UNB3_UINT, _client_unb3 },
			{ CLIENT_UNB4_UINT, _client_unb4 },
			{ CLIENT_UNB5_UINT, _client_unb5 },
			{ CLIENT_UNB6_UINT, _client_unb6 },
			{ CLIENT_UNB7_UINT, _client_unb7 },

			{ CLIENT_PLB0_UINT, _client_plb0 },
			{ CLIENT_PLB1_UINT, _client_plb1 },
			{ CLIENT_PLB2_UINT, _client_plb2 },
			{ CLIENT_PLB3_UINT, _client_plb3 },
			{ CLIENT_PLB4_UINT, _client_plb4 },
			{ CLIENT_PLB5_UINT, _client_plb5 },
			{ CLIENT_PLB6_UINT, _client_plb6 },
			{ CLIENT_PLB7_UINT, _client_plb7 },

			{ CLIENT_BLB0_UINT, _client_blb0 },
			{ CLIENT_BLB1_UINT, _client_blb1 },
			{ CLIENT_BLB2_UINT, _client_blb2 },
			{ CLIENT_BLB3_UINT, _client_blb3 },
			{ CLIENT_BLB4_UINT, _client_blb4 },
			{ CLIENT_BLB5_UINT, _client_blb5 },
			{ CLIENT_BLB6_UINT, _client_blb6 },
			{ CLIENT_BLB7_UINT, _client_blb7 },

			{ CLIENT_INL0_UINT, _client_inl0 },
			{ CLIENT_INL1_UINT, _client_inl1 },
			{ CLIENT_INL2_UINT, _client_inl2 },
			{ CLIENT_INL3_UINT, _client_inl3 },
			{ CLIENT_INL4_UINT, _client_inl4 },
			{ CLIENT_INL5_UINT, _client_inl5 },
			{ CLIENT_INL6_UINT, _client_inl6 },
			{ CLIENT_INL7_UINT, _client_inl7 },

			{ CLIENT_UNL0_UINT, _client_unl0 },
			{ CLIENT_UNL1_UINT, _client_unl1 },
			{ CLIENT_UNL2_UINT, _client_unl2 },
			{ CLIENT_UNL3_UINT, _client_unl3 },
			{ CLIENT_UNL4_UINT, _client_unl4 },
			{ CLIENT_UNL5_UINT, _client_unl5 },
			{ CLIENT_UNL6_UINT, _client_unl6 },
			{ CLIENT_UNL7_UINT, _client_unl7 },

			{ CLIENT_PLL0_UINT, _client_pll0 },
			{ CLIENT_PLL1_UINT, _client_pll1 },
			{ CLIENT_PLL2_UINT, _client_pll2 },
			{ CLIENT_PLL3_UINT, _client_pll3 },
			{ CLIENT_PLL4_UINT, _client_pll4 },
			{ CLIENT_PLL5_UINT, _client_pll5 },
			{ CLIENT_PLL6_UINT, _client_pll6 },
			{ CLIENT_PLL7_UINT, _client_pll7 },

			{ CLIENT_BLL0_UINT, _client_bll0 },
			{ CLIENT_BLL1_UINT, _client_bll1 },
			{ CLIENT_BLL2_UINT, _client_bll2 },
			{ CLIENT_BLL3_UINT, _client_bll3 },
			{ CLIENT_BLL4_UINT, _client_bll4 },
			{ CLIENT_BLL5_UINT, _client_bll5 },
			{ CLIENT_BLL6_UINT, _client_bll6 },
			{ CLIENT_BLL7_UINT, _client_bll7 },

			{ CLIENT_INK0_UINT, _client_ink0 },
			{ CLIENT_INK1_UINT, _client_ink1 },
			{ CLIENT_INK2_UINT, _client_ink2 },
			{ CLIENT_INK3_UINT, _client_ink3 },
			{ CLIENT_INK4_UINT, _client_ink4 },
			{ CLIENT_INK5_UINT, _client_ink5 },
			{ CLIENT_INK6_UINT, _client_ink6 },
			{ CLIENT_INK7_UINT, _client_ink7 },

			{ CLIENT_UNK0_UINT, _client_unk0 },
			{ CLIENT_UNK1_UINT, _client_unk1 },
			{ CLIENT_UNK2_UINT, _client_unk2 },
			{ CLIENT_UNK3_UINT, _client_unk3 },
			{ CLIENT_UNK4_UINT, _client_unk4 },
			{ CLIENT_UNK5_UINT, _client_unk5 },
			{ CLIENT_UNK6_UINT, _client_unk6 },
			{ CLIENT_UNK7_UINT, _client_unk7 },

			{ CLIENT_PLK0_UINT, _client_plk0 },
			{ CLIENT_PLK1_UINT, _client_plk1 },
			{ CLIENT_PLK2_UINT, _client_plk2 },
			{ CLIENT_PLK3_UINT, _client_plk3 },
			{ CLIENT_PLK4_UINT, _client_plk4 },
			{ CLIENT_PLK5_UINT, _client_plk5 },
			{ CLIENT_PLK6_UINT, _client_plk6 },
			{ CLIENT_PLK7_UINT, _client_plk7 },

			{ CLIENT_BLK0_UINT, _client_blk0 },
			{ CLIENT_BLK1_UINT, _client_blk1 },
			{ CLIENT_BLK2_UINT, _client_blk2 },
			{ CLIENT_BLK3_UINT, _client_blk3 },
			{ CLIENT_BLK4_UINT, _client_blk4 },
			{ CLIENT_BLK5_UINT, _client_blk5 },
			{ CLIENT_BLK6_UINT, _client_blk6 },
			{ CLIENT_BLK7_UINT, _client_blk7 },

			{ CLIENT_BLC0_UINT, _client_blc0 },
			{ CLIENT_BLC1_UINT, _client_blc1 },
			{ CLIENT_BLC2_UINT, _client_blc2 },
			{ CLIENT_BLC3_UINT, _client_blc3 },
			{ CLIENT_BLC4_UINT, _client_blc4 },
			{ CLIENT_BLC5_UINT, _client_blc5 },
			{ CLIENT_BLC6_UINT, _client_blc6 },
			{ CLIENT_BLC7_UINT, _client_blc7 },

			{ 1, NULL }
		};

		static int handle_wolgameres_tag(t_wol_gameres_result * game_result, t_tag gamerestag, wol_gameres_type type, int size, void const * data)
		{
			t_wol_gamerestag_table_row const *p;

			for (p = wol_gamreres_htable; p->wol_gamerestag_uint != 1; p++) {
				if (gamerestag == p->wol_gamerestag_uint) {
					if (p->wol_gamerestag_handler != NULL)
						return ((p->wol_gamerestag_handler)(game_result, type, size, data));
				}
			}
			return -1;
		}

		static wol_gameres_type wol_gameres_type_from_int(int type)
		{
			switch (type) {
			case DATA_TYPE_BYTE:
				return wol_gameres_type_byte;
			case DATA_TYPE_BOOL:
				return wol_gameres_type_bool;
			case DATA_TYPE_TIME:
				return wol_gameres_type_time;
			case DATA_TYPE_INT:
				return wol_gameres_type_int;
			case DATA_TYPE_STRING:
				return wol_gameres_type_string;
			case DATA_TYPE_BIGINT:
				return wol_gameres_type_bigint;
			default:
				WARN1("type %d is not defined", type);
				return wol_gameres_type_unknown;
			}
		}

		static unsigned long wol_gameres_get_long_from_data(int size, const void * data)
		{
			unsigned int temp = 0;
			const char * chdata;
			int i = 0;

			if (!data) {
				ERROR0("got NULL data");
				return 0;
			}

			if (size < 4)
				return 0;

			chdata = (const char*)data;

			while (i < size) {
				temp = temp + (unsigned int)bn_int_nget(*((bn_int *)chdata));
				i += 4;
				if (i < size) {
					*chdata++;
					*chdata++;
					*chdata++;
					*chdata++;
				}
			}

			return temp;
		}

		static t_wol_gameres_result * gameres_result_create()
		{
			t_wol_gameres_result * gameres_result;

			gameres_result = (t_wol_gameres_result*)xmalloc(sizeof(t_wol_gameres_result));

			gameres_result->game = NULL;
			gameres_result->results = NULL;
			gameres_result->senderid = -1;
			gameres_result->myaccount = NULL;

			return gameres_result;
		}

		static int gameres_result_destroy(t_wol_gameres_result * gameres_result)
		{
			if (!gameres_result) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL gameres_result");
				return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "destroying gameres_result");

			xfree(gameres_result);

			return 0;
		}

		extern int handle_wol_gameres_packet(t_connection * c, t_packet const * const packet)
		{
			unsigned offset;
			t_tag wgtag;
			char wgtag_str[5];
			unsigned datatype;
			unsigned datalen;
			void const * data;
			wol_gameres_type type;
			t_wol_gameres_result * gameres_result;
			unsigned int i;

			DEBUG2("[%d] got WOL Gameres packet length %u", conn_get_socket(c), packet_get_size(packet));
			offset = sizeof(t_wolgameres_header);

			if ((unsigned int)bn_int_nget(*((bn_int *)packet_get_data_const(packet, offset, 4))) == 0)
				offset += 4; /* Just trying to get RNGD working */

			gameres_result = gameres_result_create();

			while (offset < packet_get_size(packet)) {
				if (packet_get_size(packet) < offset + 4) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad WOL Gameres packet (missing tag)", conn_get_socket(c));
					return -1;
				}
				wgtag = (unsigned int)bn_int_nget(*((bn_int *)packet_get_data_const(packet, offset, 4)));
				offset += 4;

				if (packet_get_size(packet) < offset + 2) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad WOL Gameres packet (missing datatype)", conn_get_socket(c));
					return -1;
				}
				datatype = (unsigned int)bn_short_nget(*((bn_short *)packet_get_data_const(packet, offset, 2)));
				offset += 2;

				if (packet_get_size(packet) < offset + 2) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad WOL Gameres packet (missing datalen)", conn_get_socket(c));
					return -1;
				}
				datalen = (unsigned int)bn_short_nget(*((bn_short *)packet_get_data_const(packet, offset, 2)));
				offset += 2;

				if (packet_get_size(packet) < offset + 1) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad WOL Gameres packet (missing data)", conn_get_socket(c));
					return -1;
				}
				data = packet_get_data_const(packet, offset, 1);

				type = wol_gameres_type_from_int(datatype);

				if (handle_wolgameres_tag(gameres_result, wgtag, type, datalen, data) != 0) {
					char ch_data[255]; /* FIXME: this is not so good */
					tag_uint_to_str(wgtag_str, wgtag);

					switch (type) {
					case wol_gameres_type_bool:
					case wol_gameres_type_byte:
						snprintf(ch_data, sizeof(ch_data), "%u", (unsigned int)bn_byte_get(*((bn_byte *)data)));
						break;
					case wol_gameres_type_int:
					case wol_gameres_type_time:
						snprintf(ch_data, sizeof(ch_data), "%u", (unsigned int)bn_int_nget(*((bn_int *)data)));
						break;
					case wol_gameres_type_string:
						snprintf(ch_data, sizeof(ch_data), "%s", (char *)data);
						break;
					case wol_gameres_type_bigint:
						snprintf(ch_data, sizeof(ch_data), "%u", wol_gameres_get_long_from_data(datalen, data));
						break;
					default:
						snprintf(ch_data, sizeof(ch_data), "UNKNOWN");
						break;
					}
					eventlog(eventlog_level_warn, __FUNCTION__, "[%d] got unknown WOL Gameres tag: %s, data type %u, data lent %u, data %s", conn_get_socket(c), wgtag_str, datatype, datalen, ch_data);

				}

				datalen = 4 * ((datalen + 3) / 4);
				offset += datalen;
			}

			if (!(gameres_result->game)) {
				ERROR0("game not found (game == NULL)");
				return -1;
			}
			if (!(gameres_result->myaccount)) {
				ERROR0("have not account of sender");
				return -1;
			}
			if (!(gameres_result->results)) {
				ERROR0("have not results of game");
				return -1;
			}

			game_set_report(gameres_result->game, gameres_result->myaccount, "head", "body");

			if (game_set_reported_results(gameres_result->game, gameres_result->myaccount, gameres_result->results) < 0)
				xfree((void *)gameres_result->results);

			conn_set_game(account_get_conn(gameres_result->myaccount), NULL, NULL, NULL, game_type_none, 0);

			gameres_result_destroy(gameres_result);

			return 0;
		}

		/**
		 * WOL Gamreres tag handlers
		 */

		static int _client_sern(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* SER# */
			return 0;
		}

		static int _client_sdfx(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			int sdfx;

			switch (type) {
			case wol_gameres_type_bool:
				sdfx = (unsigned int)bn_byte_get(*((bn_byte *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for SDFX", type);
				break;
			}

			if (sdfx)
				DEBUG0("SDFX == true");
			else
				DEBUG0("SDFX == false");

			return 0;
		}

		static int _client_idno(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int gameidnumber = 0;
			t_game * game;

			switch (type) {
			case wol_gameres_type_int:
			case wol_gameres_type_time:
				gameidnumber = (unsigned int)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for IDNO", type);
				break;
			}

			if ((gameidnumber) && (game = gamelist_find_game_byid(gameidnumber))) { //&& (game_get_status(game) & game_status_started)) {
				DEBUG2("found started game \"%s\" for gameid %u", game_get_name(game), gameidnumber);
				game_result->game = game;
				game_result->results = (t_game_result*)xmalloc(sizeof(t_game_result)* game_get_count(game));
			}

			return 0;
		}

		static int _client_gsku(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int sku;
			const t_game * game = game_result->game;

			switch (type) {
			case wol_gameres_type_int:
				sku = (unsigned int)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for GSKU", type);
				break;
			}

			if ((sku) && (game) && (game_get_clienttag(game) == tag_sku_to_uint(sku))) {
				DEBUG1("got proper WOL game resolution of %s", clienttag_get_title(tag_sku_to_uint(sku)));
			}


			return 0;
		}

		static int _client_dcon(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int dcon;

			switch (type) {
			case wol_gameres_type_int:
				dcon = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("DCON is: %u", dcon);
				break;
			default:
				WARN1("got unknown gameres type %u for DCON", type);
				break;
			}

			return 0;
		}

		static int _client_lcon(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int lcon;

			switch (type) {
			case wol_gameres_type_int:
				lcon = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("LCON is: %u", lcon);
				break;
			default:
				WARN1("got unknown gameres type %u for LCON", type);
				break;
			}

			return 0;
		}

		static int _client_type(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			int gametype;

			switch (type) {
			case wol_gameres_type_bool:
				gametype = (unsigned int)bn_byte_get(*((bn_byte *)data));
				DEBUG1("TYPE is: %u", gametype);
				break;
			default:
				WARN1("got unknown gameres type %u for TYPE", type);
				break;
			}

			return 0;
		}

		static int _client_trny(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool tournament;

			switch (type) {
			case wol_gameres_type_bool:
				tournament = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_int:
				tournament = (bool)bn_int_nget(*((bn_int *)data));
				break;
			case wol_gameres_type_string:
				if (std::strcmp("TY  ", (const char *)data) == 0) /* for RNGD */
					tournament = true;
				else if (std::strcmp("TN  ", (const char *)data) == 0) /* for RNGD */
					tournament = false;
				else {
					WARN1("got unknown string for TRNY: %s", (char *)data);
					tournament = false;
				}
				break;
			default:
				WARN1("got unknown gameres type %u for TRNY", type);
				break;
			}

			if (tournament)
				DEBUG0("game was tournament");
			else
				DEBUG0("game was not tournament");

			return 0;
		}

		static int _client_oosy(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool outofsync;

			switch (type) {
			case wol_gameres_type_bool:
				outofsync = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for OOSY", type);
				break;
			}

			if (outofsync)
				DEBUG0("Game has Out of Sync YES");
			else
				DEBUG0("Game has Out of Sync NO");

			return 0;
		}

		static int _client_fini(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* Not implemented yet */
			return 0;
		}

		static int _client_dura(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int duration;

			/* Game dureation in seconds??? */
			// DURA : Game Duration
			// DURA 00 06 00 04 [00 00 01 F4]
			// DURA 00 06 00 04 [00 00 00 06]

			switch (type) {
			case wol_gameres_type_int:
			case wol_gameres_type_time:
				duration = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game duration %u seconds", duration);
				break;
			default:
				WARN1("got unknown gameres type %u for DURA", type);
				break;
			}

			return 0;
		}

		static int _client_cred(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int credits;

			switch (type) {
			case wol_gameres_type_int:
				credits = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game was start with %u credits", credits);
				break;
			default:
				WARN1("got unknown gameres type %u for CRED", type);
				break;
			}

			return 0;
		}

		static int _client_shrt(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool shortgame;

			switch (type) {
			case wol_gameres_type_bool:
				shortgame = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_int:
				shortgame = (bool)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for SHRT", type);
				break;
			}

			if (shortgame)
				DEBUG0("Game has shortgame ON");
			else
				DEBUG0("Game has shortgame OFF");

			return 0;
		}

		static int _client_supr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool superweapons;

			switch (type) {
			case wol_gameres_type_bool:
				superweapons = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_int:
				superweapons = (bool)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for SUPR", type);
				break;
			}

			if (superweapons)
				DEBUG0("Game has superweapons ON");
			else
				DEBUG0("Game has superweapons OFF");

			return 0;
		}

		static int _client_mode(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */

			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Game mode: %s", (char *)data); /* For RNGD */
				break;
			default:
				WARN1("got unknown gameres type %u for MODE", type);
				break;
			}

			return 0;
		}

		static int _client_bamr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* Not implemented yet */
			return 0;
		}

		static int _client_crat(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool crates;

			switch (type) {
			case wol_gameres_type_bool:
				crates = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_int:
				crates = (bool)bn_int_nget(*((bn_int *)data));
				break;
			case wol_gameres_type_string:
				if (std::strcmp("ON", (const char *)data) == 0)
					crates = true;
				else if (std::strcmp("OFF", (const char *)data) == 0)
					crates = false;
				else {
					WARN1("got unknown string for CRAT: %s", (char *)data);
					crates = false;
				}
				break;
			default:
				WARN1("got unknown gameres type %u for CRAT", type);
				break;
			}

			if (crates)
				DEBUG0("Game has crates ON");
			else
				DEBUG0("Game has crates OFF");

			return 0;
		}

		static int _client_aipl(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int aicount;

			if (type == wol_gameres_type_int) {
				aicount = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game has %u AI players", aicount);
			}
			else {
				WARN1("got unknown gameres type %u for AIPL", type);
				return 0;
			}

			return 0;
		}

		static int _client_unit(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int units;

			switch (type) {
			case wol_gameres_type_int:
				units = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game was started with %u units", units);
				break;
			default:
				WARN1("got unknown gameres type %u for UNIT", type);
				break;
			}

			return 0;
		}

		static int _client_scen(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Secnario %s", (char *)data);
				break;
			case wol_gameres_type_int:
				DEBUG1("Secnario num %u", (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			default:
				WARN1("got unknown gameres type %u for SCEN", type);
				break;
			}

			return 0;
		}

		static int _client_cmpl(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			int cmpl;

			switch (type) {
			case wol_gameres_type_byte:
				cmpl = (unsigned int)bn_byte_get(*((bn_byte *)data));
				DEBUG1("CMPL is: %u", cmpl);
				break;
			default:
				WARN1("got unknown gameres type %u for CMPL", type);
				break;
			}

			return 0;
		}

		static int _client_pngs(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int pngs;

			switch (type) {
			case wol_gameres_type_int:
				pngs = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("PNGS %u ", pngs);
				break;
			default:
				WARN1("got unknown gameres type %u for PNGS", type);
				break;
			}
			return 0;
		}

		static int _client_pngr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int pngr;

			switch (type) {
			case wol_gameres_type_int:
				pngr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("PNGR %u ", pngr);
				break;
			default:
				WARN1("got unknown gameres type %u for PNGR", type);
				break;
			}
			return 0;
		}

		static int _client_plrs(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int playercount = 0;

			switch (type) {
			case wol_gameres_type_int:
			case wol_gameres_type_time:
				playercount = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Player count %u ", playercount);
				break;
			default:
				WARN1("got unknown gameres type %u for PLRS", type);
				break;
			}
			return 0;
		}

		static int _client_spid(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			int spid = -1;

			switch (type) {
			case wol_gameres_type_int:
				spid = (int)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN1("got unknown gameres type %u for SPID", type);
				break;
			}

			if (spid != -1) {
				DEBUG1("Setting sender ID to %u", spid);
				game_result->senderid = spid;
			}

			return 0;
		}

		static int _client_time(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* PELISH: Time when was game started. This time is WOL STARTG packet time +-2 sec */
			std::time_t time;

			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Game was start at %s ", (char *)data);
				break;
			case wol_gameres_type_time:
				time = (std::time_t) bn_int_nget(*((bn_int *)data));
				DEBUG1("Game was start at %s ", ctime(&time));
				break;
			default:
				WARN1("got unknown gameres type %u for TIME", type);
				break;
			}
			return 0;
		}

		static int _client_afps(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int afps;

			switch (type) {
			case wol_gameres_type_int:
				afps = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Average Frames per Second %u ", afps);
				break;
			default:
				WARN1("got unknown gameres type %u for AFPS", type);
				break;
			}
			return 0;
		}

		static int _client_proc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Procesor %s", (char *)data);
				break;
			default:
				WARN1("got unknown gameres type %u for PROC", type);
				break;
			}
			return 0;
		}

		static int _client_memo(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* Not implemented yet */
			return 0;
		}

		static int _client_vidm(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* Not implemented yet */
			return 0;
		}

		static int _client_sped(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int speed;

			//SPED 00 01 00 01 [00 00 00 00]

			if (type == wol_gameres_type_byte) {
				speed = (unsigned int)bn_byte_get(*((bn_byte *)data));
			}
			else if (type == wol_gameres_type_int) {
				speed = (unsigned int)bn_int_nget(*((bn_int *)data));
			}
			else {
				WARN1("got unknown gameres type %u for SPED", type);
				return 0;
			}

			DEBUG1("Game speed %u", speed);
			return 0;
		}

		static int _client_vers(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int version;

			if (type == wol_gameres_type_string) {
				DEBUG1("Version of client %s", (char *)data);
			}
			else if (type == wol_gameres_type_int) {
				version = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Version of client %u", version);
			}
			else
				WARN1("got unknown gameres type %u for VERS", type);

			return 0;
		}

		static int _client_date(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			//WOLv1: DATE 00 14 00 08  52 11 ea 00  01 c5 cd a9

			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Date %s", (char *)data);
				break;
			default:
				WARN1("got unknown gameres type %u for DATE", type);
				break;
			}
			return 0;
		}

		static int _client_base(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool bases;

			//WOLv1 BASE 00 07 00 03 ON

			switch (type) {
			case wol_gameres_type_bool:
				bases = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_string:
				if (std::strcmp("ON", (const char *)data) == 0)
					bases = true;
				else if (std::strcmp("OFF", (const char *)data) == 0)
					bases = false;
				else {
					WARN1("got unknown string for BASE: %s", (char *)data);
					bases = false;
				}
				break;
			default:
				WARN1("got unknown gameres type %u for BASE", type);
				break;
			}

			if (bases)
				DEBUG0("Game has bases ON");
			else
				DEBUG0("Game has bases OFF");

			return 0;
		}

		static int _client_tibr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool tiberium;

			if (type == wol_gameres_type_bool) {
				tiberium = (bool)bn_byte_get(*((bn_byte *)data));
				if (tiberium)
					DEBUG0("Game has tiberium ON");
				else
					DEBUG0("Game has tiberium OFF");
			}
			else if (type == wol_gameres_type_string) {
				DEBUG1("Game has tiberium %s", (char *)data);
			}
			else
				WARN1("got unknown gameres type %u for TIBR", type);

			return 0;
		}

		static int _client_shad(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool shadows;

			switch (type) {
			case wol_gameres_type_bool:
				shadows = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_string:
				if (std::strcmp("ON", (const char *)data) == 0)
					shadows = true;
				else if (std::strcmp("OFF", (const char *)data) == 0)
					shadows = false;
				else {
					WARN1("got unknown string for SHAD: %s", (char *)data);
					shadows = false;
				}
				break;
			default:
				WARN1("got unknown gameres type %u for SHAD", type);
				break;
			}

			if (shadows)
				DEBUG0("Game has shadows ON");
			else
				DEBUG0("Game has shadows OFF");

			return 0;
		}

		static int _client_flag(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			bool captureflag;

			switch (type) {
			case wol_gameres_type_bool:
				captureflag = (bool)bn_byte_get(*((bn_byte *)data));
				break;
			case wol_gameres_type_string:
				if (std::strcmp("ON", (const char *)data) == 0)
					captureflag = true;
				else if (std::strcmp("OFF", (const char *)data) == 0)
					captureflag = false;
				else {
					WARN1("got unknown string for FLAG: %s", (char *)data);
					captureflag = false;
				}
				break;
			default:
				WARN1("got unknown gameres type %u for FLAG", type);
				break;
			}

			if (captureflag)
				DEBUG0("Game has capture the flag ON");
			else
				DEBUG0("Game has capture the flag OFF");

			return 0;

		}

		static int _client_tech(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int techlevel;

			if (type == wol_gameres_type_int) {
				techlevel = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game has techlevel %u", techlevel);
			}
			else
				WARN1("got unknown gameres type %u for TECH", type);

			return 0;
		}

		static int _client_brok(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int brok;

			/* This tag sends YURI */
			//BROK 00 06 00 04 00 00 00 02

			if (type == wol_gameres_type_int) {
				brok = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game has BROK %u", brok);
			}
			else
				WARN1("got unknown gameres type %u for BROK", type);

			return 0;
		}

		static int _client_acco(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int acco;

			/* This tag sends YURI */

			switch (type) {
			case wol_gameres_type_int:
				acco = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Game has ACCO %u", acco);
				break;
			default:
				WARN1("got unknown gameres type %u for ACCO", type);
				break;
			}

			return 0;
		}

		static int _client_etim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */

			/* Elapsed time??? this tag got unknown data type 0x0014 */

			return 0;
		}

		static int _client_pspd(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */
			unsigned int procesorspeed;

			switch (type) {
			case wol_gameres_type_int:
				procesorspeed = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Server have processor speed: %u MHz", procesorspeed);
				break;
			default:
				WARN1("got unknown gameres type %u for PSPD", type);
				break;
			}
			return 0;
		}

		static int _client_smem(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */
			unsigned int servermemory;

			switch (type) {
			case wol_gameres_type_int:
				servermemory = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Server have %u memory", servermemory);
				break;
			default:
				WARN1("got unknown gameres type %u for SMEM", type);
				break;
			}
			return 0;
		}

		static int _client_svid(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */

			/* Server ID ??? this tag got unknown data type 0x0014 */

			return 0;
		}

		static int _client_snam(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */

			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Server name: %s", (char *)data);
				break;
			default:
				WARN1("got unknown gameres type %u for SNAM", type);
				break;
			}
			return 0;
		}

		static int _client_gmap(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */

			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Game map: %s", (char *)data);
				break;
			default:
				WARN1("got unknown gameres type %u for GMAP", type);
				break;
			}
			return 0;
		}

		static int _client_dsvr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			/* This tag sends RNGD only */
			unsigned int dsvr;

			switch (type) {
			case wol_gameres_type_byte:
				dsvr = (unsigned int)bn_byte_get(*((bn_byte *)data));
				DEBUG1("DSVR: %u", dsvr);
				break;
			default:
				WARN1("got unknown gameres type %u for SCEN", type);
				break;
			}
			return 0;
		}


		/* Renegade player tag handlers */
		static int _client_pnam(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_string:
				DEBUG1("Player name: %s", (char *)data);
				break;
			default:
				WARN1("got unknown gameres type %u for PNAM", type);
				break;
			}
			return 0;
		}

		static int _client_ploc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int playerloc;

			switch (type) {
			case wol_gameres_type_int:
				playerloc = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Player locale: %u", playerloc);
				break;
			default:
				WARN1("got unknown gameres type %u for PLOC", type);
				break;
			}
			return 0;
		}

		static int _client_team(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int team;

			switch (type) {
			case wol_gameres_type_int:
				team = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Team: %u", team);
				break;
			default:
				WARN1("got unknown gameres type %u for TEAM", type);
				break;
			}
			return 0;
		}

		static int _client_pscr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int score;

			switch (type) {
			case wol_gameres_type_int:
				score = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Players score: %u", score);
				break;
			default:
				WARN1("got unknown gameres type %u for PSCR", type);
				break;
			}
			return 0;
		}

		static int _client_ppts(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int points;

			switch (type) {
			case wol_gameres_type_time: /* PELISH: THIS is not a BUG! This is Renegade developers bug */
				points = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Points %u", points);
				break;
			default:
				WARN1("got unknown gameres type %u for PPTS", type);
				break;
			}
			return 0;
		}

		static int _client_ptim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int playertime;

			switch (type) {
			case wol_gameres_type_int:
				playertime = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Player game time %u", playertime);
				break;
			default:
				WARN1("got unknown gameres type %u for PTIM", type);
				break;
			}
			return 0;
		}

		static int _client_phlt(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int health;

			switch (type) {
			case wol_gameres_type_int:
				health = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Health: %u", health);
				break;
			default:
				WARN1("got unknown gameres type %u for PHLT", type);
				break;
			}
			return 0;
		}

		static int _client_pkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int kills;

			switch (type) {
			case wol_gameres_type_int:
				kills = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Kills: %u", kills);
				break;
			default:
				WARN1("got unknown gameres type %u for PKIL", type);
				break;
			}
			return 0;
		}

		static int _client_ekil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int enemy_kills;

			switch (type) {
			case wol_gameres_type_int:
				enemy_kills = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Enemy kills: %u", enemy_kills);
				break;
			default:
				WARN1("got unknown gameres type %u for EKIL", type);
				break;
			}
			return 0;
		}

		static int _client_akil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int ally_kills;

			switch (type) {
			case wol_gameres_type_int:
				ally_kills = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Ally kills %u", ally_kills);
				break;
			default:
				WARN1("got unknown gameres type %u for AKIL", type);
				break;
			}
			return 0;
		}

		static int _client_shot(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int shots;

			switch (type) {
			case wol_gameres_type_int:
				shots = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Shots %u", shots);
				break;
			default:
				WARN1("got unknown gameres type %u for SHOT", type);
				break;
			}
			return 0;
		}

		static int _client_hedf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int hedf;

			switch (type) {
			case wol_gameres_type_int:
				hedf = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("hedf %u", hedf);
				break;
			default:
				WARN1("got unknown gameres type %u for HEDF", type);
				break;
			}
			return 0;
		}

		static int _client_torf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int torf;

			switch (type) {
			case wol_gameres_type_int:
				torf = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("torf %u", torf);
				break;
			default:
				WARN1("got unknown gameres type %u for TORF", type);
				break;
			}
			return 0;
		}

		static int _client_armf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int armf;

			switch (type) {
			case wol_gameres_type_int:
				armf = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("armf %u", armf);
				break;
			default:
				WARN1("got unknown gameres type %u for ARMF", type);
				break;
			}
			return 0;
		}

		static int _client_legf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int legf;

			switch (type) {
			case wol_gameres_type_int:
				legf = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("legf %u", legf);
				break;
			default:
				WARN1("got unknown gameres type %u for LEGF", type);
				break;
			}
			return 0;
		}

		static int _client_crtf(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int crtf;

			switch (type) {
			case wol_gameres_type_int:
				crtf = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("crtf %u", crtf);
				break;
			default:
				WARN1("got unknown gameres type %u for CRTF", type);
				break;
			}
			return 0;
		}

		static int _client_pups(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int pups;

			switch (type) {
			case wol_gameres_type_int:
				pups = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("pups %u", pups);
				break;
			default:
				WARN1("got unknown gameres type %u for PUPS", type);
				break;
			}
			return 0;
		}

		static int _client_vkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int victory_kills;

			switch (type) {
			case wol_gameres_type_int:
				victory_kills = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Victory kills %u", victory_kills);
				break;
			default:
				WARN1("got unknown gameres type %u for VKIL", type);
				break;
			}
			return 0;
		}

		static int _client_vtim(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int victory_time;

			switch (type) {
			case wol_gameres_type_int:
				victory_time = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("Victory time %u", victory_time);
				break;
			default:
				WARN1("got unknown gameres type %u for VTIM", type);
				break;
			}
			return 0;
		}

		static int _client_nkfv(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int nkfv;

			switch (type) {
			case wol_gameres_type_int:
				nkfv = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("nkfv %u", nkfv);
				break;
			default:
				WARN1("got unknown gameres type %u for NKFV", type);
				break;
			}
			return 0;
		}

		static int _client_squi(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int squi;

			switch (type) {
			case wol_gameres_type_int:
				squi = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("squi %u", squi);
				break;
			default:
				WARN1("got unknown gameres type %u for SQUI", type);
				break;
			}
			return 0;
		}

		static int _client_pcrd(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int pcrd;

			switch (type) {
			case wol_gameres_type_int:
				pcrd = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("pcrd %u", pcrd);
				break;
			default:
				WARN1("got unknown gameres type %u for PCRD", type);
				break;
			}
			return 0;
		}

		static int _client_bkil(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int bkil;

			switch (type) {
			case wol_gameres_type_int:
				bkil = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("bkil %u", bkil);
				break;
			default:
				WARN1("got unknown gameres type %u for BKIL", type);
				break;
			}
			return 0;
		}

		static int _client_hedr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int hedr;

			switch (type) {
			case wol_gameres_type_int:
				hedr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("hedr %u", hedr);
				break;
			default:
				WARN1("got unknown gameres type %u for HEDR", type);
				break;
			}
			return 0;
		}

		static int _client_torr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int torr;

			switch (type) {
			case wol_gameres_type_int:
				torr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("torr %u", torr);
				break;
			default:
				WARN1("got unknown gameres type %u for TORR", type);
				break;
			}
			return 0;
		}

		static int _client_armr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int armr;

			switch (type) {
			case wol_gameres_type_int:
				armr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("armr %u", armr);
				break;
			default:
				WARN1("got unknown gameres type %u for ARMR", type);
				break;
			}
			return 0;
		}

		static int _client_legr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int legr;

			switch (type) {
			case wol_gameres_type_int:
				legr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("legr %u", legr);
				break;
			default:
				WARN1("got unknown gameres type %u for LEGR", type);
				break;
			}
			return 0;
		}

		static int _client_crtr(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int crtr;

			switch (type) {
			case wol_gameres_type_int:
				crtr = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("crtr %u", crtr);
				break;
			default:
				WARN1("got unknown gameres type %u for CRTR", type);
				break;
			}
			return 0;
		}

		static int _client_flgc(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			unsigned int flgc;

			switch (type) {
			case wol_gameres_type_int:
				flgc = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG1("flgc %u", flgc);
				break;
			default:
				WARN1("got unknown gameres type %u for FLGC", type);
				break;
			}
			return 0;
		}



		/* player related tag handlers */

		static int _cl_nam_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			int senderid = game_result->senderid;
			t_account * account = accountlist_find_account((char const *)data);

			DEBUG2("Name of palyer %u: %s", (char *)num, data);

			game_result->otheraccount = account;

			if (senderid == num) {
				DEBUG1("Packet was sent by %s", (char *)data);
				game_result->myaccount = account;
			}

			return 0;
		}

		static int _client_nam0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 0, type, size, data);
		}

		static int _client_nam1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 1, type, size, data);
		}

		static int _client_nam2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 2, type, size, data);
		}

		static int _client_nam3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 3, type, size, data);
		}

		static int _client_nam4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 4, type, size, data);
		}

		static int _client_nam5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 5, type, size, data);
		}

		static int _client_nam6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 6, type, size, data);
		}

		static int _client_nam7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_nam_general(game_result, 7, type, size, data);
		}

		static int _cl_ipa_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			unsigned int ipaddress;

			switch (type) {
			case wol_gameres_type_int:
				ipaddress = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG2("IP address of player%u: %u", num, ipaddress);
				break;
			default:
				WARN2("got unknown gameres type %u for IPA%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_ipa0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 0, type, size, data);
		}

		static int _client_ipa1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 1, type, size, data);
		}

		static int _client_ipa2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 2, type, size, data);
		}

		static int _client_ipa3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 3, type, size, data);
		}

		static int _client_ipa4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 4, type, size, data);
		}

		static int _client_ipa5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 5, type, size, data);
		}

		static int _client_ipa6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 6, type, size, data);
		}

		static int _client_ipa7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ipa_general(game_result, 7, type, size, data);
		}

		static int _cl_cid_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			unsigned int clanid;

			switch (type) {
			case wol_gameres_type_int:
				clanid = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG2("Clan ID of player%u: %u", num, clanid);
				break;
			default:
				WARN2("got unknown gameres type %u for CID%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_cid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 0, type, size, data);
		}

		static int _client_cid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 1, type, size, data);
		}

		static int _client_cid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 2, type, size, data);
		}

		static int _client_cid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 3, type, size, data);
		}

		static int _client_cid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 4, type, size, data);
		}

		static int _client_cid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 5, type, size, data);
		}

		static int _client_cid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 6, type, size, data);
		}

		static int _client_cid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cid_general(game_result, 7, type, size, data);
		}

		static int _cl_sid_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			unsigned int sideid;

			switch (type) {
			case wol_gameres_type_int:
				sideid = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG2("Side ID of player%u: %u", num, sideid);
				break;
			default:
				WARN2("got unknown gameres type %u for SID%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_sid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 0, type, size, data);
		}

		static int _client_sid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 1, type, size, data);
		}

		static int _client_sid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 2, type, size, data);
		}

		static int _client_sid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 3, type, size, data);
		}

		static int _client_sid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 4, type, size, data);
		}

		static int _client_sid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 5, type, size, data);
		}

		static int _client_sid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 6, type, size, data);
		}

		static int _client_sid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_sid_general(game_result, 7, type, size, data);
		}

		static int _cl_tid_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			unsigned int teamid;

			switch (type) {
			case wol_gameres_type_int:
				teamid = (unsigned int)bn_int_nget(*((bn_int *)data));
				DEBUG2("Team ID of player%u: %u", num, teamid);
				break;
			default:
				WARN2("got unknown gameres type %u for TID%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_tid0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 0, type, size, data);
		}

		static int _client_tid1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 1, type, size, data);
		}

		static int _client_tid2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 2, type, size, data);
		}

		static int _client_tid3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 3, type, size, data);
		}

		static int _client_tid4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 4, type, size, data);
		}

		static int _client_tid5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 5, type, size, data);
		}

		static int _client_tid6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 6, type, size, data);
		}

		static int _client_tid7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_tid_general(game_result, 7, type, size, data);
		}


		static int _cl_cmp_general(t_wol_gameres_result * game_result, int num, wol_gameres_type type, int size, void const * data)
		{
			//CMPx 00 06 00 04 00 00 02 00
			//0x0100=WIN 0x0200(0x0210)=LOSE 0x0300=DISCONECT

			t_account * other_account = game_result->otheraccount;
			t_game * game = game_result->game;
			t_game_result result;
			t_game_result * results = game_result->results;
			int resultnum;
			int i;

			switch (type) {
			case wol_gameres_type_int:
				resultnum = (unsigned int)bn_int_nget(*((bn_int *)data));
				break;
			default:
				WARN2("got unknown gameres type %u for CMP%u", type, num);
				return 0;
			}

			DEBUG2("Got %u player resultnum %u", num, resultnum);

			resultnum &= 0x0000FF00;
			resultnum = resultnum >> 8;

			if ((!game) || (!other_account)) {
				ERROR0("got corrupt gameres packet - game == NULL || other_account == NULL");
				return 0;
			}

			for (i = 0; i < game_get_count(game); i++) {
				if (game_get_player(game, i) == other_account) break;
			}

			switch (resultnum) {
			case 1:
				DEBUG0("WIN");
				result = game_result_win;
				break;
			case 2:
				DEBUG0("LOSS");
				result = game_result_loss;
				break;
			case 3:
				DEBUG0("DISCONECT");
				result = game_result_disconnect;
				break;
			default:
				DEBUG2("Got wrong %u player resultnum %u", num, resultnum);
				result = game_result_disconnect;
				break;
			}

			if (results) {
				results[i] = result;
				game_result->results = results;
				DEBUG0("game result was set");
			}

			return 0;
		}

		static int _client_cmp0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 0, type, size, data);
		}

		static int _client_cmp1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 1, type, size, data);
		}

		static int _client_cmp2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 2, type, size, data);
		}

		static int _client_cmp3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 3, type, size, data);
		}

		static int _client_cmp4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 4, type, size, data);
		}

		static int _client_cmp5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 5, type, size, data);
		}

		static int _client_cmp6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 6, type, size, data);
		}

		static int _client_cmp7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_cmp_general(game_result, 7, type, size, data);
		}

		static int _cl_col_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_int:
				DEBUG2("Player %u colour num: %u", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			default:
				WARN2("got unknown gameres type %u for COL%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_col0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(0, type, size, data);
		}

		static int _client_col1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(1, type, size, data);
		}

		static int _client_col2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(2, type, size, data);
		}

		static int _client_col3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(3, type, size, data);
		}

		static int _client_col4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(4, type, size, data);
		}

		static int _client_col5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(5, type, size, data);
		}

		static int _client_col6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(6, type, size, data);
		}

		static int _client_col7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_col_general(7, type, size, data);
		}

		static int _cl_crd_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u had %u credits on end of game", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			default:
				WARN2("got unknown gameres type %u for CRD%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_crd0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(0, type, size, data);
		}

		static int _client_crd1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(1, type, size, data);
		}

		static int _client_crd2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(2, type, size, data);
		}

		static int _client_crd3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(3, type, size, data);
		}

		static int _client_crd4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(4, type, size, data);
		}

		static int _client_crd5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(5, type, size, data);
		}

		static int _client_crd6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(6, type, size, data);
		}

		static int _client_crd7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_crd_general(7, type, size, data);
		}

		static int _cl_inb_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u build %u infantry", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u build %u infantry", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for INB%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_inb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(0, type, size, data);
		}

		static int _client_inb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(1, type, size, data);
		}


		static int _client_inb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(2, type, size, data);
		}


		static int _client_inb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(3, type, size, data);
		}


		static int _client_inb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(4, type, size, data);
		}


		static int _client_inb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(5, type, size, data);
		}


		static int _client_inb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(6, type, size, data);
		}


		static int _client_inb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inb_general(7, type, size, data);
		}

		static int _cl_unb_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u build %u units", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u build %u units", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for UNB%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_unb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(0, type, size, data);
		}


		static int _client_unb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(1, type, size, data);
		}

		static int _client_unb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(2, type, size, data);
		}

		static int _client_unb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(3, type, size, data);
		}

		static int _client_unb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(4, type, size, data);
		}

		static int _client_unb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(5, type, size, data);
		}

		static int _client_unb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(6, type, size, data);
		}

		static int _client_unb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unb_general(7, type, size, data);
		}

		static int _cl_plb_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u build %u planes", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u build %u planes", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for PLB%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_plb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(0, type, size, data);
		}

		static int _client_plb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(1, type, size, data);
		}

		static int _client_plb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(2, type, size, data);
		}

		static int _client_plb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(3, type, size, data);
		}

		static int _client_plb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(4, type, size, data);
		}

		static int _client_plb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(5, type, size, data);
		}

		static int _client_plb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(6, type, size, data);
		}

		static int _client_plb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plb_general(7, type, size, data);
		}

		static int _cl_blb_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u build %u buildings", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u build %u buildings", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for BLB%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_blb0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(0, type, size, data);
		}

		static int _client_blb1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(1, type, size, data);
		}

		static int _client_blb2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(2, type, size, data);
		}

		static int _client_blb3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(3, type, size, data);
		}

		static int _client_blb4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(4, type, size, data);
		}

		static int _client_blb5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(5, type, size, data);
		}

		static int _client_blb6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(6, type, size, data);
		}

		static int _client_blb7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blb_general(7, type, size, data);
		}

		static int _cl_inl_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u lost %u infantry", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u lost %u infantry", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for INL%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_inl0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(0, type, size, data);
		}

		static int _client_inl1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(1, type, size, data);
		}

		static int _client_inl2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(2, type, size, data);
		}

		static int _client_inl3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(3, type, size, data);
		}

		static int _client_inl4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(4, type, size, data);
		}

		static int _client_inl5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(5, type, size, data);
		}

		static int _client_inl6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(6, type, size, data);
		}

		static int _client_inl7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_inl_general(7, type, size, data);
		}

		static int _cl_unl_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u lost %u units", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u lost %u units", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for UNL%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_unl0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(0, type, size, data);
		}

		static int _client_unl1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(1, type, size, data);
		}

		static int _client_unl2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(2, type, size, data);
		}

		static int _client_unl3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(3, type, size, data);
		}

		static int _client_unl4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(4, type, size, data);
		}

		static int _client_unl5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(5, type, size, data);
		}

		static int _client_unl6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(6, type, size, data);
		}

		static int _client_unl7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unl_general(7, type, size, data);
		}

		static int _cl_pll_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u lost %u planes", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u lost %u planes", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for PLL%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_pll0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(0, type, size, data);
		}

		static int _client_pll1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(1, type, size, data);
		}

		static int _client_pll2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(2, type, size, data);
		}

		static int _client_pll3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(3, type, size, data);
		}

		static int _client_pll4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(4, type, size, data);
		}

		static int _client_pll5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(5, type, size, data);
		}

		static int _client_pll6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(6, type, size, data);
		}

		static int _client_pll7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_pll_general(7, type, size, data);
		}

		static int _cl_bll_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u lost %u buildings", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u lost %u buildings", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for BLL%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_bll0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(0, type, size, data);
		}

		static int _client_bll1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(1, type, size, data);
		}

		static int _client_bll2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(2, type, size, data);
		}

		static int _client_bll3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(3, type, size, data);
		}

		static int _client_bll4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(4, type, size, data);
		}

		static int _client_bll5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(5, type, size, data);
		}

		static int _client_bll6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(6, type, size, data);
		}

		static int _client_bll7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_bll_general(7, type, size, data);
		}

		static int _cl_ink_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u killed %u infantry", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u killed %u infantry", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for INK%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_ink0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(0, type, size, data);
		}

		static int _client_ink1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(1, type, size, data);
		}

		static int _client_ink2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(2, type, size, data);
		}

		static int _client_ink3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(3, type, size, data);
		}

		static int _client_ink4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(4, type, size, data);
		}

		static int _client_ink5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(5, type, size, data);
		}

		static int _client_ink6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(6, type, size, data);
		}

		static int _client_ink7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_ink_general(7, type, size, data);
		}

		static int _cl_unk_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u killed %u units", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u killed %u units", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for UNK%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_unk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(0, type, size, data);
		}

		static int _client_unk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(1, type, size, data);
		}

		static int _client_unk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(2, type, size, data);
		}

		static int _client_unk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(3, type, size, data);
		}

		static int _client_unk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(4, type, size, data);
		}

		static int _client_unk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(5, type, size, data);
		}

		static int _client_unk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(6, type, size, data);
		}

		static int _client_unk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_unk_general(7, type, size, data);
		}

		static int _cl_plk_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u killed %u planes", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u killed %u planes", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for PLK%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_plk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(0, type, size, data);
		}

		static int _client_plk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(1, type, size, data);
		}

		static int _client_plk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(2, type, size, data);
		}

		static int _client_plk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(3, type, size, data);
		}

		static int _client_plk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(4, type, size, data);
		}

		static int _client_plk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(5, type, size, data);
		}

		static int _client_plk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(6, type, size, data);
		}

		static int _client_plk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_plk_general(7, type, size, data);
		}

		static int _cl_blk_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_time:
				DEBUG2("Player %u killed %u buildings", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u killed %u buildings", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for BLK%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_blk0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(0, type, size, data);
		}

		static int _client_blk1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(1, type, size, data);
		}

		static int _client_blk2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(2, type, size, data);
		}

		static int _client_blk3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(3, type, size, data);
		}

		static int _client_blk4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(4, type, size, data);
		}

		static int _client_blk5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(5, type, size, data);
		}

		static int _client_blk6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(6, type, size, data);
		}

		static int _client_blk7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blk_general(7, type, size, data);
		}

		static int _cl_blc_general(int num, wol_gameres_type type, int size, void const * data)
		{
			switch (type) {
			case wol_gameres_type_int:
				DEBUG2("Player %u captured %u buildings", num, (unsigned int)bn_int_nget(*((bn_int *)data)));
				break;
			case wol_gameres_type_bigint:
				DEBUG2("Player %u captured %u buildings", num, wol_gameres_get_long_from_data(size, data));
				break;
			default:
				WARN2("got unknown gameres type %u for BLC%u", type, num);
				break;
			}
			return 0;
		}

		static int _client_blc0(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(0, type, size, data);
		}

		static int _client_blc1(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(1, type, size, data);
		}

		static int _client_blc2(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(2, type, size, data);
		}

		static int _client_blc3(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(3, type, size, data);
		}

		static int _client_blc4(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(4, type, size, data);
		}

		static int _client_blc5(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(5, type, size, data);
		}

		static int _client_blc6(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(6, type, size, data);
		}

		static int _client_blc7(t_wol_gameres_result * game_result, wol_gameres_type type, int size, void const * data)
		{
			return _cl_blc_general(7, type, size, data);
		}


	}

}
