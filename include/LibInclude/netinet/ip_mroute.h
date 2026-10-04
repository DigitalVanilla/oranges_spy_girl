#ifndef _NETINET_IP_MROUTE_H_
#define _NETINET_IP_MROUTE_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _NETINET_IN_H_
#include <netinet/in.h>
#endif

/*
 * Multicast Routing set/getsockopt commands.
 */
#define	MRT_INIT	100	/* initialize forwarder */
#define	MRT_DONE	101	/* shut down forwarder */
#define	MRT_ADD_VIF	102	/* create virtual interface */
#define	MRT_DEL_VIF	103	/* delete virtual interface */
#define MRT_ADD_MFC	104	/* insert forwarding cache entry */
#define MRT_DEL_MFC	105	/* delete forwarding cache entry */
#define MRT_VERSION	106	/* get kernel version number */
#define MRT_ASSERT      107     /* enable PIM assert processing */


/*
 * Types and macros for handling bitmaps with one bit per virtual interface.
 */
#define	MAXVIFS 32
typedef u_long vifbitmap_t;
typedef u_short vifi_t;		/* type of a vif index */
#define ALL_VIFS (vifi_t)-1

#define	VIFM_SET(n, m)		((m) |= (1 << (n)))
#define	VIFM_CLR(n, m)		((m) &= ~(1 << (n)))
#define	VIFM_ISSET(n, m)	((m) & (1 << (n)))
#define	VIFM_CLRALL(m)		((m) = 0x00000000)
#define	VIFM_COPY(mfrom, mto)	((mto) = (mfrom))
#define	VIFM_SAME(m1, m2)	((m1) == (m2))


/*
 * Argument structure for MRT_ADD_VIF.
 * (MRT_DEL_VIF takes a single vifi_t argument.)
 */
struct vifctl {
	vifi_t	vifc_vifi;	    	/* the index of the vif to be added */
	u_char	vifc_flags;     	/* VIFF_ flags defined below */
	u_char	vifc_threshold; 	/* min ttl required to forward on vif */
	u_int	vifc_rate_limit;	/* max rate */
	struct	in_addr vifc_lcl_addr;	/* local interface address */
	struct	in_addr vifc_rmt_addr;	/* remote address (tunnels only) */
};

#define	VIFF_TUNNEL	0x1		/* vif represents a tunnel end-point */
#define VIFF_SRCRT	0x2		/* tunnel uses IP source routing */

/*
 * Argument structure for MRT_ADD_MFC and MRT_DEL_MFC
 * (mfcc_tos to be added at a future point)
 */
struct mfcctl {
    struct in_addr  mfcc_origin;		/* ip origin of mcasts       */
    struct in_addr  mfcc_mcastgrp; 		/* multicast group associated*/
    vifi_t	    mfcc_parent;   		/* incoming vif              */
    u_char	    mfcc_ttls[MAXVIFS]; 	/* forwarding ttls on vifs   */
};

/*
 * Argument structure used by mrouted to get src-grp pkt counts
 */
struct sioc_sg_req {
    struct in_addr src;
    struct in_addr grp;
    u_long pktcnt;
    u_long bytecnt;
    u_long wrong_if;
};

/*
 * Argument structure used by mrouted to get vif pkt counts
 */
struct sioc_vif_req {
    vifi_t vifi;		/* vif number				*/
    u_short pad1;		/* force natural alignment	*/
    u_long icount;		/* Input packet count on vif		*/
    u_long ocount;		/* Output packet count on vif		*/
    u_long ibytes;		/* Input byte count on vif		*/
    u_long obytes;		/* Output byte count on vif		*/
};

#endif /* _NETINET_IP_MROUTE_H_ */
