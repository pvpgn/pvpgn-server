--[[
	Copyright (C) 2014 HarpyWar (harpywar@gmail.com)
	
	This file is a part of the PvPGN Project http://pvpgn.pro
	Licensed under the same terms as Lua itself.
]]--


quiz = {} -- object
q_dictionary = {} -- dictionary word=question
q_records_current = { } -- table with current player records
q_records_total = {} -- table with total player records
q_records_diff = {} -- table with current diffs

-- counter of questions
local _q_question_counter = 0
-- count of current question's hints
local _q_hint_counter = 0
-- count of current question's hints
local _q_current_index = 0
-- current hint
local _q_hint = nil
-- time when question was shown
local _q_time_start = 0
-- last user streaks
local _q_streak = { username = nil, count = 0 }

-- delay flag for next_question timer
local _q_next_question_skip_first = false
-- is question unswered?
local q_unswered = true

function quiz:start(channelname, quizname)
	local filename = q_directory() .. "/questions/" .. string.lower(quizname) .. ".txt"
	
	if not file_exists(filename) then
		channel_send_message(config.quiz_channel, filename, message_type_error)
		return false
	end
	
	config.quiz_channel = channelname
	
	-- reset
	_q_question_counter = 0
	_q_streak.username = nil
	_q_streak.count = 0
	-- clear tables
	table.clear(q_dictionary)
	table.clear(q_records_current)
	table.clear(q_records_diff)
	-- table.clear(q_records_total) -- do not clear it, it should be cached

	-- load records (if it was not loaded yet)
	q_load_records()
	
	-- fill dictionary from file
	if file_load(filename, file_load_dictionary_callback, q_read_dictionary_callback) then
		channel_send_message(config.quiz_channel, string.format("Quiz \"%s\" started!", quizname), message_type_error)
		q_next_question()
	end
end
function q_read_dictionary_callback(a, b)
	table.insert(q_dictionary, { question = a, word = b })
end


function quiz:stop(username)

	timer_del("q_hint")
	timer_del("q_question")
	
	local output = nil
	if username then
		output = "Quiz stopped by "..username
	else
		output = "Quiz finished!"
	end
	channel_send_message(config.quiz_channel, output, message_type_error)
	
	-- display current records
	if (q_records_current) and next(q_records_current) then
		table.sort(q_records_current, q_compare_desc)
		
		channel_send_message(config.quiz_channel, "Records of this game:", message_type_info)
		for i,t in pairs(q_records_current) do
			channel_send_message(config.quiz_channel, string.format("  %d. %s [%d points]", i, t.username, t.points), message_type_info)
		end
	end

	-- merge current with total records
	if (q_records_total) and next(q_records_total) then
		for i,t in pairs(q_records_current) do

			-- steal points from players in total table accordingly with half points of current table players
			if (config.quiz_competitive_mode) then
				if q_records_total[i] and not (q_records_total[i].username == t.username) then
					-- remove half points
					local points = math.floor(t.points / 2)
					q_records_diff[q_records_total[i].username] = points * -1
					q_records_total[i].points = q_records_total[i].points - points
					-- avoid negative value
					if q_records_total[i].points <= 0 then
						q_records_total[i].points = 0
					end
				end
			end

			-- add non exist users from current to total table
			-- and increase total users points
			local idx = q_records_total_find(t.username)
			-- set diff as winned points
			local tusername = q_records_total[idx].username
			q_records_diff[tusername] = t.points
			
			-- if user not found (new user with 0 points)
			if q_records_total[idx].points == 0 then 
				q_records_total[idx].points = t.points
			else
				-- update diff if it was changed to negative
				if q_records_diff[tusername] < 0 then
					q_records_diff[tusername] = q_records_diff[tusername] + t.points
				end
				q_records_total[idx].points = q_records_total[idx].points + t.points
			end
		end
		
		-- sort by points
		table.sort(q_records_total, q_compare_desc)

		quiz_display_top_players()
	else
		-- if records.txt is empty then save first records
		q_records_total = q_records_current
	end
	q_save_records()

	config.quiz_channel = nil
