#ifndef  CLIB_MIAMIBPF_PROTOS_H
#define  CLIB_MIAMIBPF_PROTOS_H

/*
**	$VER: miamibpf_protos.h 1.1 (10.07.97)
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1997 Nordic Global Inc.
**	    All Rights Reserved
*/

void MiamiBPFInit(struct Library *,struct Library *);
unsigned long MiamiBPFOpen(struct TagItem *);
void MiamiBPFClose(unsigned long);
long MiamiBPFRead(unsigned long,unsigned char *,unsigned long);
unsigned long MiamiBPFSignalmask(unsigned long);
long MiamiBPFIoctl(unsigned long,unsigned long,unsigned char *);
void MiamiBPFSetAbortmask(unsigned long,unsigned long);
unsigned long MiamiBPFFilter(void *,unsigned char *,unsigned long,char);

#define MIAMIBPFNAME "Miami:Libs/miamibpf.library"

#endif
