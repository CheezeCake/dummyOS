#ifndef _I8254_
#define _I8254_

#define I8254_FREQUENCY 1193182
#define I8254_MAX_FREQUENCY_DIVIDER 65536

#define I8254_CHANNEL0_PORT 0x40

#define I8254_MODE_COMMAND 0x43

int i8254_set_tick_interval(unsigned int ms);

#endif
