#ifdef __PPC__
#include <ppcpragmas/usergroup_pragmas.h>
#else
#pragma libcall UserGroupBase ug_SetupContextTagList 1e 9802
#pragma tagcall UserGroupBase ug_SetupContextTags 1e 9802
#pragma libcall UserGroupBase ug_GetErr 24 0
#pragma libcall UserGroupBase ug_StrError 2a 101
#pragma libcall UserGroupBase getuid 30 0
#pragma libcall UserGroupBase geteuid 36 0
#pragma libcall UserGroupBase setreuid 3c 1002
#pragma libcall UserGroupBase setuid 42 001
#pragma libcall UserGroupBase getgid 48 0
#pragma libcall UserGroupBase getegid 4e 0
#pragma libcall UserGroupBase setregid 54 1002
#pragma libcall UserGroupBase setgid 5a 001
#pragma libcall UserGroupBase getgroups 60 9002
#pragma libcall UserGroupBase setgroups 66 9002
#pragma libcall UserGroupBase initgroups 6c 0902
#pragma libcall UserGroupBase getpwnam 72 901
#pragma libcall UserGroupBase getpwuid 78 001
#pragma libcall UserGroupBase setpwent 7e 0
#pragma libcall UserGroupBase getpwent 84 0
#pragma libcall UserGroupBase endpwent 8a 0
#pragma libcall UserGroupBase getgrnam 90 901
#pragma libcall UserGroupBase getgrgid 96 001
#pragma libcall UserGroupBase setgrent 9c 0
#pragma libcall UserGroupBase getgrent a2 0
#pragma libcall UserGroupBase endgrent a8 0
#pragma libcall UserGroupBase crypt ae 9802
#pragma libcall UserGroupBase ug_GetSalt b4 09803
#pragma libcall UserGroupBase getpass ba 901
#pragma libcall UserGroupBase umask c0 001
#pragma libcall UserGroupBase getumask c6 0
#pragma libcall UserGroupBase setsid cc 0
#pragma libcall UserGroupBase getpgrp d2 0
#pragma libcall UserGroupBase getlogin d8 0
#pragma libcall UserGroupBase setlogin de 901
#pragma libcall UserGroupBase setutent e4 0
#pragma libcall UserGroupBase getutent ea 0
#pragma libcall UserGroupBase endutent f0 0
#pragma libcall UserGroupBase getlastlog f6 001
#pragma libcall UserGroupBase setlastlog fc 98003
#pragma libcall UserGroupBase getcredentials 102 801
#endif
