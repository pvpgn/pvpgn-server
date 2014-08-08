#ifndef PVPGN_CONFIG_H
#define PVPGN_CONFIG_H

#define HAVE_FCNTL_H
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_UTSNAME_H */
#define HAVE_SYS_TIMEB_H
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_SYS_PARAM_H */
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_ARPA_INET_H */
/* #undef HAVE_NETDB_H */
/* #undef HAVE_TERMIOS_H */
#define HAVE_SYS_TYPES_H
/* #undef HAVE_SYS_WAIT_H */
/* #undef HAVE_SYS_IOCTL_H */
#define HAVE_STDINT_H
/* #undef HAVE_SYS_FILE_H */
/* #undef HAVE_POLL_H */
/* #undef HAVE_SYS_POLL_H */
/* #undef HAVE_SYS_STROPTS_H */
#define HAVE_SYS_STAT_H
/* #undef HAVE_PWD_H */
/* #undef HAVE_GRP_H */
/* #undef HAVE_DIR_H */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_SYS_NDIR_H */
#define HAVE_DIRECT_H
/* #undef HAVE_SYS_MMAN_H */
/* #undef HAVE_SYS_EVENT_H */
/* #undef HAVE_SYS_EPOLL_H */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_PCAP_H */
#define HAVE_WINDOWS_H
#define HAVE_WINSOCK2_H
#define HAVE_PROCESS_H

#define SIZEOF_UNSIGNED_CHAR 1
#define SIZEOF_UNSIGNED_SHORT 2
#define SIZEOF_UNSIGNED_INT 4
#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#define SIZEOF_SIGNED_CHAR 1
#define SIZEOF_SIGNED_SHORT 2
#define SIZEOF_SIGNED_INT 4
#define SIZEOF_SIGNED_LONG 4
#define SIZEOF_SIGNED_LONG_LONG 8

/* #undef HAVE_MMAP */
#define HAVE_GETHOSTNAME
/* #undef HAVE_GETTIMEOFDAY */
#define HAVE_SELECT
#define HAVE_SOCKET
#define HAVE_STRDUP
#define HAVE_STRTOUL
/* #undef HAVE_INET_ATON */
#define HAVE_INET_NTOA
/* #undef HAVE_UNAME */
#define HAVE_RECV
#define HAVE_SEND
#define HAVE_RECVFROM
#define HAVE_SENDTO
/* #undef HAVE_UNAME */
/* #undef HAVE_FORK */
#define HAVE_GETPID
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SIGPROCMASK */
/* #undef HAVE_SIGADDSET */
/* #undef HAVE_SETPGID */
#define HAVE_FTIME
/* #undef HAVE_STRCASECMP */
/* #undef HAVE_STRNCASECMP */
#define HAVE_STRICMP
#define HAVE_STRNICMP
#define HAVE_CHDIR
#define HAVE_DIFFTIME
#define HAVE_STRCHR
#define HAVE_STRRCHR
/* #undef HAVE_INDEX */
/* #undef HAVE_RINDEX */
/* #undef HAVE_WAIT */
/* #undef HAVE_WAITPID */
/* #undef HAVE_PIPE */
#define HAVE_GETENV
/* #undef HAVE_IOCTL */
/* #undef HAVE_SETSID */
/* #undef HAVE_POLL */
#define HAVE_GETHOSTBYNAME
#define HAVE_GETSERVBYNAME
/* #undef HAVE_GETLOGIN */
/* #undef HAVE_GETPWNAME */
/* #undef HAVE_GETGRNAM */
/* #undef HAVE_GETUID */
/* #undef HAVE_GETGID */
/* #undef HAVE_SETUID */
#define HAVE_MKDIR
#define HAVE__MKDIR
#define MKDIR_TAKES_ONE_ARG 
/* #undef HAVE_STRSEP */
/* #undef HAVE_GETOPT */
/* #undef HAVE_KQUEUE */
/* #undef HAVE_SETITIMER */
/* #undef HAVE_EPOLL_CREATE */
/* #undef HAVE_GETRLIMIT */
#define HAVE_VSNPRINTF
#define HAVE__VSNPRINTF
/* #undef HAVE_SNPRINTF */
#define HAVE__SNPRINTF
/* #undef HAVE_SETPGRP */

#define BNETD_DEFAULT_CONF_FILE "conf/bnetd.conf"
#define D2CS_DEFAULT_CONF_FILE "conf/d2cs.conf"
#define D2DBS_DEFAULT_CONF_FILE "conf/d2dbs.conf"

#endif
