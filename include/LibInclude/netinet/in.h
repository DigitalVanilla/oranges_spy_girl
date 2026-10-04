#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _SYS_SYSCTL_H_
#include <sys/sysctl.h>
#endif

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, and numerous additions.
 */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_HOPOPTS		0		/* IPv6 Hop-by-Hop options */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_IGMP		2		/* group mgmt protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define IPPROTO_IPIP		4 		/* IP encapsulation in IP */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */
#define	IPPROTO_TP		29 		/* tp-4 w/ class negotiation */
#define IPPROTO_IPV6		41		/* IPv6 into IPv4 */
#define	IPPROTO_ROUTING		43		/* IPv6 Routing header */
#define	IPPROTO_FRAGMENT	44		/* IPv6 fragmentation header */
#define IPPROTO_RSVP		46 		/* resource reservation */
#define	IPPROTO_ESP		50		/* encap security payload */
#define	IPPROTO_AH		51		/* authentication header */
#define IPPROTO_ICMPV6		58		/* ICMPv6/IGMPv6/NDPv6 */
#define	IPPROTO_NONE		59		/* IPv6 no next header */
#define	IPPROTO_DSTOPTS		60		/* IPv6 Destination options */
#define	IPPROTO_EON		80		/* ISO cnlp */
#define	IPPROTO_ENCAP		98		/* encapsulation header */

#define	IPPROTO_DIVERT		254		/* divert pseudo-protocol */
#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Local port number conventions:
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	u_long s_addr;
};

#define INET_ADDRSTRLEN		16

/*
 * IPv6 address (a structure for 64 bit architectures)
 */
struct in6_addr {
	union {
		u_int32_t u6_addr32[4];
		u_int8_t  u6_addr8[16];
	} u6_addr;
};
#define s6_addr32	u6_addr.u6_addr32
#define s6_addr8	u6_addr.u6_addr8
#define s6_addr		u6_addr.u6_addr8

