add_library(compat 
	access.h gethostname.h gettimeofday.cpp gettimeofday.h inet_aton.cpp 
	inet_aton.h inet_ntoa.cpp inet_ntoa.h mkdir.h mmap.cpp mmap.h 
	netinet_in.h pdir.cpp pdir.h pgetopt.cpp pgetopt.h pgetpid.h psock.cpp 
	psock.h read.h recv.h rename.h send.h snprintf.cpp snprintf.h socket.h 
	statmacros.h stdfileno.h strcasecmp.cpp strcasecmp.h strdup.cpp 
	strdup.h strerror.cpp strerror.h strncasecmp.cpp strncasecmp.h 
	strsep.cpp strsep.h strtoul.cpp strtoul.h termios.h uint.h uname.cpp 
	uname.h vsnprintf.cpp vsnprintf.h)
