#ifndef _NETINET_UDP_VAR_H_
#define _NETINET_UDP_VAR_H_

#ifndef _SYS_SYSCTL_H_
#include <sys/sysctl.h>
#endif

/*
 * Names for UDP sysctl objects
 */
#define	UDPCTL_CHECKSUM		1	/* checksum UDP packets */
#define UDPCTL_STATS		2	/* statistics (read-only) */
#define	UDPCTL_MAXDGRAM		3	/* max datagram size */
#define	UDPCTL_RECVSPACE	4	/* default receive buffer space */
#define UDPCTL_MAXID		5

#define UDPCTL_NAMES { \
	{ 0, 0 }, \
	{ "checksum", CTLTYPE_INT }, \
	{ "stats", CTLTYPE_STRUCT }, \
	{ "maxdgram", CTLTYPE_INT }, \
	{ "recvspace", CTLTYPE_INT }, \
}

#endif
