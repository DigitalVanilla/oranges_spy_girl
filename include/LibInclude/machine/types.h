#ifndef	_MACHINE_TYPES_H_
#define	_MACHINE_TYPES_H_

/*
 * Basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
typedef	signed char		   int8_t;
typedef	unsigned char		 u_int8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	long			  int32_t;
typedef	unsigned long		u_int32_t;

typedef	int32_t			register_t;

#endif	/* _MACHINE_TYPES_H_ */
