#ifndef _BSDSOCKET_MIAMISYSCTL_H_
#define _BSDSOCKET_MIAMISYSCTL_H_

#ifndef _SYS_SYSCTL_H_
#include <sys/sysctl.h>
#endif

#define DNS_MAXID	1

#define CTL_DNS_NAMES { \
	{"cache",CTLTYPE_NODE }, \
	{ 0, 0 }, \
}

#define DNSCACHE_MAXID	3

#define CTL_DNSCACHE_NAMES { \
	{"size",CTLTYPE_INT }, \
	{"flush",CTLTYPE_INT }, \
	{"enabled",CTLTYPE_INT }, \
	{ 0, 0 }, \
}

#define INETD_MAXID	5

#define CTL_INETD_NAMES { \
	{"retrytime",CTLTYPE_INT }, \
	{"toomany",CTLTYPE_INT }, \
	{"cntintvl",CTLTYPE_INT }, \
	{"maxbuiltin",CTLTYPE_INT }, \
	{"processpri",CTLTYPE_INT }, \
	{ 0, 0 }, \
}

#define SOCKET_MAXID	1

#define CTL_SOCKET_NAMES { \
	{"maxqlen",CTLTYPE_INT }, \
	{ 0, 0 }, \
}

#endif
