#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H 1

#ifndef NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

long accept(long,struct sockaddr *,long *);
long bind(long,const struct sockaddr *,long);
long CloseSocket(long);
long connect(long,const struct sockaddr *,long);
long Dup2Socket(long,long);
long Errno(void);
long getdtablesize(void);
struct hostent *gethostbyaddr(const unsigned char *,long,long);
struct hostent *gethostbyname(const unsigned char *);
long gethostname(char *,long);
unsigned long gethostid(void);
struct netent *getnetbyaddr(long,long);
struct netent *getnetbyname(const unsigned char *);
long getpeername(long,struct sockaddr *,long *);
struct protoent *getprotobyname(const unsigned char *);
struct protoent *getprotobynumber(long);
struct servent *getservbyname(const unsigned char *,const unsigned char *);
struct servent *getservbyport(long,const unsigned char *);
long GetSocketEvents(unsigned long *);
long getsockname(long,struct sockaddr *,long *);
long getsockopt(long,long,long,void *,long *);
unsigned long inet_addr(const unsigned char *);
unsigned long Inet_LnaOf(long);
unsigned long inet_lnaof(struct in_addr);
unsigned long Inet_MakeAddr(unsigned long,unsigned long);
struct in_addr inet_makeaddr(unsigned long,unsigned long);
unsigned long Inet_NetOf(long);
unsigned long inet_netof(struct in_addr);
unsigned long inet_network(const unsigned char *);
char *Inet_NtoA(unsigned long);
char *inet_ntoa(struct in_addr);
long IoctlSocket(long,unsigned long,char *);
long listen(long,long);
long ObtainSocket(long,long,long,long);
long recv(long,unsigned char *,long,long);
long recvfrom(long,unsigned char *,long,long,struct sockaddr *,long *);
long recvmsg(long,struct msghdr *,long);
long ReleaseCopyOfSocket(long,long);
long ReleaseSocket(long,long);
long select(long,fd_set *,fd_set *,fd_set *,void *);
long send(long,const unsigned char *,long,long);
long sendmsg(long,struct msghdr *,long);
long sendto(long,const unsigned char *,long,long,
 const struct sockaddr *,long);
long SetErrnoPtr(void *,long);
void SetSocketSignals(unsigned long,unsigned long,unsigned long);
long setsockopt(long,long,long,const void *,long);
long shutdown(long,long);
long socket(long,long,long);
long SocketBaseTagList(struct TagItem *);
long SocketBaseTags(long,...);
void syslog(unsigned long,const char *, ...);
void vsyslog(unsigned long,const char *,long *);
long WaitSelect(long,fd_set *,fd_set *,fd_set *,void *,
 unsigned long *);

#define SOCKETNAME "bsdsocket.library"

#endif /* !CLIB_SOCKET_PROTOS_H */