#define INET6_ADDRSTRLEN	46

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((long)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((long)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((long)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((long)(i) & 0xf0000000) == 0xe0000000)
#define	IN_CLASSD_NET		0xf0000000	/* These ones aren't really */
#define	IN_CLASSD_NSHIFT	28		/* net and host fields, but */
#define	IN_CLASSD_HOST		0x0fffffff	/* routing needn't know.    */
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((long)(i) & 0xf0000000) == 0xf0000000)
#define	IN_BADCLASS(i)		(((long)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(u_long)0x00000000
#define	INADDR_BROADCAST	(u_long)0xffffffff	/* must be masked */
#define	INADDR_NONE		0xffffffff		/* -1 return */

#define	INADDR_UNSPEC_GROUP	(u_long)0xe0000000	/* 224.0.0.0 */
#define	INADDR_ALLHOSTS_GROUP	(u_long)0xe0000001	/* 224.0.0.1 */
#define	INADDR_ALLRTRS_GROUP	(u_long)0xe0000002	/* 224.0.0.2 */
#define	INADDR_MAX_LOCAL_GROUP	(u_long)0xe00000ff	/* 224.0.0.255 */

#define	IN_LOOPBACKNET		127			/* official! */

#define IN6ADDR_ANY_INIT	{0, 0, 0, 0}
#define IPV6ADDR_ANY_INIT	IN6ADDR_ANY_INIT	/* old name */
#if BYTE_ORDER == BIG_ENDIAN
#define IN6ADDR_LOOPBACK_INIT	{0, 0, 0, 1}
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define IN6ADDR_LOOPBACK_INIT	{0, 0, 0, 0x01000000}
#endif
#define IPV6ADDR_LOOPBACK_INIT	IN6ADDR_LOOPBACK_INIT	/* old name */

extern const struct in6_addr in6addr_any, in6addr_loopback;

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	u_char	sin_len;
	u_char	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

/*
 * IPv6 socket address
 */
#define	SIN6_LEN

struct sockaddr_in6 {
	u_int8_t  sin6_len;
	u_int8_t  sin6_family;
	u_int16_t sin6_port;
	u_int32_t sin6_flowinfo;
	struct	  in6_addr sin6_addr;
};

#define IPNGVERSION	6

/*
 * IPv6 routing socket pseudo-address
 */

struct sockaddr_rt6 {
	u_int8_t  sin6_len;
	u_int8_t  sin6_family;
	u_int16_t sin6_rtpad1;
	u_int16_t sin6_rtpad2;
	u_int16_t sin6_local;
	struct	  in6_addr sin6_addr;
};

/*
 * Definitions for flow labels.
 */

#if BYTE_ORDER == BIG_ENDIAN
#define IPV6_FLOWINFO_FLOWLABEL		0x00ffffff
#define IPV6_FLOWINFO_PRIORITY		0x0f000000
#define IPV6_FLOWINFO_PRIFLOW		0x0fffffff
#define IPV6_FLOWINFO_SRFLAG		0x10000000
#define IPV6_FLOWINFO_VERSION		0xf0000000

#define IPV6_PRIORITY_UNCHARACTERIZED	0x00000000
#define IPV6_PRIORITY_FILLER		0x01000000
#define IPV6_PRIORITY_UNATTENDED	0x02000000
#define IPV6_PRIORITY_RESERVED1		0x03000000
#define IPV6_PRIORITY_BULK		0x04000000
#define IPV6_PRIORITY_RESERVED2		0x05000000
#define IPV6_PRIORITY_INTERACTIVE	0x06000000
#define IPV6_PRIORITY_CONTROL		0x07000000
#define IPV6_PRIORITY_8			0x08000000
#define IPV6_PRIORITY_9			0x09000000
#define IPV6_PRIORITY_10		0x0a000000
#define IPV6_PRIORITY_11		0x0b000000
#define IPV6_PRIORITY_12		0x0c000000
#define IPV6_PRIORITY_13		0x0d000000
#define IPV6_PRIORITY_14		0x0e000000
#define IPV6_PRIORITY_15		0x0f000000

#define IPV6_SRFLAG_STRICT		0x10000000
#define IPV6_SRFLAG_LOOSE		0x00000000

#define IPV6_VERSION			0x60000000

#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define IPV6_FLOWINFO_FLOWLABEL		0xffffff00
#define IPV6_FLOWINFO_PRIORITY		0x0000000f
#define IPV6_FLOWINFO_PRIFLOW		0xffffff0f
#define IPV6_FLOWINFO_SRFLAG		0x00000010
#define IPV6_FLOWINFO_VERSION		0x000000f0

#define IPV6_PRIORITY_UNCHARACTERIZED	0x00000000
#define IPV6_PRIORITY_FILLER		0x00000001
#define IPV6_PRIORITY_UNATTENDED	0x00000002
#define IPV6_PRIORITY_RESERVED1		0x00000003
#define IPV6_PRIORITY_BULK		0x00000004
#define IPV6_PRIORITY_RESERVED2		0x00000005
#define IPV6_PRIORITY_INTERACTIVE	0x00000006
#define IPV6_PRIORITY_CONTROL		0x00000007
#define IPV6_PRIORITY_8			0x00000008
#define IPV6_PRIORITY_9			0x00000009
#define IPV6_PRIORITY_10		0x0000000a
#define IPV6_PRIORITY_11		0x0000000b
#define IPV6_PRIORITY_12		0x0000000c
#define IPV6_PRIORITY_13		0x0000000d
#define IPV6_PRIORITY_14		0x0000000e
#define IPV6_PRIORITY_15		0x0000000f

#define IPV6_SRFLAG_STRICT		0x00000010
#define IPV6_SRFLAG_LOOSE		0x00000000

#define IPV6_VERSION			0x00000060

#endif

#define IPV6_GET_FLOWLABEL(x)		(ntohl(x) & 0x00ffffff)
#define IPV6_GET_PRIORITY(x)		((ntohl(x) >> 24) & 0xf)
#define IPV6_GET_VERSION(x)		((ntohl(x) >> 28) & 0xf)
#define IPV6_SET_FLOWLABEL(x)		(htonl(x) & IPV6_FLOWINFO_FLOWLABEL)
#define IPV6_SET_PRIORITY(x)		(htonl((x & 0xf) << 24))

/*
 * Definitions for IPv6 addresses.
 */

#define CLR_ADDR6(a) \
	{ \
		(a).s6_addr32[0] = 0; \
		(a).s6_addr32[1] = 0; \
		(a).s6_addr32[2] = 0; \
		(a).s6_addr32[3] = 0; \
	}

#define COPY_ADDR6(from, to) \
	{ \
		(to).s6_addr32[0] = (from).s6_addr32[0]; \
		(to).s6_addr32[1] = (from).s6_addr32[1]; \
		(to).s6_addr32[2] = (from).s6_addr32[2]; \
		(to).s6_addr32[3] = (from).s6_addr32[3]; \
	}

#define SAME_SOCKADDR(a, b) \
	(((a)->sin6_addr.s6_addr32[0] == (b)->sin6_addr.s6_addr32[0]) && \
	 ((a)->sin6_addr.s6_addr32[1] == (b)->sin6_addr.s6_addr32[1]) && \
	 ((a)->sin6_addr.s6_addr32[2] == (b)->sin6_addr.s6_addr32[2]) && \
	 ((a)->sin6_addr.s6_addr32[3] == (b)->sin6_addr.s6_addr32[3]))

#define SAME_ADDR6(a, b) \
	(((a).s6_addr32[0] == (b).s6_addr32[0]) && \
	 ((a).s6_addr32[1] == (b).s6_addr32[1]) && \
	 ((a).s6_addr32[2] == (b).s6_addr32[2]) && \
	 ((a).s6_addr32[3] == (b).s6_addr32[3]))

#define IS_ANYSOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0) && \
	 ((a)->sin6_addr.s6_addr32[3] == 0))

