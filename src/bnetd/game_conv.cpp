/*
 * Copyright (C) 1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999,2000  Rob Crittenden (rcrit@greyoak.com)
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
#define GAME_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "game_conv.h"

#include <cstring>

#include "common/eventlog.h"
#include "common/tag.h"
#include "common/bnet_protocol.h"
#include "common/bn_type.h"
#include "common/util.h"

#include "compat/strsep.h"

#include "game.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		extern t_game_type bngreqtype_to_gtype(t_clienttag clienttag, unsigned short bngtype)
		{
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag");
				return game_type_none;
			}

			if (clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMELISTREQ_ALL:
					return game_type_all;
				case CLIENT_GAMELISTREQ_TOPVBOT:
					return game_type_topvbot;
				case CLIENT_GAMELISTREQ_MELEE:
					return game_type_melee;
				case CLIENT_GAMELISTREQ_FFA:
					return game_type_ffa;
				case CLIENT_GAMELISTREQ_ONEONONE:
					return game_type_oneonone;
				case CLIENT_GAMELISTREQ_LADDER:
					return game_type_ladder;
				case CLIENT_GAMELISTREQ_IRONMAN:
					return game_type_ironman;
				case CLIENT_GAMELISTREQ_MAPSET:
					return game_type_mapset;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLO2DV_UINT ||
				clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMELISTREQ_ALL:
					return game_type_diablo2open;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo II bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLORTL_UINT ||
				clienttag == CLIENTTAG_DIABLOSHR_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMETYPE_DIABLO_0:
				case CLIENT_GAMETYPE_DIABLO_1:
				case CLIENT_GAMETYPE_DIABLO_2:
				case CLIENT_GAMETYPE_DIABLO_3:
				case CLIENT_GAMETYPE_DIABLO_4:
				case CLIENT_GAMETYPE_DIABLO_5:
				case CLIENT_GAMETYPE_DIABLO_6:
				case CLIENT_GAMETYPE_DIABLO_7:
				case CLIENT_GAMETYPE_DIABLO_8:
				case CLIENT_GAMETYPE_DIABLO_9:
				case CLIENT_GAMETYPE_DIABLO_a:
				case CLIENT_GAMETYPE_DIABLO_b:
				case CLIENT_GAMETYPE_DIABLO_c:
				case CLIENT_GAMETYPE_DIABLO_d:
					return game_type_diablo;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_STARCRAFT_UINT ||
				clienttag == CLIENTTAG_BROODWARS_UINT ||
				clienttag == CLIENTTAG_SHAREWARE_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMELISTREQ_ALL:
					return game_type_all;
				case CLIENT_GAMELISTREQ_MELEE:
					return game_type_melee;
				case CLIENT_GAMELISTREQ_FFA:
					return game_type_ffa;
				case CLIENT_GAMELISTREQ_ONEONONE:
					return game_type_oneonone;
				case CLIENT_GAMELISTREQ_CTF:
					return game_type_ctf;
				case CLIENT_GAMELISTREQ_GREED:
					return game_type_greed;
				case CLIENT_GAMELISTREQ_SLAUGHTER:
					return game_type_slaughter;
				case CLIENT_GAMELISTREQ_SDEATH:
					return game_type_sdeath;
				case CLIENT_GAMELISTREQ_LADDER:
					return game_type_ladder;
				case CLIENT_GAMELISTREQ_MAPSET:
					return game_type_mapset;
				case CLIENT_GAMELISTREQ_TEAMMELEE:
					return game_type_teammelee;
				case CLIENT_GAMELISTREQ_TEAMFFA:
					return game_type_teamffa;
				case CLIENT_GAMELISTREQ_TEAMCTF:
					return game_type_teamctf;
				case CLIENT_GAMELISTREQ_PGL:
					return game_type_pgl;
				case CLIENT_GAMELISTREQ_TOPVBOT:
					return game_type_topvbot;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_WARCRAFT3_UINT ||
				clienttag == CLIENTTAG_WAR3XP_UINT)
			{
				return game_type_all;
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown game clienttag \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
				return game_type_none;
			}
		}


		extern t_game_type bngtype_to_gtype(t_clienttag clienttag, unsigned short bngtype)
		{
			char clienttag_str[5];
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag");
				return game_type_none;
			}

			if (clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMELISTREQ_TOPVBOT:
					return game_type_topvbot;
				case CLIENT_GAMELISTREQ_MELEE:
					return game_type_melee;
				case CLIENT_GAMELISTREQ_FFA:
					return game_type_ffa;
				case CLIENT_GAMELISTREQ_ONEONONE:
					return game_type_oneonone;
				case CLIENT_GAMELISTREQ_LADDER:
					return game_type_ladder;
				case CLIENT_GAMELISTREQ_IRONMAN:
					return game_type_ironman;
				case CLIENT_GAMELISTREQ_MAPSET:
					return game_type_mapset;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLO2DV_UINT ||
				clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMETYPE_DIABLO2_OPEN_NORMAL:
				case CLIENT_GAMETYPE_DIABLO2_OPEN_NIGHTMARE:
				case CLIENT_GAMETYPE_DIABLO2_OPEN_HELL:
					return game_type_diablo2open;
				case CLIENT_GAMETYPE_DIABLO2_CLOSE:
					return game_type_diablo2closed;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo II bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLORTL_UINT ||
				clienttag == CLIENTTAG_DIABLOSHR_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMETYPE_DIABLO_0:
				case CLIENT_GAMETYPE_DIABLO_1:
				case CLIENT_GAMETYPE_DIABLO_2:
				case CLIENT_GAMETYPE_DIABLO_3:
				case CLIENT_GAMETYPE_DIABLO_4:
				case CLIENT_GAMETYPE_DIABLO_5:
				case CLIENT_GAMETYPE_DIABLO_6:
				case CLIENT_GAMETYPE_DIABLO_7:
				case CLIENT_GAMETYPE_DIABLO_8:
				case CLIENT_GAMETYPE_DIABLO_9:
				case CLIENT_GAMETYPE_DIABLO_a:
				case CLIENT_GAMETYPE_DIABLO_b:
				case CLIENT_GAMETYPE_DIABLO_c:
					return game_type_diablo;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_STARCRAFT_UINT ||
				clienttag == CLIENTTAG_BROODWARS_UINT ||
				clienttag == CLIENTTAG_SHAREWARE_UINT)
			{
				switch (bngtype)
				{
				case CLIENT_GAMELISTREQ_ALL:
					return game_type_all;
				case CLIENT_GAMELISTREQ_MELEE:
					return game_type_melee;
				case CLIENT_GAMELISTREQ_FFA:
					return game_type_ffa;
				case CLIENT_GAMELISTREQ_ONEONONE:
					return game_type_oneonone;
				case CLIENT_GAMELISTREQ_CTF:
					return game_type_ctf;
				case CLIENT_GAMELISTREQ_GREED:
					return game_type_greed;
				case CLIENT_GAMELISTREQ_SLAUGHTER:
					return game_type_slaughter;
				case CLIENT_GAMELISTREQ_SDEATH:
					return game_type_sdeath;
				case CLIENT_GAMELISTREQ_LADDER:
					return game_type_ladder;
				case CLIENT_GAMELISTREQ_MAPSET:
					return game_type_mapset;
				case CLIENT_GAMELISTREQ_TEAMMELEE:
					return game_type_teammelee;
				case CLIENT_GAMELISTREQ_TEAMFFA:
					return game_type_teamffa;
				case CLIENT_GAMELISTREQ_TEAMCTF:
					return game_type_teamctf;
				case CLIENT_GAMELISTREQ_PGL:
					return game_type_pgl;
				case CLIENT_GAMELISTREQ_TOPVBOT:
					return game_type_topvbot;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
					return game_type_none;
				}
			}
			else if (clienttag == CLIENTTAG_WARCRAFT3_UINT ||
				clienttag == CLIENTTAG_WAR3XP_UINT)
			{
				return game_type_all;
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown game clienttag \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), bngtype);
				return game_type_none;
			}
		}


		extern unsigned short gtype_to_bngtype(t_game_type gtype)
		{
			switch (gtype)
			{
			case game_type_all:
				return CLIENT_GAMELISTREQ_ALL;
			case game_type_melee:
				return CLIENT_GAMELISTREQ_MELEE;
			case game_type_ffa:
				return CLIENT_GAMELISTREQ_FFA;
			case game_type_oneonone:
				return CLIENT_GAMELISTREQ_ONEONONE;
			case game_type_ctf:
				return CLIENT_GAMELISTREQ_CTF;
			case game_type_greed:
				return CLIENT_GAMELISTREQ_GREED;
			case game_type_slaughter:
				return CLIENT_GAMELISTREQ_SLAUGHTER;
			case game_type_sdeath:
				return CLIENT_GAMELISTREQ_SDEATH;
			case game_type_ladder:
				return CLIENT_GAMELISTREQ_LADDER;
			case game_type_mapset:
				return CLIENT_GAMELISTREQ_MAPSET;
			case game_type_teammelee:
				return CLIENT_GAMELISTREQ_TEAMMELEE;
			case game_type_teamffa:
				return CLIENT_GAMELISTREQ_TEAMFFA;
			case game_type_teamctf:
				return CLIENT_GAMELISTREQ_TEAMCTF;
			case game_type_pgl:
				return CLIENT_GAMELISTREQ_PGL;
			case game_type_topvbot:
				return CLIENT_GAMELISTREQ_TOPVBOT;
			case game_type_diablo:
				return CLIENT_GAMELISTREQ_DIABLO;
			case game_type_diablo2open:
				return SERVER_GAMELISTREPLY_TYPE_DIABLO2_OPEN;
			case game_type_diablo2closed:
				eventlog(eventlog_level_error, __FUNCTION__, "don't know how to list Diablo II");
				return 0;
			case game_type_anongame:
				return 0;
			case game_type_none:
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown game type %u", (unsigned int)gtype);
				return 0xffff;
			}
		}


		extern t_game_option bngoption_to_goption(t_clienttag clienttag, t_game_type gtype, unsigned short bngoption)
		{
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag");
				return game_option_none;
			}

			if (clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				switch (gtype)
				{
				case game_type_topvbot:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_1:
						return game_option_topvbot_1;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_2:
						return game_option_topvbot_2;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_3:
						return game_option_topvbot_3;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_4:
						return game_option_topvbot_4;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_5:
						return game_option_topvbot_5;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_6:
						return game_option_topvbot_6;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_7:
						return game_option_topvbot_7;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_melee:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_MELEE_NORMAL:
						return game_option_melee_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ffa:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_FFA_NORMAL:
						return game_option_ffa_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_oneonone:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_ONEONONE_NORMAL:
						return game_option_oneonone_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ladder:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_LADDER_COUNTASLOSS:
						return game_option_ladder_countasloss;
					case CLIENT_STARTGAME4_OPTION_LADDER_NOPENALTY:
						return game_option_ladder_nopenalty;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ironman:
					switch (bngoption)
					{
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_mapset:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_MAPSET_NORMAL:
						return game_option_mapset_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Warcraft II game type \"%s\" %u", tag_uint_to_str(clienttag_str, clienttag), (unsigned int)gtype);
					return game_option_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLO2DV_UINT ||
				clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				switch (gtype)
				{
				case game_type_diablo2open:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_NONE: /* FIXME: really? */
						return game_option_none;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_diablo2closed:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_NONE: /* FIXME: really? */
						return game_option_none;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo II bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo II game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), (unsigned int)gtype);
					return game_option_none;
				}
			}
			else if (clienttag == CLIENTTAG_DIABLORTL_UINT ||
				clienttag == CLIENTTAG_DIABLOSHR_UINT)
			{
				switch (gtype)
				{
				case game_type_diablo:
					switch (bngoption)
					{
					default:
						/* diablo doesn't use any options */
						return game_option_none;
					}
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Diablo game type \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), (unsigned int)gtype);
					return game_option_none;
				}
			}
			else if (clienttag == CLIENTTAG_STARCRAFT_UINT ||
				clienttag == CLIENTTAG_BROODWARS_UINT ||
				clienttag == CLIENTTAG_SHAREWARE_UINT)
			{
				switch (gtype)
				{
				case game_type_melee:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_MELEE_NORMAL:
						return game_option_melee_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ffa:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_FFA_NORMAL:
						return game_option_ffa_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_oneonone:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_ONEONONE_NORMAL:
						return game_option_oneonone_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ctf:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_CTF_NORMAL:
						return game_option_ctf_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_greed:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_GREED_10000:
						return game_option_greed_10000;
					case CLIENT_STARTGAME4_OPTION_GREED_7500:
						return game_option_greed_7500;
					case CLIENT_STARTGAME4_OPTION_GREED_5000:
						return game_option_greed_5000;
					case CLIENT_STARTGAME4_OPTION_GREED_2500:
						return game_option_greed_2500;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_slaughter:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_SLAUGHTER_60:
						return game_option_slaughter_60;
					case CLIENT_STARTGAME4_OPTION_SLAUGHTER_45:
						return game_option_slaughter_45;
					case CLIENT_STARTGAME4_OPTION_SLAUGHTER_30:
						return game_option_slaughter_30;
					case CLIENT_STARTGAME4_OPTION_SLAUGHTER_15:
						return game_option_slaughter_15;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_sdeath:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_SDEATH_NORMAL:
						return game_option_sdeath_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_ladder:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_LADDER_COUNTASLOSS:
						return game_option_ladder_countasloss;
					case CLIENT_STARTGAME4_OPTION_LADDER_NOPENALTY:
						return game_option_ladder_nopenalty;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_mapset:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_MAPSET_NORMAL:
						return game_option_mapset_normal;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_teammelee:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_TEAMMELEE_4:
						return game_option_teammelee_4;
					case CLIENT_STARTGAME4_OPTION_TEAMMELEE_3:
						return game_option_teammelee_3;
					case CLIENT_STARTGAME4_OPTION_TEAMMELEE_2:
						return game_option_teammelee_2;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_teamffa:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_TEAMFFA_4:
						return game_option_teamffa_4;
					case CLIENT_STARTGAME4_OPTION_TEAMFFA_3:
						return game_option_teamffa_3;
					case CLIENT_STARTGAME4_OPTION_TEAMFFA_2:
						return game_option_teamffa_2;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_teamctf:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_TEAMCTF_4:
						return game_option_teamctf_4;
					case CLIENT_STARTGAME4_OPTION_TEAMCTF_3:
						return game_option_teamctf_3;
					case CLIENT_STARTGAME4_OPTION_TEAMCTF_2:
						return game_option_teamctf_2;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_pgl:
					switch (bngoption)
					{
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_topvbot:
					switch (bngoption)
					{
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_1:
						return game_option_topvbot_1;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_2:
						return game_option_topvbot_2;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_3:
						return game_option_topvbot_3;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_4:
						return game_option_topvbot_4;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_5:
						return game_option_topvbot_5;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_6:
						return game_option_topvbot_6;
					case CLIENT_STARTGAME4_OPTION_TOPVBOT_7:
						return game_option_topvbot_7;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft bnet game option for \"%s\" game \"%s\" %hu", tag_uint_to_str(clienttag_str, clienttag), game_type_get_str(gtype), bngoption);
						return game_option_none;
					}
				case game_type_none:
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "unknown Starcraft game type \"%s\" %u(%s)", tag_uint_to_str(clienttag_str, clienttag), (unsigned int)gtype, game_type_get_str(gtype));
					return game_option_none;
				}
			}
			else if (clienttag == CLIENTTAG_WARCRAFT3_UINT ||
				clienttag == CLIENTTAG_WAR3XP_UINT)
			{
				return game_option_none;
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown game clienttag \"%s\" %u", tag_uint_to_str(clienttag_str, clienttag), (unsigned int)gtype);
				return game_option_none;
			}
		}


		extern t_game_result bngresult_to_gresult(unsigned int bngresult)
		{
			switch (bngresult)
			{
			case CLIENT_GAME_REPORT_RESULT_PLAYING:
				return game_result_playing;
			case CLIENT_GAME_REPORT_RESULT_WIN:
				return game_result_win;
			case CLIENT_GAME_REPORT_RESULT_LOSS:
				return game_result_loss;
			case CLIENT_GAME_REPORT_RESULT_DRAW:
				return game_result_draw;
			case CLIENT_GAME_REPORT_RESULT_DISCONNECT:
				return game_result_disconnect;
			case CLIENT_GAME_REPORT_RESULT_OBSERVER:
				return game_result_observer;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown bnet game result %u", bngresult);
				return game_result_disconnect; /* bad packet? */
			}
		}


		extern t_game_maptype bngmaptype_to_gmaptype(unsigned int bngmaptype)
		{
			switch (bngmaptype)
			{
			case CLIENT_MAPTYPE_SELFMADE:
				return game_maptype_selfmade;
			case CLIENT_MAPTYPE_BLIZZARD:
				return game_maptype_blizzard;
			case CLIENT_MAPTYPE_LADDER:
				return game_maptype_ladder;
			case CLIENT_MAPTYPE_PGL:
				return game_maptype_pgl;
			case CLIENT_MAPTYPE_KBK:
				return game_maptype_kbk;
			case CLIENT_MAPTYPE_CompUSA:
				return game_maptype_compusa;
			default:
				return game_maptype_none;
			}
		}


		extern t_game_tileset bngtileset_to_gtileset(unsigned int bngtileset)
		{
			switch (bngtileset)
			{
			case CLIENT_TILESET_BADLANDS:
				return game_tileset_badlands;
			case CLIENT_TILESET_SPACE:
				return game_tileset_space;
			case CLIENT_TILESET_INSTALLATION:
				return game_tileset_installation;
			case CLIENT_TILESET_ASHWORLD:
				return game_tileset_ashworld;
			case CLIENT_TILESET_JUNGLE:
				return game_tileset_jungle;
			case CLIENT_TILESET_DESERT:
				return game_tileset_desert;
			case CLIENT_TILESET_ICE:
				return game_tileset_ice;
			case CLIENT_TILESET_TWILIGHT:
				return game_tileset_twilight;
			default:
				return game_tileset_none;
			}
		}


		extern t_game_speed bngspeed_to_gspeed(unsigned int bngspeed)
		{
			switch (bngspeed)
			{
			case CLIENT_GAMESPEED_SLOWEST:
				return game_speed_slowest;
			case CLIENT_GAMESPEED_SLOWER:
				return game_speed_slower;
			case CLIENT_GAMESPEED_SLOW:
				return game_speed_slow;
			case CLIENT_GAMESPEED_NORMAL:
				return game_speed_normal;
			case CLIENT_GAMESPEED_FAST:
				return game_speed_fast;
			case CLIENT_GAMESPEED_FASTER:
				return game_speed_faster;
			case CLIENT_GAMESPEED_FASTEST:
				return game_speed_fastest;
			default:
				return game_speed_none;
			}
		}

		t_game_speed w3speed_to_gspeed(unsigned int w3speed)
		{
			switch (w3speed)
			{
			case 0: return game_speed_slow;
			case 1: return game_speed_normal;
			case 2: return game_speed_fast;
			default: return game_speed_none;
			}
		}


		extern t_game_difficulty bngdifficulty_to_gdifficulty(unsigned int bngdifficulty)
		{
			switch (bngdifficulty)
			{
			case CLIENT_DIFFICULTY_NORMAL:
				return game_difficulty_normal;
			case CLIENT_DIFFICULTY_NIGHTMARE:
				return game_difficulty_nightmare;
			case CLIENT_DIFFICULTY_HELL:
				return game_difficulty_hell;
			case CLIENT_DIFFICULTY_HARDCORE_NORMAL:
				return game_difficulty_hardcore_normal;
			case CLIENT_DIFFICULTY_HARDCORE_NIGHTMARE:
				return game_difficulty_hardcore_nightmare;
			case CLIENT_DIFFICULTY_HARDCORE_HELL:
				return game_difficulty_hardcore_hell;
			default:
				return game_difficulty_none;
			}
		}


		static const char * _w3_decrypt_mapinfo(const char *enc)
		{
			char	*mapinfo;
			char	*dec;
			unsigned    pos;
			unsigned char bitmask;

			if (!(mapinfo = xstrdup(enc))) {
				eventlog(eventlog_level_error, __FUNCTION__, "not enough memory to setup temporary buffer");
				return NULL;
			}

			dec = mapinfo;
			pos = 0;
			bitmask = 0; /* stupid gcc warning */
			while (*enc)
			{
				if (pos % 8)
				{
					*dec = *enc;
					if (!(bitmask & 0x1)) (*dec)--;
					dec++;
					bitmask >>= 1;
				}
				else
					bitmask = *enc >> 1;
				enc++;
				pos++;
			}
			*dec = '\0';

			return mapinfo;
		}


		extern int game_parse_info(t_game * game, char const * gameinfo)
		{
			t_clienttag  clienttag;
			char *       save;
			char *       line1;
			char *       line2;
			char *       currtok;
			char const * unknown;
			char const * mapsize;
			char const * maxplayers;
			char const * speed;
			char const * maptype;
			char const * gametype;
			char const * option;
			char const * checksum;
			char const * tileset;
			char const * player;
			char const * mapname;
			unsigned int bngmapsize;
			unsigned int bngmaxplayers;
			unsigned int bngspeed;
			unsigned int bngmaptype;
			unsigned int bngtileset;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!gameinfo)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL gameinfo");
				return -1;
			}

			/*

			BW 104:
			9: recv class=bnet[0x01] type=CLIENT_STARTGAME4[0x1cff] length=74
			0000:   FF 1C 4A 00 00 00 00 00   00 00 00 00 09 00 01 00    ..J.............
			0010:   00 00 00 00 01 00 00 00   4E 4F 50 45 4E 41 4C 54    ........NOPENALT
			0020:   59 00 00 2C 34 34 2C 31   34 2C 2C 32 2C 39 2C 31    Y..,44,14,,2,9,1
			0030:   2C 33 65 33 37 61 38 34   63 2C 33 2C 52 6F 73 73    ,3e37a84c,3,Ross
			0040:   0D 41 73 68 72 69 67 6F   0D 00                      .Ashrigo..

			Diablo:
			7: cli class=bnet[0x01] type=CLIENT_STARTGAME3[0x1aff] length=105
			0000:   FF 1A 69 00 01 00 00 00   00 00 00 00 00 00 00 00    ..i.............
			0010:   0F 00 00 00 00 00 00 00   E0 17 00 00 61 73 64 66    ............asdf
			0020:   61 73 64 66 61 73 64 66   61 73 64 66 32 00 61 73    asdfasdfasdf2.as
			0030:   64 66 61 73 64 66 73 61   64 66 61 73 64 66 32 00    dfasdfsadfasdf2.
			0040:   30 0D 7A 62 6E 7A 62 6E   7A 62 6E 0D 4C 54 52 44    0.zbnzbnzbn.LTRD
			0050:   20 31 20 30 20 30 20 33   30 20 31 30 20 32 30 20     1 0 0 30 10 20
			0060:   32 35 20 31 30 30 20 30   00                         25 100 0.

			10: recv class=bnet[0x01] type=CLIENT_STARTGAME3[0x1aff] length=72
			0000:   FF 1A 48 00 00 00 00 00   00 00 00 00 00 00 00 00    ..H.............
			0010:   0F 00 00 00 00 00 00 00   E0 17 00 00 61 6E 73 00    ............ans.
			0020:   00 30 0D 77 61 72 72 69   6F 72 0D 4C 54 52 44 20    .0.warrior.LTRD
			0030:   31 20 30 20 30 20 33 30   20 31 30 20 32 30 20 32    1 0 0 30 10 20 2
			0040:   35 20 31 30 30 20 30 00                              5 100 0.


			Warcraft:
			4: srv class=bnet[0x01] type=SERVER_GAMELISTREPLY[0x09ff] length=524
			0000:   FF 09 0C 02 05 00 00 00   02 00 01 00 09 04 00 00    ................
			0010:   02 00 17 E0 D0 C6 66 F0   00 00 00 00 00 00 00 00    ......f.........
			0020:   04 00 00 00 74 00 00 00   48 65 79 77 6F 6F 64 73    ....t...Heywoods
			0030:   20 47 4F 57 32 76 32 00   00 2C 2C 2C 36 2C 32 2C     GOW2v2..,,,6,2,
			0040:   32 2C 31 2C 31 39 37 33   64 35 66 32 2C 33 30 30    2,1,1973d5f2,300
			0050:   30 2C 48 65 79 77 6F 6F   64 0D 47 61 72 64 65 6E    0,Heywood.Garden
			0060:   20 6F 66 20 77 61 72 20   42 4E 45 2E 70 75 64 0D     of war BNE.pud.
			0070:   00 09 00 02 00 09 04 00   00 02 00 17 E0 CF 45 3E    ..............E>
			0080:   65 00 00 00 00 00 00 00   00 10 00 00 00 11 00 00    e...............
			0090:   00 67 6F 77 20 6C 61 64   64 65 72 20 31 76 31 00    .gow ladder 1v1.
			00A0:   00 2C 2C 2C 36 2C 32 2C   39 2C 32 2C 65 66 38 34    .,,,6,2,9,2,ef84
			00B0:   33 61 35 33 2C 2C 77 63   73 63 6D 61 73 74 65 72    3a53,,wcscmaster
			00C0:   0D 47 61 72 64 65 6E 20   6F 66 20 77 61 72 20 42    .Garden of war B
			00D0:   4E 45 2E 70 75 64 0D 00   09 00 01 00 09 04 00 00    NE.pud..........
			00E0:   02 00 17 E0 98 CE 7C E2   00 00 00 00 00 00 00 00    ......|.........
			00F0:   04 00 00 00 4D 00 00 00   4C 61 64 64 65 72 20 46    ....M...Ladder F
			0100:   46 41 20 6F 6E 6C 79 00   00 2C 2C 2C 36 2C 32 2C    FA only..,,,6,2,
			0110:   39 2C 31 2C 61 65 32 66   61 61 62 37 2C 2C 6B 69    9,1,ae2faab7,,ki
			0120:   6C 6F 67 72 61 6D 0D 50   6C 61 69 6E 73 20 6F 66    logram.Plains of
			0130:   20 73 6E 6F 77 20 42 4E   45 2E 70 75 64 0D 00 0F     snow BNE.pud...
			0140:   00 04 00 09 04 00 00 02   00 17 E0 C6 0B 13 3C 00    ..............<.
			0150:   00 00 00 00 00 00 00 04   00 00 00 BE 00 00 00 4C    ...............L
			0160:   61 64 64 65 72 20 31 20   6F 6E 20 31 00 00 2C 2C    adder 1 on 1..,,
			0170:   2C 36 2C 32 2C 66 2C 34   2C 66 63 63 35 38 65 34    ,6,2,f,4,fcc58e4
			0180:   61 2C 37 32 30 30 2C 49   63 65 36 39 62 75 72 67    a,7200,Ice69burg
			0190:   0D 46 6F 72 65 73 74 20   54 72 61 69 6C 20 42 4E    .Forest Trail BN
			01A0:   45 2E 70 75 64 0D 00 0F   00 04 00 09 04 00 00 02    E.pud...........
			01B0:   00 04 15 D1 F4 B6 6A 00   00 00 00 00 00 00 00 04    ......j.........
			01C0:   00 00 00 C8 03 00 00 52   6F 63 6B 20 49 74 21 21    .......Rock It!!
			01D0:   20 32 76 32 2B 00 00 2C   2C 2C 38 2C 31 2C 66 2C     2v2+..,,,8,1,f,
			01E0:   34 2C 62 30 36 38 63 62   34 62 2C 66 30 30 30 2C    4,b068cb4b,f000,
			01F0:   6E 75 66 74 63 72 6F 77   0D 43 72 6F 73 73 68 61    nuftcrow.Crossha
			0200:   69 72 20 42 4E 45 2E 70   75 64 0D 00                ir BNE.pud..

			FIXME:
			Note the map size and max players fields are empty. Does WCII have different
			map sizes and starting positions? If so, are they reported here?
			Also note the map tileset is "3000". Maybe this isn't the tileset for WCII
			but instead the starting gold or something.

			FIXME:
			Also, what is the upper player limit on WCII... 8 like on Starcraft?
			*/

			if (!(clienttag = game_get_clienttag(game)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "NULL clienttag for game?");
				return -1;
			}

			if (clienttag == CLIENTTAG_DIABLORTL_UINT ||
				clienttag == CLIENTTAG_DIABLOSHR_UINT)
			{
				game_set_maxplayers(game, 4);
				eventlog(eventlog_level_debug, __FUNCTION__, "no gameinfo for Diablo");
				return 0;
			}
			else if (clienttag == CLIENTTAG_DIABLO2DV_UINT ||
				clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				if ((game->type == game_type_diablo2closed) &&
					(!std::strlen(gameinfo)))

				{
					/* D2 closed games are handled by d2cs so we can have only have a generic startgame4
					   without any info :(, this fix also a memory leak for description allocation */
					game_set_difficulty(game, game_difficulty_none);
					game_set_description(game, "");
				}
				else
				{
					char         difficulty[2];
					unsigned int bngdifficulty;

					if (!std::strlen(gameinfo))
					{
						eventlog(eventlog_level_info, __FUNCTION__, "got empty gameinfo (from D2 client)");
						return -1;
					}

					difficulty[0] = gameinfo[0];
					difficulty[1] = '\0';
					if (str_to_uint(difficulty, &bngdifficulty) < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing difficulty) \"%s\"", gameinfo);
						return -1;
					}
					game_set_difficulty(game, bngdifficulty);
					game_set_description(game, &gameinfo[1]);

					if ((game->type == game_type_diablo2closed))
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "D2 bug workarround needed (open games tagged as closed)");
						game->type = game_type_diablo2open;
					}
				}
				return 0;
			}
			else if (clienttag == CLIENTTAG_WARCRAFT3_UINT ||
				clienttag == CLIENTTAG_WAR3XP_UINT)
			{
				/*  Warcraft 3 game info format -- by Soar
					0x00 -- 1 byte  (char, Empty slots)
					0x01 -- 8 bytes (char[8], Count of games created)

					From offset 0x11 there is a bitmask byte before every 7 bytes, offset 0x09 to 0x10 also has bitmask 0.
					Bit 0 corresponds to byte 0, bit 1 to byte 1, and so on.
					Except offset 0x09, only Byte 1-7 contribute to the info data, bit 0 seems to be always 1;
					Decoding these bytes works as follows:
					If the corresponding bit is a '1' then the character is moved over directly.
					If the corresponding bit is a '0' then subtract 1 from the character.
					(We decode info data and remove the bitmask bytes from info data in following description)
					0x09 -- 5 bytes (char[5], map options)
					0x0e -- 1 bytes (0, seems to be a seperate sign)
					0x0f -- 2 bytes (short, mapsize x)
					0x11 -- 2 bytes (short, mapsize y)
					0x13 -- 4 bytes (long, unknown, map checksum ?)
					0x17 -- n bytes (string, mapname \0 terminated)
					0x17+n -- m bytes (string, game creator \0 terminated, but we already set this to game before, :P)
					0x17+n+m -- 2 bytes: \0 \0 (first \0 might be a reserved string for password, but blizzard didn't implement it, second \0 is the info string end sign)
					*/
				const char *pstr;

				if (!std::strlen(gameinfo))
				{
					eventlog(eventlog_level_info, __FUNCTION__, "got empty gameinfo (from W3 client)");
					return -1;
				}

				if (std::strlen(gameinfo) < 0xf + 2 + 1 + 2 + 4) {
					eventlog(eventlog_level_error, __FUNCTION__, "got too short W3 mapinfo");
					return -1;
				}
				pstr = gameinfo + 9;
				pstr = _w3_decrypt_mapinfo(pstr);
				if (!pstr) return -1;
				/* after decryption we dont have the mask bytes anymore so offsets need
				 * to be adjusted acordingly */
				game_set_speed(game, w3speed_to_gspeed(bn_byte_get(*((bn_byte*)(pstr)))));
				game_set_mapsize_x(game, bn_short_get(*((bn_short*)(pstr + 5))));
				game_set_mapsize_y(game, bn_short_get(*((bn_short*)(pstr + 7))));
				game_set_mapname(game, pstr + 13);
				xfree((void*)pstr);

				return 0;
			}

			/* otherwise it's Starcraft, Brood War, or Warcraft II */
			if (!(save = xstrdup(gameinfo)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for save");
				return -1;
			}

			if (!(line1 = std::strtok(save, "\r"))) /* actual game info fields */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing line1) \"%s\"", gameinfo);
				xfree(save);
				return -1;
			}
			if (!(line2 = std::strtok(NULL, "\r")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing player) \"%s\"", gameinfo);
				xfree(save);
				return -1;
			}
			/* is there room for another field after that? */

			/*
			 * This is the same as the normal std::strtok() function but it doesn't skip over
			 * empty entries.  The C-library std::strtok() will skip past entries like 12,,3
			 * and rather than you being able to get "12", "", "3" you get "12", "3".
			 * Since some values returned by the client contain these empty entries we
			 * need this.  Unlike std::strtok() all state is recorded in the first argument.
			 */
			currtok = line1;
			if (!(unknown = strsep(&currtok, ","))) /* skip past first field (always empty?) */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing unknown)");
				xfree(save);
				return -1;
			}
			if (!(mapsize = strsep(&currtok, ",")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing mapsize)");
				xfree(save);
				return -1;
			}
			if (!(maxplayers = strsep(&currtok, ","))) /* for later use (FIXME: what is upper field, max observers?) */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing maxplayers)");
				xfree(save);
				return -1;
			}
			if (!(speed = strsep(&currtok, ",")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing speed)");
				xfree(save);
				return -1;
			}
			if (!(maptype = strsep(&currtok, ",")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing maptype)");
				xfree(save);
				return -1;
			}
			if (!(gametype = strsep(&currtok, ","))) /* this is set from another field */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing gametype)");
				xfree(save);
				return -1;
			}
			if (!(option = strsep(&currtok, ","))) /* this is set from another field */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing option)");
				xfree(save);
				return -1;
			}
			if (!(checksum = strsep(&currtok, ","))) /* FIXME */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing checksum)");
				xfree(save);
				return -1;
			}
			if (!(tileset = strsep(&currtok, ",")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing tileset)");
				xfree(save);
				return -1;
			}
			if (!(player = strsep(&currtok, ",")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bad gameinfo format (missing player)");
				xfree(save);
				return -1;
			}

			mapname = line2; /* only one item on this line */

			eventlog(eventlog_level_debug, __FUNCTION__, "got info \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", mapsize, maxplayers, speed, maptype, gametype, option, checksum, tileset, player, mapname);

			/* The map size is determined by breaking the number into two pieces and
			 * multiplying each piece by 32.
			 * for example, 34 = (32*3) x (32*4) = 96 x 128
			 */
			/* special handling for mapsize. empty is 256x256 */
			if ((mapsize[0] == '\0') || (str_to_uint(mapsize, &bngmapsize) < 0))
				bngmapsize = 88; /* 256x256 */
			game_set_mapsize_x(game, (bngmapsize / 10) * 32);
			game_set_mapsize_y(game, (bngmapsize % 10) * 32);

			/* special handling for maxplayers, empty is 8 */
			if ((maxplayers[0] == '\0') || (str_to_uint(maxplayers, &bngmaxplayers) < 0))
				bngmaxplayers = 8;
			game_set_maxplayers(game, (bngmaxplayers % 10));

			/* special handling for gamespeed. empty is fast */
			if ((speed[0] == '\0') || (str_to_uint(speed, &bngspeed) < 0))
				bngspeed = CLIENT_GAMESPEED_FAST;
			game_set_speed(game, bngspeed_to_gspeed(bngspeed));

			/* special handling for maptype. empty is self-made */
			if ((maptype[0] == '\0') || (str_to_uint(maptype, &bngmaptype) < 0))
				bngmaptype = CLIENT_MAPTYPE_SELFMADE;
			game_set_maptype(game, bngmaptype_to_gmaptype(bngmaptype));

			/* special handling for tileset. empty is badlands */
			if ((tileset[0] == '\0') || (str_to_uint(tileset, &bngtileset) < 0))
				bngtileset = CLIENT_TILESET_BADLANDS;
			game_set_tileset(game, bngtileset_to_gtileset(bngtileset));

			game_set_mapname(game, mapname);

			xfree(save);

			return 0;
		}

	}

}
