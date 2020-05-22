#include <arch/broadcom/bcm2835/irq.h>
#include <arch/broadcom/bcm2835/mailbox/property.h>
#include <arch/broadcom/bcm2835/peripherals.h>
#include <kernel/kassert.h>
#include <kernel/types.h>
#include "gpio.h"
#include "uart.h"

enum uart0_register
{
	DR     = (0x00 >> 2), // Data register
	RSRECR = (0x04 >> 2), // Receive Status Reg / Error Clear Reg
	FR     = (0x18 >> 2), // Flag register
	ILPR   = (0x20 >> 2), // IrDA register
	IBRD   = (0x24 >> 2), // Integer Baud rate divisor
	FBRD   = (0x28 >> 2), // Fractional Baud rate divisor
	LCRH   = (0x2c >> 2), // Line Control register
	CR     = (0x30 >> 2), // Control register
	IFLS   = (0x34 >> 2), // Interrupt FIFO Level Select Register
	IMSC   = (0x38 >> 2), // Interupt Mask Set Clear Register
	RIS    = (0x3c >> 2), // Raw Interupt Status Register
	MIS    = (0x40 >> 2), // Masked Interupt Status Register
	ICR    = (0x44 >> 2), // Interupt Clear Register
	DMACR  = (0x48 >> 2), // DMA Control Register
	ITCR   = (0x80 >> 2), // Test Control register
	ITIP   = (0x84 >> 2), // Integration test input reg
	ITOP   = (0x88 >> 2), // Integration test output reg
	TDR    = (0x8c >> 2), // Test Data reg
};

enum
{
	// Flag Register (depends on LCRH.FEN)
	FR_TXFE		= 1 << 7, // Transmit FIFO empty
	FR_RXFF		= 1 << 6, // Receive FIFO full
	FR_TXFF 	= 1 << 5, // Transmit FIFO full
	FR_RXFE 	= 1 << 4, // Receive FIFO empty
	FR_BUSY 	= 1 << 3, // BUSY transmitting data
	FR_CTS  	= 1 << 0, // Clear To Send

	// Line Control Register
	LCRH_SPS	= 1 << 7, // sticky parity selected
	LCRH_WLEN	= 3 << 5, // word length (5, 6, 7 or 8 bit)
	LCRH_WLEN5	= 0 << 5, // word length 5 bit
	LCRH_WLEN6	= 1 << 5, // word length 6 bit
	LCRH_WLEN7	= 2 << 5, // word length 7 bit
	LCRH_WLEN8	= 3 << 5, // word length 8 bit
	LCRH_FEN	= 1 << 4, // Enable FIFOs
	LCRH_STP2	= 1 << 3, // Two stop bits select
	LCRH_EPS	= 1 << 2, // Even Parity Select
	LCRH_PEN	= 1 << 1, // Parity enable
	LCRH_BRK	= 1 << 0, // send break

	// Control Register
	CR_CTSEN	= 1 << 15, // CTS hardware flow control
	CR_RTSEN	= 1 << 14, // RTS hardware flow control
	CR_RTS		= 1 << 11, // (not) Request to send
	CR_RXE		= 1 <<  9, // Receive enable
	CR_TXW		= 1 <<  8, // Transmit enable
	CR_LBE		= 1 <<  7, // Loopback enable
	CR_UARTEN	= 1 <<  0, // UART enable

	// Interrupts (IMSC / RIS / MIS / ICR)
	INT_ALL		= 0x7F2,
	INT_RX		= 0x10,
};

static volatile uint32_t* const uart0_base = (volatile uint32_t*)UART0_BASE;


static struct tty* uart0_console_tty = NULL;

void uart0_set_tty(struct tty* tty)
{
	uart0_console_tty = tty;
}

static void uart0_irq_handle(void)
{
	int c;

	if (uart0_base[MIS] & INT_RX) {
		c = uart0_base[DR];

		if (uart0_console_tty) {
			if (c == '\r') c = '\n';
			tty_input(uart0_console_tty, c);
		}

		// clear interrupt if receive fifo is empty
		if (uart0_base[FR] & FR_RXFE)
			uart0_base[ICR] = 0;
	}
}

void uart0_init(void)
{
	uint32_t uart_clock = 0;
	kassert(bcm2835_mailbox_property_get_uart_clock(&uart_clock) ==
		BCM2835_MAILBOX_PROPERTY_REQUEST_SUCCESS);

	// disable uart0
	uart0_base[CR] = 0;

	// function 0 for pins 14 & 15 is TX0 and RX0 respectively
	gpio_configure(14, GPIO_FN0, GPIO_PUD_OFF);
	gpio_configure(15, GPIO_FN0, GPIO_PUD_OFF);

	// Clear pending interrupts.
	uart0_base[ICR] = INT_ALL;

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 48000000; Baud = 115200.
	const uint32_t baud_16 = 16 * 115200;
	const uint32_t frac_64 = ((uart_clock % baud_16) * 64) / baud_16;
	uart0_base[IBRD] = uart_clock / baud_16;
	uart0_base[FBRD] = (2 * frac_64 + 1) / 2;

	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	uart0_base[LCRH] = LCRH_FEN | LCRH_WLEN8;

	uart0_base[IFLS] = 0; // 1/8 full

	//  enable RX interrupt.
	uart0_base[IMSC] = INT_RX;

	// Enable UART0, receive & transfer part of UART.
	uart0_base[CR] = CR_UARTEN | CR_TXW | CR_RXE;

	bcm2835_set_irq_handler(IRQ_UART, uart0_irq_handle);
}

