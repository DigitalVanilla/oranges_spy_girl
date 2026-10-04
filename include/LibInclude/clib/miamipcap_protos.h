#ifndef  CLIB_MIAMIPCAP_PROTOS_H
#define  CLIB_MIAMIPCAP_PROTOS_H

/*
**	$VER: miamipcap_protos.h 1.0 (16.01.97)
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1997 Nordic Global Inc.
**	    All Rights Reserved
*/

#include <net/pcap.h>
#include <dos/dos.h>

char *MiamiPCapStrerror(long);
long MiamiPCapDispatch(pcap_t *,long,pcap_handler,u_char *);
char *MiamiPCapGeterr(pcap_t *);
long MiamiPCapSnapshot(pcap_t *);
long MiamiPCapDatalink(pcap_t *);
BPTR MiamiPCapAmigaFile(pcap_t *);
long MiamiPCapStats(pcap_t *,struct pcap_stat *);
void MiamiPCapClose(pcap_t *);
void MiamiPCapAmigaDump(u_char *,struct pcap_pkthdr *,u_char *);
void MiamiPCapAmigaDumpClose(pcap_dumper_t *);
void MiamiPCapInit(struct Library *,struct Library *);
pcap_t *MiamiPCapOpenOffline(char *,char *);
char *MiamiPCapLookupdev(char *);
pcap_t *MiamiPCapOpenLive(char *,long,long,long,char *);
long MiamiPCapLookupnet(char *,bpf_u_int32 *,bpf_u_int32 *,char *);
long MiamiPCapCompile(pcap_t *,struct bpf_program *,char *,long,unsigned long);
long MiamiPCapSetfilter(pcap_t *,struct bpf_program *);
pcap_dumper_t *MiamiPCapAmigaDumpOpen(pcap_t *,char *);
long MiamiPCapLoop(pcap_t *,long,pcap_handler,u_char *);
char *MiamiPCapBPFImage(struct bpf_insn *,long);
long MiamiPCapReady(pcap_t *);
unsigned long MiamiPCapSignalmask(pcap_t *);
void MiamiPCapSetAbortMask(pcap_t *,unsigned long);
const u_char *MiamiPCapNext(pcap_t *,struct pcap_pkthdr *);
long MiamiPCapIsSwapped(pcap_t *);
long MiamiPCapMajorVersion(pcap_t *);
long MiamiPCapMinorVersion(pcap_t *);
void MiamiPCapPerror(pcap_t *,char *);
long MiamiPCapAmigaBPFHandle(pcap_t *);
long MiamiPCapSetCallbackType(pcap_t *,long);

#define MIAMIPCAPNAME "Miami:Libs/miamipcap.library"

#endif
