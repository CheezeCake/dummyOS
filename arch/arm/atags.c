#include "atags.h"

static inline const struct atag* next_tag(const struct atag* tag)
{
	return (const struct atag*)((uint32_t*)tag + tag->hdr.size);
}

uint32_t atags_get_ram_size(const struct atag* atags)
{
	while (atags->hdr.tag != ATAG_NONE) {
		if (atags->hdr.tag == ATAG_MEM)
			return atags->u.mem.size;

		atags = next_tag(atags);
	}

	return 0;
}
