/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ANSI_TERM_PROTOS
#define INCLUDED_ANSI_TERM_PROTOS

#include <cstdio>

#define ansi_beep()\
	std::printf("\007");

#define ansi_screen_clear()\
	std::printf("\033[2J");

#define ansi_line_clear()\
	std::printf("\033[K");

#define ansi_cursor_move_up(lines)\
	std::printf("\033[%dA", lines);

#define ansi_cursor_move_down(lines)\
	std::printf("\033[%dB", lines);

#define ansi_cursor_move_left(chars)\
	std::printf("\033[%dD", chars);

#define ansi_cursor_move_right(chars)\
	std::printf("\033[%dC", chars);

#define ansi_cursor_move_home()\
	std::printf("\033[1;0H");

#define ansi_cursor_move(y,x)\
	std::printf("\033[%d;%dH", x + 1, y);

#define ansi_cursor_save()\
	std::printf("\033[u");

#define ansi_cursor_load()\
	std::printf("\033[s");

#define ansi_text_reset()\
	std::printf("\033[0m");

#define ansi_text_style(style)\
	std::printf("\033[%dm", style);

#define ansi_text_color_fore(color)\
	std::printf("\033[3%dm", color);

#define ansi_text_color_back(color)\
	std::printf("\033[4%dm", color);

#define ansi_text_style_bold      1
#define ansi_text_style_underline 4
#define ansi_text_style_blink     5
#define ansi_text_style_inverse   7

#define ansi_text_color_black     0
#define ansi_text_color_red       1
#define ansi_text_color_green     2
#define ansi_text_color_yellow    3
#define ansi_text_color_blue      4
#define ansi_text_color_magenta   5
#define ansi_text_color_cyan      6
#define ansi_text_color_white     7

#endif
#endif
