#ifndef CLIB_USERGROUP_PROTOS_H
#define CLIB_USERGROUP_PROTOS_H 1

char *crypt(const char *,const char *);
void endgrent(void);
void endpwent(void);
void endutent(void);
struct UserGroupCredentials_u *getcredentials(struct Task *);
long getegid(void);
long geteuid(void);
long getgid(void);
struct group *getgrent(void);
struct group *getgrgid(long);
struct group *getgrnam(const char *);
long getgroups(long,long *);
struct lastlog *getlastlog(long);
char *getlogin(void);
char *getpass(const char *);
struct Task *getpgrp(void);
struct passwd *getpwent(void);
struct passwd *getpwnam(const char *);
struct passwd *getpwuid(long);
long getuid(void);
unsigned short getumask(void);
struct utmp *getutent(void);
long initgroups(const char *,long);
long setgid(long);
void setgrent(void);
long setgroups(long,const long *);
long setlastlog(long,char *,char *);
long setlogin(const char *);
void setpwent(void);
long setregid(long,long);
long setreuid(long,long);
struct Task *setsid(void);
long setuid(long);
void setutent(void);
long ug_GetErr(void);
long ug_SetupContextTagList(const unsigned char *, struct TagItem *);
long ug_SetupContextTags(const unsigned char *,...);
const char *ug_StrError(long);
char *ug_GetSalt(const struct passwd *,char *,unsigned long);
unsigned short umask(unsigned short);

#define USERGROUPNAME "usergroup.library"

#endif /* CLIB_USERGROUP_PROTOS_H */
