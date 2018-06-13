#ifndef _DUMMYOS_KEYMAP_H_
#define _DUMMYOS_KEYMAP_H_

#include <kernel/types.h>

typedef uint16_t const keyboard_map_t[128][3];

enum keyboard_state {
	KBD_REGULAR,
	KBD_SHIFT,
	KBD_CTRL
};

#define CTRL(c) ((c) & 0x1f)

#endif
