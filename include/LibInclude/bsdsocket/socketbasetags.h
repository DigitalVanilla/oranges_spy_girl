#ifndef _BSDSOCKET_SOCKETBASETAGS_H_
#define _BSDSOCKET_SOCKETBASETAGS_H_
/*
 *
 * please see the file COPYRIGHT for information on copyright
 * and distribution
 *      
 */

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/*
 * Argument passed by reference or by value (covers bit 15)
 */
#define SBTF_VAL 0x0000
#define SBTF_REF 0x8000

/*
 * Code value (covers bits 1-14)
 */
#define SBTB_CODE 1
#define SBTS_CODE 0x3FFF
#define SBTM_CODE(tag) ((((UWORD)(tag))>>SBTB_CODE) & SBTS_CODE)

/* 
 * Get/Set (read/write) selection (covers bit 0)
 */
#define SBTF_GET  0x0
#define SBTF_SET  0x1

/*
 * These are the macros that should actually be used in the
 * SocketBaseTags() call. Pass one of the SBTC_ values as the
 * "code" parameter here, and pass the argument in ti_Data.
 */
#define SBTM_GETREF(code) \
 (TAG_USER | SBTF_REF | (((code) & SBTS_CODE) << SBTB_CODE))

#define SBTM_GETVAL(code) (TAG_USER | (((code) & SBTS_CODE) << SBTB_CODE))

#define SBTM_SETREF(code) \
 (TAG_USER | SBTF_REF | (((code) & SBTS_CODE) << SBTB_CODE) | SBTF_SET)

#define SBTM_SETVAL(code) \
 (TAG_USER | (((code) & SBTS_CODE) << SBTB_CODE) | SBTF_SET)

/*
 * Available codes for the above macros.
 */

/* masks defining the signals for task notification */
#define SBTC_BREAKMASK      1
#define SBTC_SIGIOMASK      2
#define SBTC_SIGURGMASK     3
#define SBTC_SIGEVENTMASK   4

/* current value of the errno and h_errno variables */
#define SBTC_ERRNO          6
#define SBTC_HERRNO         7

/* current size of the socket descriptor table */
#define SBTC_DTABLESIZE     8

/* callback function to control and intercept the allocation of
 * socket descriptor values
 * 
 * The argument is a pointer to a callback function which is called
 * by the kernel with the following prototype:
 *
 * long fd = fdCallback(long fd, long action);
 *      D0                   D0       D1
 *
 */
#define SBTC_FDCALLBACK     9
/*
 * "action" values:
 */
#define FDCB_FREE  0
#define FDCB_ALLOC 1
#define FDCB_CHECK 2

/* control the behavior of syslog() and vsyslog() */
#define SBTC_LOGSTAT        10
#define SBTC_LOGTAGPTR      11
#define SBTC_LOGFACILITY    12
#define SBTC_LOGMASK        13

/*
 * The argument for the following tags is an ULONG with the error
 * number (as an input argument, passed to the function). On return
 * the error number has been replaced with a read-only pointer
 * to the corresponding error message.
 *
 * NOTE: error numbers defined in <exec/errors.h>
 * are negative and must be negated (so they become positive)
 * before passing them to SocketBaseTagList().
 */
#define SBTC_ERRNOSTRPTR    14 /* <sys/errno.h> */
#define SBTC_HERRNOSTRPTR   15 /* <netdb.h> */
#define SBTC_IOERRNOSTRPTR  16 /* <exec/errors.h> */
#define SBTC_S2ERRNOSTRPTR  17 /* <devices/sana2.h> */
#define SBTC_S2WERRNOSTRPTR 18 /* <devices/sana2.h> */


/* link the errno variable in the protocol stack to the errno variable
   in your program */
#define SBTC_ERRNOBYTEPTR   21
#define SBTC_ERRNOWORDPTR   22
#define SBTC_ERRNOLONGPTR   24
/*
 * Macro for generating the errnoptr tag code from a given size.
 * Only 1, 2 & 4 are allowed as 'size' values, corresponding to
 * 'char', 'short', and 'long' with typical compilers.
 * If the 'size' value is illegal, this macro sets the tag to 0,
 * which causes SocketBaseTagList() to fail.
 */
#define SBTC_ERRNOPTR(size)    ((size == sizeof(long)) ? SBTC_ERRNOLONGPTR :\
 ((size == sizeof(short)) ? SBTC_ERRNOWORDPTR :\
 ((size == sizeof(char)) ? SBTC_ERRNOBYTEPTR :\
 0)))

/* link the h_errno variable in the stack to the h_errno variable in
   your program */
#define SBTC_HERRNOLONGPTR  25

/* pointer to a text string with release information on the
   protocol stack being used (read only!) */
#define SBTC_RELEASESTRPTR  29

#endif /* !BSDSOCKET_SOCKETBASETAGS_H */
