#ifndef _ATAGS_H_
#define _ATAGS_H_

#include <kernel/types.h>

// www.simtec.co.uk/products/SWLINUX/files/booting_article.html

enum atag_tag
{
	ATAG_NONE		= 0x00000000,
	ATAG_CORE		= 0x54410001,
	ATAG_MEM		= 0x54410002,
	ATAG_VIDEOTEXT	= 0x54410003,
	ATAG_RAMDISK	= 0x54410004,
	ATAG_INITRD2	= 0x54420005,
	ATAG_SERIAL		= 0x54410006,
	ATAG_REVISION	= 0x54410007,
	ATAG_VIDEOLFB	= 0x54410008,
	ATAG_CMDLINE	= 0x54410009
};

struct atag_header
{
	uint32_t size; /* legth of tag in words including this header */
	uint32_t tag;  /* tag value */
};

struct atag_mem
{
	uint32_t size;   /* size of the area */
	uint32_t start;  /* physical start address */
};

struct atag_cmdline
{
	char cmdline[1];     /* this is the minimum size */
};

struct atag
{
	struct atag_header hdr;
	union
	{
		/* struct atag_core         core; */
		struct atag_mem          mem;
		/* struct atag_videotext    videotext; */
		/* struct atag_ramdisk      ramdisk; */
		/* struct atag_initrd2      initrd2; */
		/* struct atag_serialnr     serialnr; */
		/* struct atag_revision     revision; */
		/* struct atag_videolfb     videolfb; */
		struct atag_cmdline      cmdline;
	} u;
};

uint32_t atags_get_ram_size(const struct atag* atags);

#endif
