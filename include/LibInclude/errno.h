#ifndef _ERRNO_H_
#define _ERRNO_H_

#ifndef _SYS_ERRNO_H_
#include <sys/errno.h>
#endif

extern int errno;
extern int __sys_nerr;
extern char *__sys_errlist[];

extern int _OSERR;

extern int __os_nerr;

#if 0
/* deprecated because of bad alignment */
extern struct {
    short errnum;
    char  *msg;
} __os_errlist[];
#endif

#endif
