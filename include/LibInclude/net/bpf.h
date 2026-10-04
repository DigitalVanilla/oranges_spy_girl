#ifndef _NET_BPF_H_
#define	_NET_BPF_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef _SYS_IOCCOM_H_
#include <sys/ioccom.h>
#endif

#ifndef _SYS_TIME_H_
#include <sys/time.h>
#endif


#define BPF_ALIGNMENT sizeof(long)
#define BPF_WORDALIGN(x) (((x)+(BPF_ALIGNMENT-1))&~(BPF_ALIGNMENT-1))

#define BPF_MAXINSNS	512

struct bpf_program {
	u_int bf_len;
	struct bpf_insn *bf_insns;
};

struct bpf_stat {
	u_int bs_recv;
	u_int bs_drop;
};

struct bpf_version {
	u_short bv_major;
	u_short bv_minor;
};

#define BPF_MAJOR_VERSION	1
#define BPF_MINOR_VERSION	1

#define BIOCGBLEN		_IOR('B',102,u_int)
#define BIOCSBLEN		_IOWR('B',102,u_int)
#define BIOCSETF		_IOW('B',103,struct bpf_program)
#define BIOCFLUSH		_IO('B',104)
#define BIOCGDLT		_IOR('B',106,u_int)
#define BIOCGETIF		_IOR('B',107,struct ifreq)
#define BIOCSETIF		_IOW('B',108,struct ifreq)
#define BIOCSRTIMEOUT	_IOW('B',109,struct timeval)
#define BIOCGRTIMEOUT	_IOR('B',110,struct timeval)
#define BIOCGSTATS		_IOR('B',111,struct bpf_stat)
#define BIOCIMMEDIATE	_IOW('B',112,u_int)
#define BIOCVERSION		_IOR('B',113,struct bpf_version)

struct bpf_hdr {
	struct timeval bh_tstamp;
	u_long bh_caplen;
	u_long bh_datalen;
	u_short bh_hdrlen;
	u_short bh_pad;
};

#define DLT_MIAMI		100

#define BPF_CLASS(code)	((code)&0x07)
#define BPF_LD		0x00
#define BPF_LDX		0x01
#define BPF_ST		0x02
#define BPF_STX		0x03
#define BPF_ALU		0x04
#define BPF_JMP		0x05
#define BPF_RET		0x06
#define BPF_MISC	0x07

#define BPF_SIZE(code)	((code)&0x18)
#define BPF_W		0x00
#define BPF_H		0x08
#define BPF_B		0x10
#define BPF_MODE(code)	((code)&0xe0)
#define BPF_IMM		0x00
#define BPF_ABS		0x20
#define BPF_IND		0x40
#define BPF_MEM		0x60
#define BPF_LEN		0x80
#define BPF_MSH		0xa0

#define BPF_OP(code)	((code)&0xf0)
#define BPF_ADD		0x00
#define BPF_SUB		0x10
#define BPF_MUL		0x20
#define BPF_DIV		0x30
#define BPF_OR		0x40
#define BPF_AND		0x50
#define BPF_LSH		0x60
#define BPF_RSH		0x70
#define BPF_NEG		0x80
#define BPF_JA		0x00
#define BPF_JEQ		0x10
#define BPF_JGT		0x20
#define BPF_JGE		0x30
#define BPF_JSET	0x40
#define BPF_SRC(code)	((code)&0x08)
#define BPF_K		0x00
#define BPF_X		0x08

#define BPF_RVAL(code)	((code)&0x18)
#define BPF_A		0x10

#define BPF_MISCOP(code)	((code)&0xf8)
#define BPF_TAX		0x00
#define BPF_TXA		0x80

struct bpf_insn {
	u_short code;
	u_char jt;
	u_char jf;
	long k;
};

#define BPF_MEMWORDS	16

#endif
