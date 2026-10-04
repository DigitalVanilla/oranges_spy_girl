#ifdef __PPC__
#include <ppcpragmas/miamibpf_pragmas.h>
#else
#pragma libcall MiamiBPFBase MiamiBPFInit 1e 9802
#pragma libcall MiamiBPFBase MiamiBPFOpen 24 801
#pragma libcall MiamiBPFBase MiamiBPFClose 2a 001
#pragma libcall MiamiBPFBase MiamiBPFRead 30 18003
#pragma libcall MiamiBPFBase MiamiBPFSignalmask 36 001
#pragma libcall MiamiBPFBase MiamiBPFIoctl 3c 81003
#pragma libcall MiamiBPFBase MiamiBPFSetAbortmask 42 1002
#pragma libcall MiamiBPFBase MiamiBPFFilter 48 109804
#endif