#define IS_ANYADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0) && \
	 ((a).s6_addr32[3] == 0))

#define IS_COMPATSOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0) && \
	 ((a)->sin6_addr.s6_addr8[12] != 0))

#define IS_COMPATADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0) && \
	 ((a).s6_addr8[12] != 0))

#if BYTE_ORDER == BIG_ENDIAN
#define IS_LOOPSOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0) && \
	 ((a)->sin6_addr.s6_addr32[3] == 1))

#define IS_LOOPADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0) && \
	 ((a).s6_addr32[3] == 1))

#define IS_IPV4SOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0x0000ffff))

#define IS_IPV4ADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0x0000ffff))
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define IS_LOOPSOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0) && \
	 ((a)->sin6_addr.s6_addr32[3] == 0x1000000))

#define IS_LOOPADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0) && \
	 ((a).s6_addr32[3] == 0x1000000))

#define IS_IPV4SOCKADDR(a) \
	(((a)->sin6_addr.s6_addr32[0] == 0) && \
	 ((a)->sin6_addr.s6_addr32[1] == 0) && \
	 ((a)->sin6_addr.s6_addr32[2] == 0xffff0000))

#define IS_IPV4ADDR6(a) \
	(((a).s6_addr32[0] == 0) && \
	 ((a).s6_addr32[1] == 0) && \
	 ((a).s6_addr32[2] == 0xffff0000))
#endif

#define IS_LOCALADDR6(a)	((a).s6_addr8[0] == 0xfe)

#define IS_LINKLADDR6(a) \
	(IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0x80))

#define IS_SITELADDR6(a) \
	(IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0xc0))

#define IS_MULTIADDR6(a)	((a).s6_addr8[0] == 0xff)

#define MADDR6_FLAGS(a)		((a).s6_addr8[1] >> 4)

#define MADDR6_FLG_WK		0	/* "well-known" multicast address */
#define MADDR6_FLG_TS		1	/* "transient" multicast address */

#define MADDR6_SCOPE(a)		((a).s6_addr8[1] & 0x0f)

