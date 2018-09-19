#ifndef _DUMMYOS_STAT_H_
#define _DUMMYOS_STAT_H_

#include <dummyos/types.h>

struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;

	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
};

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */
#define	S_ISVTX		0001000	/* save swapped text even after use */
#define	S_IREAD		0000400	/* read permission, owner */
#define	S_IWRITE 	0000200	/* write permission, owner */
#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */

#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#endif
