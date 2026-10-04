#ifndef _NET_IF_H_
#define	_NET_IF_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _SYS_SOCKET_H_
#include <sys/socket.h>
#endif

#ifndef _SYS_TIME_H_
#include <sys/time.h>
#endif


#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define	IFF_OACTIVE	0x400		/* transmission in progress */
#define	IFF_SIMPLEX	0x800		/* can't hear own transmissions */
#define	IFF_LINK0	0x1000		/* per link layer defined bit */
#define	IFF_LINK1	0x2000		/* per link layer defined bit */
#define	IFF_LINK2	0x4000		/* per link layer defined bit */
#define	IFF_MULTICAST	0x8000		/* supports multicast */

/* flags set internally only: */
#define	IFF_CANTCHANGE \
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_RUNNING|IFF_OACTIVE|\
	    IFF_SIMPLEX|IFF_MULTICAST|IFF_ALLMULTI)


struct	if_data {
/* generic interface information */
	u_char	ifi_type;	/* ethernet, tokenring, etc */
	u_char	ifi_addrlen;	/* media address length */
	u_char	ifi_hdrlen;	/* media header length */
    u_char  ifi_pad1;   /* force natural alignment */
	u_long	ifi_mtu;	/* maximum transmission unit */
	u_long	ifi_metric;	/* routing metric (external only) */
	u_long	ifi_baudrate;	/* linespeed */
/* volatile statistics */
	u_long	ifi_ipackets;	/* packets received on interface */
	u_long	ifi_ierrors;	/* input errors on interface */
	u_long	ifi_opackets;	/* packets sent on interface */
	u_long	ifi_oerrors;	/* output errors on interface */
	u_long	ifi_collisions;	/* collisions on csma interfaces */
	u_long	ifi_ibytes;	/* total number of octets received */
	u_long	ifi_obytes;	/* total number of octets sent */
	u_long	ifi_imcasts;	/* packets received via multicast */
	u_long	ifi_omcasts;	/* packets sent via multicast */
	u_long	ifi_iqdrops;	/* dropped on input, this interface */
	u_long	ifi_noproto;	/* destined for unsupported protocol */
	struct	timeval ifi_lastchange;/* last updated */
};

/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct	sockaddr *ifa_addr;	/* address of interface */
	struct	sockaddr *ifa_dstaddr;	/* other end of p-to-p link */
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
	struct	sockaddr *ifa_netmask;	/* used to determine subnet */
	struct	ifnet *ifa_ifp;		/* back-pointer to interface */
	struct	ifaddr *ifa_next;	/* next address for interface */
	void (*ifa_rtrequest)(long,struct rtentry *,struct sockaddr *);
	u_short	ifa_flags;		/* mostly rt_flags for cloning */
	short	ifa_refcnt;		/* extra to malloc for link info */
	long ifa_metric;		/* cost of going out this interface */
};
#define	IFA_ROUTE	RTF_UP		/* route installed */

/*
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket
 */
struct if_msghdr {
	u_short	ifm_msglen;	/* to skip over non-understood messages */
	u_char	ifm_version;	/* future binary compatability */
	u_char	ifm_type;	/* message type */
	long	ifm_addrs;	/* like rtm_addrs */
	long	ifm_flags;	/* value of if_flags */
	u_short	ifm_index;	/* index for associated ifp */
	u_short	ifm_pad1;	/* force natural alignment */
	struct	if_data ifm_data;/* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from getkerninfo and the routing socket
 */
struct ifa_msghdr {
	u_short	ifam_msglen;	/* to skip over non-understood messages */
	u_char	ifam_version;	/* future binary compatability */
	u_char	ifam_type;	/* message type */
	long	ifam_addrs;	/* like rtm_addrs */
	long	ifam_flags;	/* value of ifa_flags */
	u_short	ifam_index;	/* index for associated ifp */
	u_short	ifam_pad1;	/* force natural alignment */
	long    ifam_metric;	/* value of ifa_metric */
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
#define	IFNAMSIZ	16
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		short	ifru_flags;
		long	ifru_metric;
		long	ifru_mtu;
		long	ifru_phys;
		caddr_t	ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu */
#define ifr_phys	ifr_ifru.ifru_phys	/* physical wire */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

struct ifaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr ifra_addr;
	struct	sockaddr ifra_broadaddr;
	struct	sockaddr ifra_mask;
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	long	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

struct if_nameindex {
	unsigned long	if_index;
	char 			*if_name;
};

#endif /* !_NET_IF_H_ */