#define MADDR6_SCP_NODE		0x1	/* node-local scope */
#define MADDR6_SCP_LINK		0x2	/* link-local scope */
#define MADDR6_SCP_SITE		0x5	/* site-local scope */
#define MADDR6_SCP_ORG		0x8	/* organization-local scope */
#define MADDR6_SCP_GLO		0xe	/* global scope */

#define MADDR6_ALLNODES		1	/* all-nodes groups */
#define MADDR6_ALLROUTERS	2	/* all-routers groups */
#define MADDR6_ALLHOSTS		3	/* all-hosts groups */

/*
 * New IPv6 address macros of BSD API
 */

#define IN6_IS_ADDR_UNSPECIFIED(p)	IS_ANYADDR6(*p)
#define IN6_IS_ADDR_LOOPBACK(p)		IS_LOOPADDR6(*p)
#define IN6_IS_ADDR_MULTICAST(p)	IS_MULTIADDR6(*p)
#define IN6_IS_ADDR_LINKLOCAL(p)	IS_LINKLADDR6(*p)
#define IN6_IS_ADDR_SITELOCAL(p)	IS_SITELADDR6(*p)
#define IN6_IS_ADDR_V4MAPPED(p)		IS_IPV4ADDR6(*p)
#define IN6_IS_ADDR_V4COMPAT(p)		IS_COMPATADDR6(*p)

#define IN6_IS_ADDR_MC_NODELOCAL(p) \
	(IS_MULTIADDR6(*p) && (MADDR6_SCOPE(*p) == MADDR6_SCP_NODE))
#define IN6_IS_ADDR_MC_LINKLOCAL(p) \
	(IS_MULTIADDR6(*p) && (MADDR6_SCOPE(*p) == MADDR6_SCP_LINK))
#define IN6_IS_ADDR_MC_SITELOCAL(p) \
	(IS_MULTIADDR6(*p) && (MADDR6_SCOPE(*p) == MADDR6_SCP_SITE))
#define IN6_IS_ADDR_MC_ORGLOCAL(p) \
	(IS_MULTIADDR6(*p) && (MADDR6_SCOPE(*p) == MADDR6_SCP_ORG))
#define IN6_IS_ADDR_MC_GLOBAL(p) \
	(IS_MULTIADDR6(*p) && (MADDR6_SCOPE(*p) == MADDR6_SCP_GLO))

#define IN6_ARE_ADDR_EQUAL(pa,pb)	SAME_ADDR6(*pa,*pb)

/* My own extension (:-) */

#define IN6_ADDR_V6_TO_V4(a)	((struct in_addr *)&(a)->s6_addr[12])

/* Grabbed from Girish's, Tom Pusateri's & Yixin Jin's code */
/*
 * Multicast downstream destinations
 */

struct ds_in6addr {
	struct	in6_addr sin6_addr;	/* downstream destination */
	u_short	hoplimit;		/* min ttl required to forward */
	u_short	flags;			/* attributes */
};

struct sockaddr_inds {
	u_char	sin_len;
	u_char	sin_family;
	u_short	sin_num;
	char	sin_data[12];
};

/*
 * Structure used to describe IP options.
 * Used to store options internally, to pass them to a process,
 * or to restore options retrieved earlier.
 * The ip_dst is used for the first-hop gateway when using a source route
 * (this gets put into the header proper).
 */
