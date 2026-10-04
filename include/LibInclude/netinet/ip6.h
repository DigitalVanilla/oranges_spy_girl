#ifndef _NETINET_IP6_H_
#define _NETINET_IP6_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _NETINET_IN_H_
#include <netinet/in.h>
#endif

/*
 * Definitions for internet protocol version 6.
 * Per RFC 1883
 */

/*
 * Structure of an IPv6 (first) header.
 */

struct ipv6 {
	u_int32_t ip6_head;		/* version and flow label */
	u_int16_t ip6_len;		/* payload length */
	u_int8_t  ip6_nh;		/* next header */
	u_int8_t  ip6_hlim;		/* hop limit */
	struct    in6_addr ip6_src;	/* source address */
	struct	  in6_addr ip6_dst;	/* destination address */
};

#define IP6_MMTU	576		/* minimal MTU and reassembly */

/*
 * Definitions of next header field values.
 */

#define IP6_NHDR_HOP	0	/* hop-by-hop IPv6 header */
#define IP6_NHDR_RT	43	/* routing IPv6 header */
#define IP6_NHDR_FRAG	44	/* fragment IPv6 header */
#define IP6_NHDR_AUTH	51	/* authentication IPv6 header */
#define IP6_NHDR_ESP	50	/* encryption IPv6 header */
#define IP6_NHDR_DOPT	60	/* destination options IPv6 header */
#define IP6_NHDR_NONH	59	/* no next header */

/*
 * Fragment Header.
 */

struct ipv6_fraghdr {
	u_int8_t  if6_nh;	/* next header */
	u_int8_t  if6_res;	/* reserved */
	u_int16_t if6_off;	/* offset */
#define IP6_MF 0x1		/* more flag */
#define IP6_OFFMASK 0xfff8	/* mask of real offset field */
	u_int32_t if6_id;	/* identifier */
};

/*
 * Hop-by-Hop Options Header.
 */

struct ipv6_h2hhdr {
	u_int8_t  ih6_nh;	/* next header */
	u_int8_t  ih6_hlen;	/* header extension length */
	u_int16_t ih6_pad1;	/* to 4 byte length */
	u_int32_t ih6_pad2;	/* to 8 byte length */
};

/*
 * Routing Header.
 */

struct ipv6_rthdr {
	u_int8_t  ir6_nh;	/* next header */
	u_int8_t  ir6_hlen;	/* header extension length */
	u_int8_t  ir6_type;	/* routing type */
#define IP6_LSRRT 0		/* type 0: loose source route */
	u_int8_t  ir6_sglt;	/* index of next address */
	u_int32_t ir6_slmsk;	/* strict/loose bit mask */
};
#define IP6_RT_MAX	23	/* maximum number of addresses */
#define IP6_RT_SLMSK	0x00ffffff
#define IP6_RT_SLBIT(n)	(1 << (IP6_RT_MAX - n))

/*
 * Destination Options Header.
 */

struct ipv6_dopthdr {
	u_int8_t  io6_nh;	/* next header */
	u_int8_t  io6_hlen;	/* header extension length */
	u_int16_t io6_pad1;	/* to 4 byte length */
	u_int32_t io6_pad2;	/* to 8 byte length */
};

/*
 * Authentication Header.
 */

struct ipv6_authhdr {
	u_int8_t  ah6_nh;	/* next header */
	u_int8_t  ah6_hlen;	/* header extension length */
	u_int16_t ah6_pad;	/* to 4 byte length */
	u_int32_t ah6_spi;	/* security parameter index */
};

/*
 * Encryption Security Payload Header.
 */

struct ipv6_esphdr {
	u_int32_t esp6_spi;	/* security parameter index */
};

#define IP6_OPT_PAD1	0	/* one-byte pad option type */
#define IP6_OPT_PADN	1	/* N-byte pad option type */

/*
 * IPv6 implementation parameters.
 */

#define	IP6FRAGTTL	120		/* time to live for frags, slowhz */

#endif




