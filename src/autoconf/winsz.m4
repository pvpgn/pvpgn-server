# AM_HEADER_TIOCGWINSZ_NEEDS_SYS_IOCTL

# Copyright 1996, 1998, 2000, 2001 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# From Jim Meyering.

# serial 1

AC_DEFUN([AM_HEADER_TIOCGWINSZ_NEEDS_SYS_IOCTL],
[AC_REQUIRE([AM_SYS_POSIX_TERMIOS])
 AC_CACHE_CHECK([whether use of TIOCGWINSZ requires sys/ioctl.h],
	        am_cv_sys_tiocgwinsz_needs_sys_ioctl_h,
  [am_cv_sys_tiocgwinsz_needs_sys_ioctl_h=no

  gwinsz_in_termios_h=no
  if test $am_cv_sys_posix_termios = yes; then
    AC_EGREP_CPP([yes],
    [#include <sys/types.h>
#     include <termios.h>
#     ifdef TIOCGWINSZ
        yes
#     endif
    ], gwinsz_in_termios_h=yes)
  fi

  if test $gwinsz_in_termios_h = no; then
    AC_EGREP_CPP([yes],
    [#include <sys/types.h>
#     include <sys/ioctl.h>
#     ifdef TIOCGWINSZ
        yes
#     endif
    ], am_cv_sys_tiocgwinsz_needs_sys_ioctl_h=yes)
  fi
  ])
  if test $am_cv_sys_tiocgwinsz_needs_sys_ioctl_h = yes; then
    AC_DEFINE(GWINSZ_IN_SYS_IOCTL,1,
              [Define if TIOCGWINSZ requires sys/ioctl.h])
  fi
])
