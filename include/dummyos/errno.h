#ifndef _DUMMYOS_ERRNO_H_
#define _DUMMYOS_ERRNO_H_

#define	EPERM			1	/* Operation not permitted */
#define	ENOENT			2	/* No such file or directory */
#define	ESRCH			3	/* No such process */
#define	EINTR			4	/* Interrupted system call */
#define	EIO			5	/* Input/output error */
#define	ENXIO			6	/* Device not configured */
#define	E2BIG			7	/* Argument list too long */
#define	ENOEXEC			8	/* Exec format error */
#define	EBADF			9	/* Bad file descriptor */
#define	ECHILD			10	/* No child processes */
#define	EAGAIN			11	/* Resource temporarily unavailable */
#define	ENOMEM			12	/* Cannot allocate memory */
#define	EACCES			13	/* Permission denied */
#define	EFAULT			14	/* Bad address */
#define	ENOTBLK			15	/* Block device required */
#define	EBUSY			16	/* Device busy */
#define	EEXIST			17	/* File exists */
#define	EXDEV			18	/* Cross-device link */
#define	ENODEV			19	/* Operation not supported by device */
#define	ENOTDIR			20	/* Not a directory */
#define	EISDIR			21	/* Is a directory */
#define	EINVAL			22	/* Invalid argument */
#define	ENFILE			23	/* Too many open files in system */
#define	EMFILE			24	/* Too many open files */
#define	ENOTTY			25	/* Inappropriate ioctl for device */
#define	ETXTBSY			26	/* Text file busy */
#define	EFBIG			27	/* File too large */
#define	ENOSPC			28	/* No space left on device */
#define	ESPIPE			29	/* Illegal seek */
#define	EROFS			30	/* Read-only filesystem */
#define	EMLINK			31	/* Too many links */
#define	EPIPE			32	/* Broken pipe */
#define	EDOM			33	/* Numerical argument out of domain */
#define	ERANGE			34	/* Result too large */
#define	ENOMSG			35	/* No message of desired type */
#define	EIDRM			36	/* Identifier removed */
#define	ECHRNG			37	/* Channel number out of range */
#define	EL2NSYNC		38	/* Level 2 not synchronized */
#define	EL3HLT			39	/* Level 3 halted */
#define	EL3RST			40	/* Level 3 reset */
#define	ELNRNG			41	/* Link number out of range */
#define	EUNATCH			42	/* Protocol driver not attached */
#define	ENOCSI			43	/* No CSI structure available */
#define	EL2HLT			44	/* Level 2 halted */
#define	EDEADLK			45	/* Deadlock */
#define	ENOLCK			46	/* No lock */
#define EBADE			50	/* Invalid exchange */
#define EBADR			51	/* Invalid request descriptor */
#define EXFULL			52	/* Exchange full */
#define ENOANO			53	/* No anode */
#define EBADRQC			54	/* Invalid request code */
#define EBADSLT			55	/* Invalid slot */
#define EDEADLOCK		56	/* File locking deadlock error */
#define EBFONT			57	/* Bad font file fmt */
#define ENOSTR			60	/* Not a stream */
#define ENODATA			61	/* No data (for no delay io) */
#define ETIME			62	/* Stream ioctl timeout */
#define ENOSR			63	/* No stream resources */
#define ENONET			64	/* Machine is not on the network */
#define ENOPKG			65	/* Package not installed */
#define EREMOTE			66	/* The object is remote */
#define ENOLINK			67	/* Virtual circuit is gone */
#define EADV			68	/* Advertise error */
#define ESRMNT			69	/* Srmount error */
#define	ECOMM			70	/* Communication error on send */
#define EPROTO			71	/* Protocol error */
#define	EMULTIHOP		74	/* Multihop attempted */
#define	ELBIN			75	/* Inode is remote (not really error) */
#define	EDOTDOT			76	/* Cross mount point (not really error) */
#define EBADMSG			77	/* Bad message */
#define EFTYPE			79	/* Inappropriate file type or format */
#define ENOTUNIQ		80	/* Given log. name not unique */
#define EBADFD			81	/* f.d. invalid for this operation */
#define EREMCHG			82	/* Remote address changed */
#define ELIBACC			83	/* Can't access a needed shared lib */
#define ELIBBAD			84	/* Accessing a corrupted shared lib */
#define ELIBSCN			85	/* .lib section in a.out corrupted */
#define ELIBMAX			86	/* Attempting to link in too many libs */
#define ELIBEXEC		87	/* Attempting to exec a shared library */
#define ENOSYS			88	/* Function not implemented */

#define ENOTEMPTY		90	/* Directory not empty */
#define ENAMETOOLONG		91	/* File or path name too long */
#define ELOOP			92	/* Too many symbolic links */
#define EOPNOTSUPP		95	/* Operation not supported on socket */
#define EPFNOSUPPORT		96	/* Protocol family not supported */
#define ECONNRESET		104	/* Connection reset by peer */
#define ENOBUFS			105	/* No buffer space available */
#define EAFNOSUPPORT		106	/* Address family not supported by protocol family */
#define EPROTOTYPE		107	/* Protocol wrong type for socket */
#define ENOTSOCK		108	/* Socket operation on non-socket */
#define ENOPROTOOPT		109	/* Protocol not available */
#define ESHUTDOWN		110	/* Can't send after socket shutdown */
#define ECONNREFUSED		111	/* Connection refused */
#define EADDRINUSE		112	/* Address already in use */
#define ECONNABORTED		113	/* Software caused connection abort */
#define ENETUNREACH		114	/* Network is unreachable */
#define ENETDOWN		115	/* Network interface is not configured */
#define ETIMEDOUT		116	/* Connection timed out */
#define EHOSTDOWN		117	/* Host is down */
#define EHOSTUNREACH		118	/* Host is unreachable */
#define EINPROGRESS		119	/* Connection already in progress */
#define EALREADY		120	/* Socket already connected */
#define EDESTADDRREQ		121	/* Destination address required */
#define EMSGSIZE		122	/* Message too long */
#define EPROTONOSUPPORT		123	/* Unknown protocol */
#define ESOCKTNOSUPPORT		124	/* Socket type not supported */
#define EADDRNOTAVAIL		125	/* Address not available */
#define ENETRESET		126	/* Connection aborted by network */
#define EISCONN			127	/* Socket is already connected */
#define ENOTCONN		128	/* Socket is not connected */
#define ETOOMANYREFS		129
#define EPROCLIM		130
#define EUSERS			131
#define EDQUOT			132
#define ESTALE			133
#define ENOTSUP			134	/* Not supported */
#define ENOMEDIUM		135	/* No medium (in tape drive) */

#define EILSEQ			138	/* Illegal byte sequence */
#define EOVERFLOW		139	/* Value too large for defined data type */
#define ECANCELED		140	/* Operation canceled */
#define ENOTRECOVERABLE		141	/* State not recoverable */
#define EOWNERDEAD		142	/* Previous owner died */
#define ESTRPIPE		143	/* Streams pipe error */
#define EWOULDBLOCK		EAGAIN	/* Operation would block */

#define __ELASTERROR		2000	/* Users can add values starting here */

#endif
