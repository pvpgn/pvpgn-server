--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- All attribute get/set actions must have a wrapper to avoid write a wrong type into database

--
-- Profile
--

function account_get_acct_email(username)
	return api.account_get_attr(username, "BNET\\acct\\email", attr_type_str)
end
function account_set_acct_email(username, value)
	return api.account_set_attr(username, "BNET\\acct\\email", attr_type_str, value)
end

function account_get_auth_admin(username, channelname)
	if channelname then
		return api.account_get_attr(username, "BNET\\auth\\admin\\" .. channelname, attr_type_bool)	
	else
		return api.account_get_attr(username, "BNET\\auth\\admin", attr_type_bool)
	end
end
function account_set_auth_admin(username, channelname, value)
	if channelname then
		return api.account_set_attr(username, "BNET\\auth\\admin\\" .. channelname, attr_type_bool, value)
	else
		return api.account_set_attr(username, "BNET\\auth\\admin", attr_type_bool, value)
	end
end

function account_get_auth_operator(username, channelname)
	if channelname then
		return api.account_get_attr(username, "BNET\\auth\\operator\\" .. channelname, attr_type_bool)
	else
		return api.account_get_attr(username, "BNET\\auth\\operator", attr_type_bool)
	end
end
function account_set_auth_operator(username, channelname, value)
	if channelname then
		return api.account_set_attr(username, "BNET\\auth\\operator\\" .. channelname, attr_type_bool, value)
	else
		return api.account_set_attr(username, "BNET\\auth\\operator", attr_type_bool, value)
	end
end

function account_get_auth_voice(username, channelname)
	if channelname then
		return api.account_get_attr(username, "BNET\\auth\\voice\\" .. channelname, attr_type_bool)
	else
		return api.account_get_attr(username, "BNET\\auth\\voice", attr_type_bool)
	end
end
function account_set_auth_voice(username, channelname, value)
	if channelname then
		return api.account_set_attr(username, "BNET\\auth\\voice\\" .. channelname, attr_type_bool, value)
	else
		return api.account_set_attr(username, "BNET\\auth\\voice", attr_type_bool, value)
	end
end

function account_is_operator_or_admin(username, channelname)
	return account_get_auth_operator(username, channelname) or account_get_auth_admin(username, channelname) or account_get_auth_operator(username, nil) or account_get_auth_admin(username, nil)
end


function account_get_auth_announce(username)
	return api.account_get_attr(username, "BNET\\auth\\announce", attr_type_bool)
end
function account_set_auth_announce(username, value)
	return api.account_set_attr(username, "BNET\\auth\\announce", attr_type_bool, value)
end

function account_get_auth_botlogin(username)
	return api.account_get_attr(username, "BNET\\auth\\botlogin", attr_type_bool)
end
function account_set_auth_botlogin(username, value)
	return api.account_set_attr(username, "BNET\\auth\\botlogin", attr_type_bool, value)
end

function account_get_auth_lock(username)
	return api.account_get_attr(username, "BNET\\auth\\lock", attr_type_bool)
end
function account_set_auth_lock(username, value)
	return api.account_set_attr(username, "BNET\\auth\\lock", attr_type_bool, value)
end

function account_get_auth_locktime(username)
	return api.account_get_attr(username, "BNET\\auth\\locktime", attr_type_int)
end
function account_set_auth_locktime(username, value)
	return api.account_set_attr(username, "BNET\\auth\\locktime", attr_type_int, value)
end

function account_get_auth_lockreason(username)
	return api.account_get_attr(username, "BNET\\auth\\lockreason", attr_type_str)
end
function account_set_auth_lockreason(username, value)
	return api.account_set_attr(username, "BNET\\auth\\lockreason", attr_type_str, value)
end

function account_get_auth_lockby(username)
	return api.account_get_attr(username, "BNET\\auth\\lockby", attr_type_str)
