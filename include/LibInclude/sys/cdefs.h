#ifndef _SYS_CDEFS_H_
#define _SYS_CDEFS_H_

#define	__BEGIN_DECLS
#define	__END_DECLS

#define	__P(protos)	protos		/* full-blown ANSI C */
#define	__CONCAT(x,y)	x ## y
#define	__STRING(x)	#x

#define	__const		const		/* define reserved names to standard */
#define	__signed	signed
#define	__volatile	volatile
#ifndef __SASC
#define	__inline
#endif

#define	__attribute__(x)	/* delete __attribute__ if non-gcc or gcc1 */

#ifndef __dead
#define	__dead
#define	__pure
#endif

#endif
