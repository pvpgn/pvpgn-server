set(COMPAT_SOURCES access.h gethostname.h gettimeofday.cpp gettimeofday.h gmtime_s.cpp gmtime_s.h
	localtime_s.cpp localtime_s.h mkdir.h mmap.cpp mmap.h netinet_in.h pdir.cpp pdir.h
	pgetopt.cpp pgetopt.h pgetpid.h psock.cpp psock.h read.h recv.h
	rename.h send.h socket.h statmacros.h
	stdfileno.h strcasecmp.cpp strcasecmp.h strdup.cpp strdup.h
	strerror.cpp strerror.h strncasecmp.cpp strncasecmp.h strsep.cpp
	strsep.h termios.h uname.cpp uname.h)

add_library(compat STATIC ${COMPAT_SOURCES})

target_link_libraries(compat PRIVATE fmt::fmt)