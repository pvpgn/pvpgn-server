/*
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
#ifndef INCLUDED_COMMON_VERSION_H
#define INCLUDED_COMMON_VERSION_H

#define WIDEN(quote) WIDEN2(quote)
#define WIDEN2(quote) L##quote

#ifndef PVPGN_VERSION
#define PVPGN_VERSION "1.99.8.0.0-rc1-PRO"
#define PVPGN_VERSIONW WIDEN(PVPGN_VERSION)
#endif

#ifndef PVPGN_SOFTWARE
#define PVPGN_SOFTWARE "PvPGN"
#define PVPGN_SOFTWAREW WIDEN(PVPGN_SOFTWARE)
#endif

#endif
