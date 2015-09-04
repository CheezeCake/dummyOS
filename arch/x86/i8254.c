#include "i8254.h"
#include "io_ports.h"

int i8254_set_frequency(unsigned int frequency)
{
	// compute the value of the counter needed to get a
	// interupt at approximately frequency Hz
	unsigned int counter = I8254_FREQUENCY / frequency;

	if (counter > I8254_MAX_FREQUENCY_DIVIDER)
		return -1;

	// for the chip 0 = I8254_MAX_FREQUENCY_DIVIDER,
	// because I8254_MAX_FREQUENCY_DIVIDER cannot fit on 16 bits
	// and you can't divide by 0
	if (counter == I8254_MAX_FREQUENCY_DIVIDER)
		counter = 0;

	// Channel: 0
	// Access mode: lobyte/hibyte
	// Operating mode: rate generator
	// http://wiki.osdev.org/Programmable_Interval_Timer#I.2FO_Ports
	outb(I8254_MODE_COMMAND, 0x34);

	// low 8 bits first
	outb(I8254_CHANNEL0_PORT, counter & 0xff);
	outb(I8254_CHANNEL0_PORT, (counter >> 8) & 0xff);

	return 0;
}