end
function account_set_auth_lockby(username, value)
	return api.account_set_attr(username, "BNET\\auth\\lockby", attr_type_str, value)
end

function account_get_auth_mute(username)
	return api.account_get_attr(username, "BNET\\auth\\mute", attr_type_bool)
end
function account_set_auth_mute(username, value)
	return api.account_set_attr(username, "BNET\\auth\\mute", attr_type_bool, value)
end

function account_get_auth_mutetime(username)
	return api.account_get_attr(username, "BNET\\auth\\mutetime", attr_type_int)
end
function account_set_auth_mutetime(username, value)
	return api.account_set_attr(username, "BNET\\auth\\mutetime", attr_type_int, value)
end

function account_get_auth_mutereason(username)
	return api.account_get_attr(username, "BNET\\auth\\mutereason", attr_type_str)
end
function account_set_auth_mutereason(username, value)
	return api.account_set_attr(username, "BNET\\auth\\mutereason", attr_type_str, value)
end

function account_get_auth_muteby(username)
	return api.account_get_attr(username, "BNET\\auth\\muteby", attr_type_str)
end
function account_set_auth_muteby(username, value)
	return api.account_set_attr(username, "BNET\\auth\\muteby", attr_type_str, value)
end

function account_get_auth_command_groups(username)
	return api.account_get_attr(username, "BNET\\auth\\command_groups", attr_type_int)
end
function account_set_auth_command_groups(username, value)
	return api.account_set_attr(username, "BNET\\auth\\command_groups", attr_type_int, value)
end

function account_get_acct_lastlogin_time(username)
	return api.account_get_attr(username, "BNET\\acct\\lastlogin_time", attr_type_int)
end
function account_set_acct_lastlogin_time(username, value)
	return api.account_set_attr(username, "BNET\\acct\\lastlogin_time", attr_type_int, value)
end

function account_get_acct_lastlogin_owner(username)
	return api.account_get_attr(username, "BNET\\acct\\lastlogin_owner", attr_type_str)
end
function account_set_acct_lastlogin_owner(username, value)
	return api.account_set_attr(username, "BNET\\acct\\lastlogin_owner", attr_type_str, value)
end

function account_get_acct_createtime(username)
	return api.account_get_attr(username, "BNET\\acct\\ctime", attr_type_int)
end
function account_set_acct_createtime(username, value)
	return api.account_set_attr(username, "BNET\\acct\\ctime", attr_type_int, value)
end

function account_get_acct_lastlogin_clienttag(username)
	return api.account_get_attr(username, "BNET\\acct\\lastlogin_clienttag", attr_type_str)
end
function account_set_acct_lastlogin_clienttag(username, value)
	return api.account_set_attr(username, "BNET\\acct\\lastlogin_clienttag", attr_type_str, value)
end

function account_get_acct_lastlogin_ip(username)
	return api.account_get_attr(username, "BNET\\acct\\lastlogin_ip", attr_type_str)
end
function account_set_acct_lastlogin_ip(username, value)
	return api.account_set_attr(username, "BNET\\acct\\lastlogin_ip", attr_type_str, value)
end

function account_get_acct_passhash(username)
	return api.account_get_attr(username, "BNET\\acct\\passhash1", attr_type_str)
end
function account_set_acct_passhash(username, value)
	return api.account_set_attr(username, "BNET\\acct\\passhash1", attr_type_str, value)
end

function account_get_acct_verifier(username)
	return api.account_get_attr(username, "BNET\\acct\\verifier", attr_type_raw)
end
function account_set_acct_verifier(username, value)
	return api.account_set_attr(username, "BNET\\acct\\verifier", attr_type_raw, value)
end

function account_get_acct_salt(username)
	return api.account_get_attr(username, "BNET\\acct\\salt", attr_type_raw)
end
function account_set_acct_salt(username, value)
	return api.account_set_attr(username, "BNET\\acct\\salt", attr_type_raw, value)
end

