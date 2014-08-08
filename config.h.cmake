#ifndef PVPGN_CONFIG_H
#define PVPGN_CONFIG_H

#cmakedefine HAVE_FCNTL_H
#cmakedefine HAVE_SYS_TIME_H
#cmakedefine HAVE_SYS_SELECT_H
#cmakedefine HAVE_UNISTD_H
#cmakedefine HAVE_SYS_UTSNAME_H
#cmakedefine HAVE_SYS_TIMEB_H
#cmakedefine HAVE_SYS_SOCKET_H
#cmakedefine HAVE_SYS_PARAM_H
#cmakedefine HAVE_NETINET_IN_H
#cmakedefine HAVE_ARPA_INET_H
#cmakedefine HAVE_NETDB_H
#cmakedefine HAVE_TERMIOS_H
#cmakedefine HAVE_SYS_TYPES_H
#cmakedefine HAVE_SYS_WAIT_H
#cmakedefine HAVE_SYS_IOCTL_H
#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_SYS_FILE_H
#cmakedefine HAVE_POLL_H
#cmakedefine HAVE_SYS_POLL_H
#cmakedefine HAVE_SYS_STROPTS_H
#cmakedefine HAVE_SYS_STAT_H
#cmakedefine HAVE_PWD_H
#cmakedefine HAVE_GRP_H
#cmakedefine HAVE_DIR_H
#cmakedefine HAVE_DIRENT_H
#cmakedefine HAVE_NDIR_H
#cmakedefine HAVE_SYS_DIR_H
#cmakedefine HAVE_SYS_NDIR_H
#cmakedefine HAVE_DIRECT_H
#cmakedefine HAVE_SYS_MMAN_H
#cmakedefine HAVE_SYS_EVENT_H
#cmakedefine HAVE_SYS_EPOLL_H
#cmakedefine HAVE_SYS_RESOURCE_H
#cmakedefine HAVE_PCAP_H
#cmakedefine HAVE_WINDOWS_H
#cmakedefine HAVE_WINSOCK2_H
#cmakedefine HAVE_PROCESS_H

#cmakedefine SIZEOF_UNSIGNED_CHAR ${SIZEOF_UNSIGNED_CHAR}
#cmakedefine SIZEOF_UNSIGNED_SHORT ${SIZEOF_UNSIGNED_SHORT}
#cmakedefine SIZEOF_UNSIGNED_INT ${SIZEOF_UNSIGNED_INT}
#cmakedefine SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}
#cmakedefine SIZEOF_UNSIGNED_LONG_LONG ${SIZEOF_UNSIGNED_LONG_LONG}
#cmakedefine SIZEOF_SIGNED_CHAR ${SIZEOF_SIGNED_CHAR}
#cmakedefine SIZEOF_SIGNED_SHORT ${SIZEOF_SIGNED_SHORT}
#cmakedefine SIZEOF_SIGNED_INT ${SIZEOF_SIGNED_INT}
#cmakedefine SIZEOF_SIGNED_LONG ${SIZEOF_SIGNED_LONG}
#cmakedefine SIZEOF_SIGNED_LONG_LONG ${SIZEOF_SIGNED_LONG_LONG}

#cmakedefine HAVE_MMAP
#cmakedefine HAVE_GETHOSTNAME
#cmakedefine HAVE_GETTIMEOFDAY
#cmakedefine HAVE_SELECT
#cmakedefine HAVE_SOCKET
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_STRTOUL
#cmakedefine HAVE_INET_ATON
#cmakedefine HAVE_INET_NTOA
#cmakedefine HAVE_UNAME
#cmakedefine HAVE_RECV
#cmakedefine HAVE_SEND
#cmakedefine HAVE_RECVFROM
#cmakedefine HAVE_SENDTO
#cmakedefine HAVE_UNAME
#cmakedefine HAVE_FORK
#cmakedefine HAVE_GETPID
#cmakedefine HAVE_SIGACTION
#cmakedefine HAVE_SIGPROCMASK
#cmakedefine HAVE_SIGADDSET
#cmakedefine HAVE_SETPGID
#cmakedefine HAVE_FTIME
#cmakedefine HAVE_STRCASECMP
#cmakedefine HAVE_STRNCASECMP
#cmakedefine HAVE_STRICMP
#cmakedefine HAVE_STRNICMP
#cmakedefine HAVE_CHDIR
#cmakedefine HAVE_DIFFTIME
#cmakedefine HAVE_STRCHR
#cmakedefine HAVE_STRRCHR
#cmakedefine HAVE_INDEX
#cmakedefine HAVE_RINDEX
#cmakedefine HAVE_WAIT
#cmakedefine HAVE_WAITPID
#cmakedefine HAVE_PIPE
#cmakedefine HAVE_GETENV
#cmakedefine HAVE_IOCTL
#cmakedefine HAVE_SETSID
#cmakedefine HAVE_POLL
#cmakedefine HAVE_GETHOSTBYNAME
#cmakedefine HAVE_GETSERVBYNAME
#cmakedefine HAVE_GETLOGIN
#cmakedefine HAVE_GETPWNAME
#cmakedefine HAVE_GETGRNAM
#cmakedefine HAVE_GETUID
#cmakedefine HAVE_GETGID
#cmakedefine HAVE_SETUID
#cmakedefine HAVE_MKDIR
#cmakedefine HAVE__MKDIR
#cmakedefine MKDIR_TAKES_ONE_ARG 
#cmakedefine HAVE_STRSEP
#cmakedefine HAVE_GETOPT
#cmakedefine HAVE_KQUEUE
#cmakedefine HAVE_SETITIMER
#cmakedefine HAVE_EPOLL_CREATE
#cmakedefine HAVE_GETRLIMIT
#cmakedefine HAVE_VSNPRINTF
#cmakedefine HAVE__VSNPRINTF
#cmakedefine HAVE_SNPRINTF
#cmakedefine HAVE__SNPRINTF
#cmakedefine HAVE_SETPGRP

#cmakedefine BNETD_DEFAULT_CONF_FILE "${BNETD_DEFAULT_CONF_FILE}"
#cmakedefine D2CS_DEFAULT_CONF_FILE "${D2CS_DEFAULT_CONF_FILE}"
#cmakedefine D2DBS_DEFAULT_CONF_FILE "${D2DBS_DEFAULT_CONF_FILE}"

#endif
