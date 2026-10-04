#ifndef	_SYS_SOCKIO_H_
#define	_SYS_SOCKIO_H_

#ifndef _SYS_IOCCOM_H_
#include <sys/ioccom.h>
#endif

/* Socket ioctl's. */
#define	SIOCSHIWAT	 _IOW('s',  0, long)		/* set high watermark */
#define	SIOCGHIWAT	 _IOR('s',  1, long)		/* get high watermark */
#define	SIOCSLOWAT	 _IOW('s',  2, long)		/* set low watermark */
#define	SIOCGLOWAT	 _IOR('s',  3, long)		/* get low watermark */
#define	SIOCATMARK	 _IOR('s',  7, long)		/* at oob mark? */
#define	SIOCSPGRP	 _IOW('s',  8, long)		/* set process group */
#define	SIOCGPGRP	 _IOR('s',  9, long)		/* get process group */
#define SIOCSRCVBULK _IOW('M',  1, long)
#define SIOCGRCVBULK _IOR('M',  2, long)
#define SIOCSSNDBULK _IOW('M',  3, long)
#define SIOCGSNDBULK _IOR('M',  4, long)

#define	SIOCADDRT	 _IOW('r', 10, struct ortentry)	/* add route */
#define	SIOCDELRT	 _IOW('r', 11, struct ortentry)	/* delete route */
#define SIOCGETVIFCNT	_IOWR('r', 15, struct sioc_vif_req)/* get vif pkt cnt */
#define SIOCGETSGCNT	_IOWR('r', 16, struct sioc_sg_req) /* get s,g pkt cnt */

#define	SIOCSIFADDR	 _IOW('i', 12, struct ifreq)	/* set ifnet address */
#define	OSIOCGIFADDR	_IOWR('i', 13, struct ifreq)	/* get ifnet address */
#define	SIOCGIFADDR	_IOWR('i', 33, struct ifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	 _IOW('i', 14, struct ifreq)	/* set p-p address */
#define	OSIOCGIFDSTADDR	_IOWR('i', 15, struct ifreq)	/* get p-p address */
#define	SIOCGIFDSTADDR	_IOWR('i', 34, struct ifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	 _IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	_IOWR('i', 17, struct ifreq)	/* get ifnet flags */
#define	OSIOCGIFBRDADDR	_IOWR('i', 18, struct ifreq)	/* get broadcast addr */
#define	SIOCGIFBRDADDR	_IOWR('i', 35, struct ifreq)	/* get broadcast addr */
#define	SIOCSIFBRDADDR	 _IOW('i', 19, struct ifreq)	/* set broadcast addr */
#define	OSIOCGIFCONF	_IOWR('i', 20, struct ifconf)	/* get ifnet list */
#define	SIOCGIFCONF	_IOWR('i', 36, struct ifconf)	/* get ifnet list */
#define	OSIOCGIFNETMASK	_IOWR('i', 21, struct ifreq)	/* get net addr mask */
#define	SIOCGIFNETMASK	_IOWR('i', 37, struct ifreq)	/* get net addr mask */
#define	SIOCSIFNETMASK	 _IOW('i', 22, struct ifreq)	/* set net addr mask */
#define	SIOCGIFMETRIC	_IOWR('i', 23, struct ifreq)	/* get IF metric */
#define	SIOCSIFMETRIC	 _IOW('i', 24, struct ifreq)	/* set IF metric */
#define	SIOCDIFADDR	 _IOW('i', 25, struct ifreq)	/* delete IF addr */
#define	SIOCAIFADDR	 _IOW('i', 26, struct ifaliasreq)/* add/chg IF alias */

#define	SIOCADDMULTI	 _IOW('i', 49, struct ifreq)	/* add m'cast addr */
#define	SIOCDELMULTI	 _IOW('i', 50, struct ifreq)	/* del m'cast addr */
#define SIOCGIFMTU	_IOWR('i', 51, struct ifreq)	/* get IF mtu */
#define SIOCSIFMTU	 _IOW('i', 52, struct ifreq)	/* set IF mtu */
#define SIOCGIFPHYS	_IOWR('i', 53, struct ifreq)	/* get IF wire */
#define SIOCSIFPHYS	 _IOW('i', 54, struct ifreq)	/* set IF wire */

#endif /* !_SYS_SOCKIO_H_ */
