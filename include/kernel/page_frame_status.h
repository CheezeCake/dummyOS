#ifndef _KPAGE_FRAME_H_
#define _KPAGE_FRAME_H_

enum page_frame_status
{
	PAGE_FRAME_RESERVED,
	PAGE_FRAME_KERNEL,
	PAGE_FRAME_HW_MAP,
	PAGE_FRAME_FREE
};

#endif