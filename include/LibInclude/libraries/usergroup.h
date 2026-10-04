#ifndef LIBRARIES_USERGROUP_H
#define LIBRARIES_USERGROUP_H


#define _PASSWORD_EFMT1 '_' /* extended encryption format */

#define _PASSWORD_LEN   128 /* max length, not counting NULL */

#define NGROUPS 32

#define MAXLOGNAME      32

/* old, unaligned version */

struct UserGroupCredentials_u {
  long    cr_ruid;
  long    cr_rgid;
  unsigned short cr_umask;
  long    cr_euid;
  short   cr_ngroups;
  long    cr_groups[NGROUPS];
  struct Task *cr_session;
  char    cr_login[MAXLOGNAME];
};


struct UserGroupCredentials {
  long    cr_ruid;
  long    cr_rgid;
  unsigned short cr_umask;
  unsigned short cr_pad1;
  long    cr_euid;
  short   cr_ngroups;
  short   cr_pad2;
  long    cr_groups[NGROUPS];
  struct Task *cr_session;
  char    cr_login[MAXLOGNAME];
};

/*
 * ID conversion macros between usergroup.library and MultiUser
 */
#define UG2MU(id) ((id) == 0 ? 65535 : (id) == -2 ? 0 : (id))
#define MU2UG(id) ((id) == 65535 ? 0L : (id) == 0L ? -2L : (id))

/*
 * Tags for ug_SetupContextTags()
 */
#define UGT_ERRNOBPTR 0x80000001
#define UGT_ERRNOWPTR 0x80000002
#define UGT_ERRNOLPTR 0x80000004
#define UGT_ERRNOPTR(size)\
  ((size == 4) ? UGT_ERRNOLPTR :\
   (size == 2) ? UGT_ERRNOWPTR :\
   (size == 1) ? UGT_ERRNOBPTR : 1L)
#define UGT_OWNER     0x80000011
#define UGT_INTRMASK  0x80000010

#endif /* LIBRARIES_USERGROUP_H */
