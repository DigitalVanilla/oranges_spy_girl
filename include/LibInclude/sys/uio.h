#ifndef _SYS_UIO_H_
#define	_SYS_UIO_H_

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

/*
 * XXX
 * iov_base should be a void *.
 */
struct iovec {
	char	*iov_base;	/* Base address. */
	size_t	 iov_len;	/* Length. */
};

enum	uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum uio_seg {
	UIO_USERSPACE,		/* from user data space */
	UIO_SYSSPACE,		/* from system space */
	UIO_USERISPACE		/* from user I space */
};

#endif
