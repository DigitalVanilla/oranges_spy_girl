#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */

struct compatible_timeval {
  union {
    long s_sec;
    ULONG u_secs;
  } mtv_sec;
  union {
    long s_usec;
    ULONG u_micro;
  } mtv_usec;
};

#define timeval compatible_timeval
#define tv_sec mtv_sec.s_sec
#define tv_usec mtv_usec.s_usec
#define tv_secs mtv_sec.u_secs
#define tv_micro mtv_usec.u_micro

struct compatible_timerequest {
    struct IORequest tr_node;
    struct timeval tr_time;
};
#define timerequest compatible_timerequest

/*
 * Structure defined by POSIX.4 to be like a timeval.
 */
struct timespec {
	long	ts_sec;		/* seconds */
	long	ts_nsec;	/* and nanoseconds */
};

#define	TIMEVAL_TO_TIMESPEC(tv, ts) {					\
	(ts)->ts_sec = (tv)->tv_sec;					\
	(ts)->ts_nsec = (tv)->tv_usec * 1000;				\
}
#define	TIMESPEC_TO_TIMEVAL(tv, ts) {					\
	(tv)->tv_sec = (ts)->ts_sec;					\
	(tv)->tv_usec = (ts)->ts_nsec / 1000;				\
}

#include "time.h"

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */

/* Operations on timevals. */
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)						\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_usec cmp (uvp)->tv_usec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))

#endif /* !_SYS_TIME_H_ */