void uart0_putchar(char c)
{
	// Wait for UART to become ready to transmit.
	while (uart0_base[FR] & FR_TXFF) { }

	uart0_base[DR] = c;
}

int uart0_getchar(void)
{
	// Wait for UART to have received something.
	while (uart0_base[FR] & FR_RXFE) { }

	return uart0_base[DR];
}

int uart0_puts(const char* str)
{
	int i;
	for (i = 0; str[i] != '\0'; i++)
		uart0_putchar(str[i]);
	return i;
}


/*
 * UART1 (mini UART)
 */
enum aux_uart1_register
{
	AUX_IRQ		= (0x00 >> 2), // Aux pending interrupts
	AUX_ENABLES	= (0x04 >> 2), // Auxiliary enables
	AUX_MU_IO_REG	= (0x40 >> 2), // Mini Uart I/O Data
	AUX_MU_IER_REG	= (0x44 >> 2), // Mini Uart Interrupt Enable
	AUX_MU_IIR_REG	= (0x48 >> 2), // Mini Uart Interrupt Identify
	AUX_MU_LCR_REG	= (0x4c >> 2), // Mini Uart Line Control
	AUX_MU_MCR_REG	= (0x50 >> 2), // Mini Uart Modem Control
	AUX_MU_LSR_REG	= (0x54 >> 2), // Mini Uart Line Status
	AUX_MU_MSR_REG	= (0x58 >> 2), // Mini Uart Modem Status
	AUX_MU_SCRATCH	= (0x5c >> 2), // Mini Uart Scratch
	AUX_MU_CNTL_REG = (0x60 >> 2), // Mini Uart Extra Control
	AUX_MU_STAT_REG = (0x60 >> 2), // Mini Uart Extra Status
	AUX_MU_BAUD_REG = (0x68 >> 2), // Mini Uart Baudrate
};

enum
{
	// AUX_MU_LSR_REG register
	TR_EMPTY = 1 << 5, // Transmitter empty

	AUX_INT_RX = 1 << 0,
	AUX_INT_TX = 1 << 1,
};

static volatile uint32_t* const aux_base = (volatile uint32_t*)AUX_BASE;


static struct tty* uart1_console_tty = NULL;

void uart1_set_tty(struct tty* tty)
{
	uart1_console_tty = tty;
}

static void uart1_irq_handle(void)
{
	int c;

	if (aux_base[AUX_MU_IIR_REG] & 0x4) {
		c = aux_base[AUX_MU_IO_REG];

		if (uart0_console_tty) {
			if (c == '\r') c = '\n';
			tty_input(uart0_console_tty, c);
		}

		// clear interrupt if receive fifo is empty
		if ((aux_base[AUX_MU_LSR_REG] & 0x1) == 0)
			aux_base[AUX_MU_IIR_REG] = 0x2;
	}
}

void uart1_init(void)
{
	gpio_configure(14, GPIO_FN5, GPIO_PUD_OFF);

	aux_base[AUX_ENABLES] = 1;
	aux_base[AUX_MU_IER_REG] = AUX_INT_RX; // enable receiver interrupt
	aux_base[AUX_MU_CNTL_REG] = 0;
	aux_base[AUX_MU_LCR_REG] = 1; // 8 bit
	aux_base[AUX_MU_MCR_REG] = 0;
	aux_base[AUX_MU_IIR_REG] = 0x6; // clear receive and transmit FIFOs
	aux_base[AUX_MU_BAUD_REG] = 270; // ((250000000 / 115200) / 8) − 1 = ~270

	aux_base[AUX_MU_CNTL_REG] = 3; // enable transmitter and receiver

	bcm2835_set_irq_handler(IRQ_AUX, uart1_irq_handle);
}

void uart1_putchar(char c)
{
	// Wait for UART to become ready to transmit.
	while (!(aux_base[AUX_MU_LSR_REG] & TR_EMPTY)) { }

	aux_base[AUX_MU_IO_REG] = c;

}

int uart1_getchar(void)
{
	return (aux_base[AUX_MU_LSR_REG] & 0x1) ? aux_base[AUX_MU_IO_REG] : -1;
}

int uart1_puts(const char* str)
{
	int i;
	for (i = 0; str[i] != '\0'; i++)
		uart1_putchar(str[i]);
	return i;
}