struct ip_opts {
	struct	in_addr ip_dst;		/* first hop, 0 w/o src rt */
	char	ip_opts[40];		/* actually variable in size */
};

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define	IP_OPTIONS		1    /* buf/ip_opts; set/get IP options */
#define	IP_HDRINCL		2    /* int; header is included with data */
#define	IP_TOS			3    /* int; IP type of service and preced. */
#define	IP_TTL			4    /* int; IP time to live */
#define	IP_UNICAST_HOPS		IP_TTL
#define	IPV6_UNICAST_HOPS	IP_TTL
#define	IP_RECVOPTS		5    /* bool; receive all IP opts w/dgram */
#define	IP_RECVRETOPTS		6    /* bool; receive IP opts for response */
#define	IP_RECVDSTADDR		7    /* bool; receive IP dst addr w/dgram */
#define	IP_RETOPTS		8    /* ip_opts; set/get IP options */
#define	IP_MULTICAST_IF		9    /* u_char; set/get IP multicast i/f  */
#define	IPV6_MULTICAST_IF	IP_MULTICAST_IF	/* u_int */
#define	IP_MULTICAST_TTL	10   /* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_HOPS	IP_MULTICAST_TTL
#define IPV6_MULTICAST_HOPS	IP_MULTICAST_TTL	/* int */
#define	IP_MULTICAST_LOOP	11   /* u_char; set/get IP multicast loopback */
#define	IPV6_MULTICAST_LOOP	IP_MULTICAST_LOOP	/* u_int */
#define	IP_ADD_MEMBERSHIP	12   /* ip_mreq; add an IP group membership */
#define	IPV6_ADD_MEMBERSHIP	IP_ADD_MEMBERSHIP
#define	IP_DROP_MEMBERSHIP	13   /* ip_mreq; drop an IP group membership */
#define	IPV6_DROP_MEMBERSHIP	IP_DROP_MEMBERSHIP
#define IP_MULTICAST_VIF	14   /* set/get IP mcast virt. iface */
#define IP_RSVP_ON		15   /* enable RSVP in kernel */
#define IP_RSVP_OFF		16   /* disable RSVP in kernel */
#define IP_RSVP_VIF_ON		17   /* set RSVP per-vif socket */
#define IP_RSVP_VIF_OFF		18   /* unset RSVP per-vif socket */

/*
 * Defaults and limits for options
 */
#define	IP_DEFAULT_MULTICAST_TTL  1	/* normally limit m'casts to 1 hop  */
#define	IP_DEFAULT_MULTICAST_LOOP 1	/* normally hear sends if a member  */
#define	IP_MAX_MEMBERSHIPS	20	/* per socket */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
	struct	in_addr imr_multiaddr;	/* IP multicast address of group */
	struct	in_addr imr_interface;	/* local IP address of interface */
};
struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr; /* IPv6 multicast address of group */
	unsigned long ipv6mr_interface;	/* local IPv6 address of interface */
};

/*
 * packet information
 */
struct in6_pktinfo {
	struct in6_addr ipi6_addr;	/* src/dst IPv6 address */
	long		ipi6_ifindex;	/* send/recv interface index */
};

/*
 * Definitions for inet sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define	IPPROTO_MAXID	(IPPROTO_IDP + 1)	/* don't list to IPPROTO_MAX */

#define	CTL_IPPROTO_NAMES { \
	{ "ip", CTLTYPE_NODE }, \
	{ "icmp", CTLTYPE_NODE }, \
	{ "igmp", CTLTYPE_NODE }, \
	{ "ggp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "tcp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ "egp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "pup", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "udp", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "idp", CTLTYPE_NODE }, \
}

/*
 * Names for IP sysctl objects
 */
#define	IPCTL_FORWARDING	1	/* act as router */
#define	IPCTL_SENDREDIRECTS	2	/* may send redirects when forwarding */
#define	IPCTL_DEFTTL		3	/* default TTL */
#ifdef notyet
#define	IPCTL_DEFMTU		4	/* default MTU */
#endif
#define IPCTL_RTEXPIRE		5	/* cloned route expiration time */
#define IPCTL_RTMINEXPIRE	6	/* min value for expiration time */
#define IPCTL_RTMAXCACHE	7	/* trigger level for dynamic expire */
#define	IPCTL_SOURCEROUTE	8	/* may perform source routes */
#define	IPCTL_MAXID		9

#define	IPCTL_NAMES { \
	{ 0, 0 }, \
	{ "forwarding", CTLTYPE_INT }, \
	{ "redirect", CTLTYPE_INT }, \
	{ "ttl", CTLTYPE_INT }, \
	{ "mtu", CTLTYPE_INT }, \
	{ "rtexpire", CTLTYPE_INT }, \
	{ "rtminexpire", CTLTYPE_INT }, \
	{ "rtmaxcache", CTLTYPE_INT }, \
	{ "sourceroute", CTLTYPE_INT }, \
}

#endif

