#ifndef _NET_PCAP_H
#define _NET_PCAP_H

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _SYS_TIME_H_
#include <sys/time.h>
#endif

#ifndef _NET_BPF_H_
#include <net/bpf.h>
#endif

#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4

#define PCAP_ERRBUF_SIZE 256

typedef	long bpf_int32;
typedef	u_long bpf_u_int32;

typedef struct pcap pcap_t;
typedef struct pcap_dumper pcap_dumper_t;

struct pcap_file_header {
	bpf_u_int32 magic;
	u_short version_major;
	u_short version_minor;
	bpf_int32 thiszone;	/* gmt to local correction */
	bpf_u_int32 sigfigs;	/* accuracy of timestamps */
	bpf_u_int32 snaplen;	/* max length saved portion of each pkt */
	bpf_u_int32 linktype;	/* data link type (DLT_*) */
};

struct pcap_pkthdr {
	struct timeval ts;	/* time stamp */
	bpf_u_int32 caplen;	/* length of portion present */
	bpf_u_int32 len;	/* length this packet (off wire) */
};

struct pcap_stat {
	u_int ps_recv;		/* number of packets received */
	u_int ps_drop;		/* number of packets dropped */
	u_int ps_ifdrop;	/* drops by interface XXX not yet supported */
};

typedef void (*pcap_handler)(u_char *, const struct pcap_pkthdr *,
			     const u_char *);

#endif
