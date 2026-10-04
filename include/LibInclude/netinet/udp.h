#ifndef _NETINET_UDP_H_
#define _NETINET_UDP_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
struct udphdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	short	uh_ulen;		/* udp length */
	u_short	uh_sum;			/* udp checksum */
};

#endif
