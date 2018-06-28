#ifndef _DUMMYOS_DIRENT_H_
#define _DUMMYOS_DIRENT_H_

#include <stdint.h>

#define NAME_MAX	255
#define MAXNAMLEN	NAME_MAX

#define DT_BLK      0 /* This is a block device.		*/
#define DT_CHR      1 /* This is a character device.	*/
#define DT_DIR      2 /* This is a directory.			*/
#define DT_FIFO     3 /* This is a named pipe (FIFO).	*/
#define DT_LNK      4 /* This is a symbolic link.		*/
#define DT_REG      6 /* This is a regular file.		*/
#define DT_SOCK     7 /* This is a UNIX domain socket.	*/
#define DT_UNKNOWN  8 /* The file type is unknown.		*/

struct dirent
{
	uint32_t d_ino;
	uint16_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[];
};

#endif
