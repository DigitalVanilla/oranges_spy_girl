#ifdef __PPC__
#include <ppcpragmas/miamipcap_pragmas.h>
#else
#pragma libcall MiamiPCapBase MiamiPCapInit 1e 9802
#pragma libcall MiamiPCapBase MiamiPCapStats 24 9802
#pragma libcall MiamiPCapBase MiamiPCapOpenLive 2a 9210805
#pragma libcall MiamiPCapBase MiamiPCapSetfilter 30 9802
#pragma libcall MiamiPCapBase MiamiPCapOpenOffline 36 9802
#pragma libcall MiamiPCapBase MiamiPCapAmigaDump 3c A9803
#pragma libcall MiamiPCapBase MiamiPCapAmigaDumpOpen 42 9802
#pragma libcall MiamiPCapBase MiamiPCapAmigaDumpClose 48 801
#pragma libcall MiamiPCapBase MiamiPCapLookupdev 4e 801
#pragma libcall MiamiPCapBase MiamiPCapLookupnet 54 BA9804
#pragma libcall MiamiPCapBase MiamiPCapDispatch 5a A90804
#pragma libcall MiamiPCapBase MiamiPCapLoop 60 A90804
#pragma libcall MiamiPCapBase MiamiPCapNext 66 9802
#pragma libcall MiamiPCapBase MiamiPCapDatalink 6c 801
#pragma libcall MiamiPCapBase MiamiPCapSnapshot 72 801
#pragma libcall MiamiPCapBase MiamiPCapIsSwapped 78 801
#pragma libcall MiamiPCapBase MiamiPCapMajorVersion 7e 801
#pragma libcall MiamiPCapBase MiamiPCapMinorVersion 84 801
#pragma libcall MiamiPCapBase MiamiPCapAmigaFile 8a 801
#pragma libcall MiamiPCapBase MiamiPCapPerror 90 9802
#pragma libcall MiamiPCapBase MiamiPCapGeterr 96 801
#pragma libcall MiamiPCapBase MiamiPCapStrerror 9c 001
#pragma libcall MiamiPCapBase MiamiPCapClose a2 801
#pragma libcall MiamiPCapBase MiamiPCapCompile a8 10A9805
#pragma libcall MiamiPCapBase MiamiPCapAmigaBPFHandle ae 801
#pragma libcall MiamiPCapBase MiamiPCapBPFImage b4 0802
#pragma libcall MiamiPCapBase MiamiPCapSetCallbackType cc 0802
#endif
