#ifndef _NETINET_IP6_ICMP_H_
#define _NETINET_IP6_ICMP_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _NETINET_IN_SYSTM_H_
#include <netinet/in_systm.h>
#endif

#ifndef _NETINET_IN_H_
#include <netinet/in.h>
#endif

#ifndef _NETINET_IP6_H_
#include <netinet/ip6.h>
#endif

/*
 * Interface Control Message Protocol Definitions.
 * Per not yet RFC xxxx
 */

/*
 * Structure of an ICMPv6 header.
 */
struct icmpv6 {
	u_int8_t	icmp6_type;	/* type of message, see below */
	u_int8_t	icmp6_code;	/* type sub code */
	u_int16_t	icmp6_cksum;	/* checksum */
	union {
	    n_long	 ih6_pmtu;	    /* next MTU (for discovery) */
	    n_long	 ih6_pptr;	    /* bad parameter pointer */
	    n_long	 ih6_flags;	    /* reserved/flags field */
	    struct ih6_idseq {		    /* echo */
		n_short	  icd6_id;
		n_short	  icd6_seq;
	    } ih6_idseq;
	    struct ih6_gmm {
		n_short	  igm6_mrd;	    /* maximum response delay */
		n_short	  igm6_spare;
	    } ih6_gmm;
	    struct ih6_rtad {
	        u_int8_t  irta_mhlim;
		u_int8_t  irta_aflg;
		u_int16_t irta_life;
	    } ih6_rtad;
	} icmp6_hun;
#define icmp6_pmtu	icmp6_hun.ih6_pmtu
#define icmp6_pptr	icmp6_hun.ih6_pptr
#define icmp6_flags	icmp6_hun.ih6_flags
#define icmp6_id	icmp6_hun.ih6_idseq.icd6_id
#define icmp6_seq	icmp6_hun.ih6_idseq.icd6_seq
#define icmp6_mrd	icmp6_hun.ih6_gmm.igm6_mrd
#define icmp6_mhlim	icmp6_hun.ih6_rtad.irta_mhlim
#define icmp6_aflg	icmp6_hun.ih6_rtad.irta_aflg
#define icmp6_life	icmp6_hun.ih6_rtad.irta_life
	union {
	    struct in6_addr id6_grp;	    /* group address */
	    struct id6_addr {
	        struct in6_addr  id6_tgt;     /* target address */
	        struct in6_addr  id6_rdst;    /* (redirect) destination */
	    } id6_addr;
	    struct id6_nud {
	        u_int32_t	 id6_reach;   /* reachable time */
	        u_int32_t	 id6_retrans; /* retrans timer */
	    } id6_nud;
	    struct ipv6	    id6_ip;	    /* header of packet in error */
	    int8_t	    id6_data[1];    /* echo packet data */
	} icmp6_dun;
#define icmp6_grp	icmp6_dun.id6_grp
#define icmp6_tgt	icmp6_dun.id6_addr.id6_tgt
#define icmp6_rdst	icmp6_dun.id6_addr.id6_rdst
#define icmp6_reach	icmp6_dun.id6_nud.id6_reach
#define icmp6_retrans	icmp6_dun.id6_nud.id6_retrans
#define icmp6_ip	icmp6_dun.id6_ip
#define icmp6_data	icmp6_dun.id6_data
};

#define	ICMP6_MINLEN	8		/* abs minimum */
#define ICMP6_GRPLEN	24		/* group stuff */
#define ICMP6_RSLEN	8		/* router solicitation */
#define ICMP6_RALEN	16		/* router advertisement */
#define ICMP6_NSLEN	24		/* neighbor solicitation */
#define ICMP6_NALEN	24		/* neighbor advertisement */
#define ICMP6_RDLEN	40		/* redirect */

/*
 * Definition of type and code field values.
 */
#define ICMP6_UNREACH		1	/* dest unreachable, codes: */
#define  ICMP6_UNREACH_NOROUTE	  0	 /* no route to destination */
#define  ICMP6_UNREACH_ADMIN	  1	 /* administratively prohibited */
#define  ICMP6_UNREACH_RTFAIL	  2	 /* not a neighbor */
#define  ICMP6_UNREACH_ADDRESS	  3	 /* address unreachable */
#define  ICMP6_UNREACH_PORT	  4	 /* port unreachable */
#define ICMP6_PKTTOOBIG		2	/* packet too big */
#define ICMP6_TIMXCEED		3	/* time exceeded, code: */
#define  ICMP6_TIMXCEED_INTRANS	  0	 /* ttl==0 in transit */
#define  ICMP6_TIMXCEED_REASS	  1	 /* ttl==0 in reass */
#define ICMP6_PARAMPROB		4	/* ip header bad */
#define  ICMP6_PARAMPROB_HDR	  0      /* erroneous header field */
#define  ICMP6_PARAMPROB_NH	  1      /* unrecognized next header */
#define  ICMP6_PARAMPROB_OPT	  2      /* unrecognized option */
#define ICMP6_OLDREDIRECT	5	/* old redirect */
#define ICMP6_ECHO		128	/* echo service */
#define ICMP6_ECHOREPLY		129	/* echo reply */
#define ICMP6_GROUPMEM_QUERY	130	/* group membership query */
#define ICMP6_GROUPMEM_REPORT	131	/* group membership report */
#define ICMP6_GROUPMEM_TERM	132	/* group membership termination */
#define ICMP6_SOLICITATION_RT	133	/* router solicitation */
#define ICMP6_ADVERTISEMENT_RT	134	/* router advertisment */
#define ICMP6_SOLICITATION_ND	135	/* neighbor solicitation */
#define ICMP6_ADVERTISEMENT_ND	136	/* neighbor advertisment */
#define ICMP6_REDIRECT		137	/* redirect */

#define	ICMP6_INFOTYPE(type)	((type) >= ICMP6_ECHO)

#define ICMP6_ND_HOPS		255	/* hop count for ND */
#define ICMP6_ND_PRIORITY	IPV6_PRIORITY_15

/* IPSec flags */

#define	ICMP6SEC_ERRORS		1	/* auth error types */
#define ICMP6SEC_REQUESTS	2	/* auth echo requests */
#define ICMP6SEC_GROUPS		4	/* auth group management */
#define ICMP6SEC_NEIGHBORS	8	/* auth neighbor management */

#endif
