#ifndef _SYS_TYPES_H_
#define	_SYS_TYPES_H_

#ifndef _MACHINE_ENDIAN_H_
#include <machine/endian.h>
#endif

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned long	u_int;
typedef	unsigned long	u_long;

typedef	char *		caddr_t;	/* core address */

#ifndef _MACHINE_ANSI_H_
#include <machine/ansi.h>
#endif

#ifndef _MACHINE_TYPES_H_
#include <machine/types.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifdef	_BSD_SIZE_T_
#ifndef _SIZE_T
typedef	_BSD_SIZE_T_	size_t;
#define _SIZE_T 1
#endif
#undef	_BSD_SIZE_T_
#endif

#define	NBBY	8		/* number of bits in a byte */

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define	FD_COPY(f, t)	bcopy(f, t, sizeof(*(f)))
#define	FD_ZERO(p)	bzero(p, sizeof(*(p)))

#endif /* !_SYS_TYPES_H_ */
