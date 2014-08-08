--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


-- Config table can be extended here with your own variables
-- values are preloaded from bnetd.conf
config = {
	flood_immunity_users = { "admin", "" }, -- ignore flood protection for these users

	-- Quiz settings
	quiz = true,
	quiz_filelist = "misc, dota, warcraft", -- display available files in "/quiz start"
	quiz_competitive_mode = true, -- top players loses half of points which last player received; at the end top of records loses half of points which players received in current game
	quiz_max_questions = 100, -- from start to end
	quiz_question_delay = 5, -- delay before send next question
	quiz_hint_delay = 20, -- delay between prompts
	quiz_users_in_top = 15, -- how many users display in TOP list
	quiz_channel = nil, -- (do not modify!) channel when quiz has started (it assigned with start)
	
	-- AntiHack (Starcraft)
	ah = true,
	ah_interval = 60, -- interval for send memory request to all players in games

	-- GHost++ (https://github.com/OHSystem/ohsystem)
	ghost = false, -- enable GHost commands
	ghost_bots = { hostbot1, hostbot2 }, -- list of authorized bots
	ghost_dota_server = true, -- replace normal Warcraft 3 stats with DotA
	ghost_ping_expire = 90, -- interval when outdated botpings should be removed (bot ping updates for each user when he join a game hosted by ghost); game list shows to user depending on the best ping to host bot

}