function account_get_userlang(username)
	return api.account_get_attr(username, "BNET\\acct\\userlang", attr_type_str)
end
function account_set_userlang(username, value)
	return api.account_set_attr(username, "BNET\\acct\\userlang", attr_type_str, value)
end

--
-- Profile
--

function account_get_auth_adminnormallogin(username)
	return api.account_get_attr(username, "BNET\\auth\\adminnormallogin", attr_type_bool)
end
function account_set_auth_adminnormallogin(username, value)
	return api.account_set_attr(username, "BNET\\auth\\adminnormallogin", attr_type_bool, value)
end

function account_get_auth_changepass(username)
	return api.account_get_attr(username, "BNET\\auth\\changepass", attr_type_bool)
end
function account_set_auth_changepass(username, value)
	return api.account_set_attr(username, "BNET\\auth\\changepass", attr_type_bool, value)
end

function account_get_auth_changeprofile(username)
	return api.account_get_attr(username, "BNET\\auth\\changeprofile", attr_type_bool)
end
function account_set_auth_changeprofile(username, value)
	return api.account_set_attr(username, "BNET\\auth\\changeprofile", attr_type_bool, value)
end

function account_get_auth_createnormalgame(username)
	return api.account_get_attr(username, "BNET\\auth\\createnormalgame", attr_type_bool)
end
function account_set_auth_createnormalgame(username, value)
	return api.account_set_attr(username, "BNET\\auth\\createnormalgame", attr_type_bool, value)
end

function account_get_auth_joinnormalgame(username)
	return api.account_get_attr(username, "BNET\\auth\\joinnormalgame", attr_type_bool)
end
function account_set_auth_joinnormalgame(username, value)
	return api.account_set_attr(username, "BNET\\auth\\joinnormalgame", attr_type_bool, value)
end

function account_get_auth_createladdergame(username)
	return api.account_get_attr(username, "BNET\\auth\\createladdergame", attr_type_bool)
end
function account_set_auth_createladdergame(username, value)
	return api.account_set_attr(username, "BNET\\auth\\createladdergame", attr_type_bool, value)
end

function account_get_auth_joinladdergame(username)
	return api.account_get_attr(username, "BNET\\auth\\joinladdergame", attr_type_bool)
end
function account_set_auth_joinladdergame(username, value)
	return api.account_set_attr(username, "BNET\\auth\\joinladdergame", attr_type_bool, value)
end

--
-- Profile
--

function account_get_sex(username)
	return api.account_get_attr(username, "profile\\sex", attr_type_str)
end
function account_set_sex(username, value)
	return api.account_set_attr(username, "profile\\sex", attr_type_str, value)
end

function account_get_age(username)
	return api.account_get_attr(username, "profile\\age", attr_type_str)
end
function account_set_age(username, value)
	return api.account_set_attr(username, "profile\\age", attr_type_str, value)
end

function account_get_location(username)
	return api.account_get_attr(username, "profile\\location", attr_type_str)
end
function account_set_location(username, value)
	return api.account_set_attr(username, "profile\\location", attr_type_str, value)
end

function account_get_description(username)
	return api.account_get_attr(username, "profile\\description", attr_type_str)
end
function account_set_description(username, value)
	return api.account_set_attr(username, "profile\\description", attr_type_str, value)
end



--
-- Warcraft 3
--

function account_get_soloxp(username)
	return api.account_get_attr(username, "Record\\W3XP\\solo_xp", attr_type_num)
