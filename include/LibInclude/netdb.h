#ifndef _NETDB_H_
#define _NETDB_H_

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	long	h_addrtype;	/* host address type */
	long	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in an unsigned long -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	long		n_addrtype;	/* net address type */
	unsigned long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	long	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	long	p_proto;	/* protocol # */
};

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 */

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

struct addrinfo {
  long		ai_flags;			/* AI_PASSIVE, AI_CANONNAME */
  long		ai_family;			/* PF_xxx */
  long		ai_socktype;		/* SOCK_xxx */
  long		ai_protocol;		/* IPPROTO_xxx for IPv4 and IPv6 */
  size_t	ai_addrlen;			/* length of ai_addr */
  char		*ai_canonname;		/* canonical name for host */
  struct sockaddr	*ai_addr;	/* binary address */
  struct addrinfo	*ai_next;	/* next structure in linked list */
};

			/* following for getaddrinfo() */
#define	AI_PASSIVE		 1	/* socket is intended for bind() + listen() */
#define	AI_CANONNAME	 2	/* return canonical name */

			/* following for getnameinfo() */
#define	NI_MAXHOST	  1025	/* max hostname returned */
#define	NI_MAXSERV	    32	/* max service name returned */

#define	NI_NOFQDN	     1	/* do not return FQDN */
#define	NI_NUMERICHOST   2	/* return numeric form of hostname */
#define	NI_NAMEREQD	     4	/* return error if hostname not found */
#define	NI_NUMERICSERV   8	/* return numeric form of service name */
#define	NI_DGRAM	    16	/* datagram service for getservbyname() */

			/* error returns */
#define	EAI_ADDRFAMILY	 1	/* address family for host not supported */
#define	EAI_AGAIN		 2	/* temporary failure in name resolution */
#define	EAI_BADFLAGS	 3	/* invalid value for ai_flags */
#define	EAI_FAIL		 4	/* non-recoverable failure in name resolution */
#define	EAI_FAMILY		 5	/* ai_family not supported */
#define	EAI_MEMORY		 6	/* memory allocation failure */
#define	EAI_NODATA		 7	/* no address associated with host */
#define	EAI_NONAME		 8	/* host nor service provided, or not known */
#define	EAI_SERVICE		 9	/* service not supported for ai_socktype */
#define	EAI_SOCKTYPE	10	/* ai_socktype not supported */
#define	EAI_SYSTEM		11	/* system error returned in errno */

#endif /* !_NETDB_H_ */
