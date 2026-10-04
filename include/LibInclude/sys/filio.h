#ifndef	_SYS_FILIO_H_
#define	_SYS_FILIO_H_

#ifndef _SYS_IOCCOM_H_
#include <sys/ioccom.h>
#endif

/* Generic file-descriptor ioctl's. */
#define	FIOCLEX		 _IO('f', 1)		/* set close on exec on fd */
#define	FIONCLEX	 _IO('f', 2)		/* remove close on exec */
#define	FIONREAD	_IOR('f', 127, long)	/* get # bytes to read */
#define	FIONBIO		_IOW('f', 126, long)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW('f', 125, long)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW('f', 124, long)	/* set owner */
#define	FIOGETOWN	_IOR('f', 123, long)	/* get owner */

#endif /* !_SYS_FILIO_H_ */