end





-- handle channel message
function quiz_handle_message(username, text)
	if q_unswered then return end

	local q = q_dictionary[_q_current_index]
	
	if string.upper(q.word) == string.upper(text) then
		-- time from question to unswer
		local time_diff = os.clock() - _q_time_start
		
		local points, total = 0,0
		-- calc points
		points = 1+(#q.word - _q_hint_counter) - math.floor(time_diff / config.quiz_hint_delay)
		-- avoid negative value
		if (points <= 0) then points = 1 end
		
		-- remember previous streak
		local prev_streak = _q_streak
		-- increase streaks
		if (_q_streak.username == username) then
			_q_streak.count = _q_streak.count + 1
		else
			_q_streak.username = username
			_q_streak.count = 0
		end
		
		local bonus = ""
		-- add bonus points for streaks
		if (_q_streak.count > 0) then
			points = points + _q_streak.count
			bonus = string.format(" +%s streak bonus", _q_streak.count)
		end
		
		local idx = q_records_current_find(username)
		total = q_records_current[idx].points + points
		q_records_current[idx].points = total
		
		-- lose half points for previous user
		if config.quiz_competitive_mode and not (prev_streak.username == username) then
			local lose_points = math.floor(total / 2)
			local idx = q_records_current_find(prev_streak.username)
			q_records_current[idx].points = q_records_current[idx].points - lose_points
			-- avoid negative value
			if q_records_current[idx].points <= 0 then
				q_records_current[idx].points = 0
			end
		end
		
		channel_send_message(config.quiz_channel, string.format("%s is correct! The unswer is: %s (+%d points%s, %d total) [%d sec]", username, q.word, (points-_q_streak.count), bonus, total, time_diff), message_type_info)
		
		q_next_question()
	end

end

-- go to next question
function q_next_question()
	q_unswered = true

	_q_question_counter = _q_question_counter + 1
	if (_q_question_counter > config.quiz_max_questions) then
		quiz:stop()
		return 0
	end
	
	timer_del("q_hint")

	_q_next_question_skip_first = false
	timer_add("q_question", config.quiz_question_delay, q_tick_next_question)
end

-- write random question
function q_tick_next_question(options)	
	-- skip first tick
	if not _q_next_question_skip_first then
		_q_next_question_skip_first = true
		return 0
	end
	_q_time_start = os.clock()
	_q_current_index = math.random(#q_dictionary)
	
	local q = q_dictionary[_q_current_index]
	-- send question
	channel_send_message(config.quiz_channel, "--------------------------------------------------------", message_type_info)
	channel_send_message(config.quiz_channel, string.format(" %s (%d letters)", q.question, #q.word), message_type_info)
	
	-- hide hint
	_q_hint = q_hide_unswer(q.word)

	-- stop this timer
	timer_del(options.id)
	
	_q_hint_counter = 0
	-- start timer to hint unswer
	timer_add("q_hint", config.quiz_hint_delay, q_tick_hint_unswer)
	
	q_unswered = false
end

-- write hint for unswer
function q_tick_hint_unswer(options)
	-- skip first tick
	if _q_hint_counter == 0 then
		_q_hint_counter = _q_hint_counter + 1
		return 0
	end
	_q_hint_counter = _q_hint_counter + 1

	local q = q_dictionary[_q_current_index]
	-- update hint
	_q_hint = q_show_next_symbol(_q_hint, q.word)
	
	if not (_q_hint == q.word) then
		-- show hint
		channel_send_message(config.quiz_channel, "Hint: ".._q_hint, message_type_info)
	else
		-- reset counter
		_q_hint_counter = 0

		-- show unswer
		q_nounswer(nil)
	end
	
end

-- nobody unswered
function q_nounswer(username)
	channel_send_message(config.quiz_channel, 'Nobody unswered. The unswer was: '.._q_hint, message_type_info)
	
	-- decrease streaks
	if (_q_streak.count > 0) then
		_q_streak.count = _q_streak.count - 1
	end
	
	q_next_question()
end