end
function account_set_soloxp(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\solo_xp", attr_type_num, value)
end

function account_get_sololevel(username)
	return api.account_get_attr(username, "Record\\W3XP\\solo_level", attr_type_num)
end
function account_set_sololevel(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\solo_level", attr_type_num, value)
end

function account_get_solowins(username)
	return api.account_get_attr(username, "Record\\W3XP\\solo_wins", attr_type_num)
end
function account_set_solowins(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\solo_wins", attr_type_num, value)
end

function account_get_sololosses(username)
	return api.account_get_attr(username, "Record\\W3XP\\solo_losses", attr_type_num)
end
function account_set_sololosses(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\solo_losses", attr_type_num, value)
end

function account_get_solorank(username)
	return api.account_get_attr(username, "Record\\W3XP\\solo_rank", attr_type_num)
end
function account_set_solorank(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\solo_rank", attr_type_num, value)
end


--
-- Starcraft
--

function account_get_normal_wins(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\wins", attr_type_num)
end
function account_set_normal_wins(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\wins", attr_type_num, value)
end

function account_get_normal_losses(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\losses", attr_type_num)
end
function account_set_normal_losses(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\losses", attr_type_num, value)
end

function account_get_normal_draws(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\draws", attr_type_num)
end
function account_set_normal_draws(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\draws", attr_type_num, value)
end

function account_get_normal_disconnects(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\disconnects", attr_type_num)
end
function account_set_normal_disconnects(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\disconnects", attr_type_num, value)
end

function account_get_normal_last_time(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\last game", attr_type_num)
end
function account_set_normal_last_time(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\last game", attr_type_num, value)
end

function account_get_normal_last_result(username)
	return api.account_get_attr(username, "Record\\SEXP\\0\\last game result", attr_type_num)
end
function account_set_normal_last_result(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\0\\last game result", attr_type_num, value)
end


function account_get_ladder_wins(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\wins", attr_type_num)
end
function account_set_ladder_wins(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\wins", attr_type_num, value)
end

function account_get_ladder_losses(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\losses", attr_type_num)
end
function account_set_ladder_losses(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\losses", attr_type_num, value)
end

function account_get_ladder_draws(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\draws", attr_type_num)
end
function account_set_ladder_draws(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\draws", attr_type_num, value)
end

function account_get_ladder_disconnects(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\disconnects", attr_type_num)
end
function account_set_ladder_disconnects(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\disconnects", attr_type_num, value)
end

function account_get_ladder_last_time(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\last game", attr_type_num)
end
function account_set_ladder_last_time(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\last game", attr_type_num, value)
end

function account_get_ladder_last_result(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\last game result", attr_type_num)
end
function account_set_ladder_last_result(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\last game result", attr_type_num, value)
end

function account_get_ladder_rating(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\rating", attr_type_num)
end
function account_set_ladder_rating(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\rating", attr_type_num, value)
end

function account_get_ladder_rank(username)
	return api.account_get_attr(username, "Record\\SEXP\\1\\rank", attr_type_num)
end
function account_set_ladder_rank(username, value)
	return api.account_set_attr(username, "Record\\SEXP\\1\\rank", attr_type_num, value)
end

-- TODO: wrappers for 2x2


--
-- Warcraft 2
--

--[[ Warcraft 2 has the same functions as Starcraft, but "W2BN" instead of "SEXP" in path. Replace it if you have a single  2 server. ]]--



--
-- Diablo
--

function account_get_normal_level(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\level", attr_type_num)
end
function account_set_normal_level(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\level", attr_type_num, value)
end

function account_get_normal_class(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\class", attr_type_num)
end
function account_set_normal_class(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\class", attr_type_num, value)
end

function account_get_normal_diablo_kills(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\diablo kills", attr_type_num)
end
function account_set_normal_diablo_kills(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\diablo kills", attr_type_num, value)
end

function account_get_normal_strength(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\strength", attr_type_num)
end
function account_set_normal_strength(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\strength", attr_type_num, value)
end

function account_get_normal_dexterity(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\dexterity", attr_type_num)
end
function account_set_normal_dexterity(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\dexterity", attr_type_num, value)
end

function account_get_normal_vitality(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\vitality", attr_type_num)
end
function account_set_normal_vitality(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\vitality", attr_type_num, value)
end

function account_get_normal_gold(username)
	return api.account_get_attr(username, "Record\\DRTL\\0\\gold", attr_type_num)
end
function account_set_normal_gold(username, value)
	return api.account_set_attr(username, "Record\\DRTL\\0\\gold", attr_type_num, value)
end


--
-- Westwood Online
--

function account_get_wol_apgar(username)
	return api.account_get_attr(username, "Record\\WOL\\auth\\apgar", attr_type_str)
end
function account_set_wol_apgar(username, value)
	return api.account_set_attr(username, "Record\\WOL\\auth\\apgar", attr_type_str, value)
end

function account_get_locale(username)
	return api.account_get_attr(username, "Record\\WOL\\auth\\locale", attr_type_str)
end
function account_set_locale(username, value)
	return api.account_set_attr(username, "Record\\WOL\\auth\\locale", attr_type_str, value)
end



--
-- Warcraft 3 (DotA)
--

function account_get_dotarating_3x3(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_3_rating", attr_type_num)
end
function account_set_dotarating_3x3(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_3_rating", attr_type_num, value)
end

function account_get_dotawins_3x3(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_3_wins", attr_type_num)
end
function account_set_dotawins_3x3(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_3_wins", attr_type_num, value)
end

function account_get_dotalosses_3x3(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_3_losses", attr_type_num)
end
function account_set_dotalosses_3x3(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_3_losses", attr_type_num, value)
end

function account_get_dotastreaks_3x3(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_3_streaks", attr_type_num)
end
function account_set_dotastreaks_3x3(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_3_streaks", attr_type_num, value)
end

function account_get_dotaleaves_3x3(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_3_leaves", attr_type_num)
end
function account_set_dotaleaves_3x3(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_3_leaves", attr_type_num, value)
end


function account_get_dotarating_5x5(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_5_rating", attr_type_num)
end
function account_set_dotarating_5x5(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_5_rating", attr_type_num, value)
end

function account_get_dotawins_5x5(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_5_wins", attr_type_num)
end
function account_set_dotawins_5x5(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_5_wins", attr_type_num, value)
end

function account_get_dotalosses_5x5(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_5_losses", attr_type_num)
end
function account_set_dotalosses_5x5(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_5_losses", attr_type_num, value)
end

function account_get_dotastreaks_5x5(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_5_streaks", attr_type_num)
end
function account_set_dotastreaks_5x5(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_5_streaks", attr_type_num, value)
end

function account_get_dotaleaves_5x5(username)
	return api.account_get_attr(username, "Record\\W3XP\\dota_5_leaves", attr_type_num)
end
function account_set_dotaleaves_5x5(username, value)
	return api.account_set_attr(username, "Record\\W3XP\\dota_5_leaves", attr_type_num, value)
end

function account_get_botping(username)
	value = api.account_get_attr(username, "BNET\\acct\\botping", attr_type_str)
	local pings = {}
	
	-- deserialize and return table
	-- data format: "unixtime,botname,ping;..."
	for chunk in string.split(value,";") do
		local item = {}
		i = 1
		for v in string.split(chunk,",") do
			if (i == 1) then 
				item.date = v
			elseif (i == 2) then 
				item.bot = v
			elseif (i == 3) then 
				item.ping = v
			end
			i = i + 1
		end
		table.insert(pings, item)
	end
	-- sort by ping ascending
	table.sort(pings, function(a,b) return tonumber(a.ping) < tonumber(b.ping) end)
	return pings
end
-- pings is a table that received from account_get_botping()
function account_set_botping(username, pings)
	local value = ""
	-- serialize table
	for k,v in pairs(pings) do
		-- ignore expired pings
		if (os.time() - v.date) < 60*60*24*ghost_ping_expire then
			value = value .. string.format("%s,%s,%s;", v.date, v.bot, v.ping);
		end
	end
	return api.account_set_attr(username, "BNET\\acct\\botping", attr_type_str, value)
end
