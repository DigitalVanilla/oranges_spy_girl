#ifndef _NETINET_ICMP_VAR_H_
#define _NETINET_ICMP_VAR_H_

#ifndef _SYS_SYSCTL_H_
#include <sys/sysctl.h>
#endif

/*
 * Names for ICMP sysctl objects
 */
#define	ICMPCTL_MASKREPL	1	/* allow replies to netmask requests */
#define ICMPCTL_MAXID		2

#define ICMPCTL_NAMES { \
	{ 0, 0 }, \
	{ "maskrepl", CTLTYPE_INT }, \
}

#endif
