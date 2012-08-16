/*
	Example "blinkenlights" program for the UBW32 and C32 compiler.
	Demonstrates software PWM, use of floating-point library, etc.

	IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present
	in the working directory when compiling code for the UBW32!  Failure
	to do so will almost certainly result in your UBW32 getting 'bricked'
	and requiring re-flashing the bootloader (which, if you don't have a
	PICkit 2 or similar PIC programmer, you're screwed).
	YOU HAVE BEEN WARNED.

	2/19/2009 - Phillip Burgess - pburgess@dslextreme.com
*/

#include <p32xxxx.h>
#include <plib.h>
#include <string.h>

#include "uart.h"

extern struct uart_fifo uart2_fifo;

int uart_init(UART_MODULE uart) {
	UARTConfigure(uart, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);

 	UARTSetFifoMode(uart, UART_INTERRUPT_ON_TX_NOT_FULL |
				UART_INTERRUPT_ON_RX_NOT_EMPTY);

	UARTSetLineControl(uart, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
				UART_STOP_BITS_1);

	UARTSetDataRate(uart, GetPeripheralClock(), 115200);

	UARTEnable(uart, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));

	/* Configure uart RX Interrupt */
	INTEnable(INT_SOURCE_UART_RX(uart), INT_ENABLED);

	/* INTEnable (INT_SOURCE_UART_TX (uart), INT_ENABLED); */
	INTSetVectorPriority(INT_VECTOR_UART(uart), INT_PRIORITY_LEVEL_2);

	INTSetVectorSubPriority(INT_VECTOR_UART(uart), INT_SUB_PRIORITY_LEVEL_0);

	return 0;
}

void uart_fifo_init(struct uart_fifo *fifo, int fifo_in_size,
			int fifo_out_size, int block_mode) {
	fifo->fifo_in.read_pos		= 0;
	fifo->fifo_in.write_pos		= 0;
	fifo->fifo_in.bufsize		= fifo_in_size;
	fifo->fifo_out.read_pos		= 0;
	fifo->fifo_out.write_pos	= 0;
	fifo->fifo_out.bufsize		= fifo_out_size;
	fifo->fifo_in.nchar		= 0;
	fifo->fifo_out.nchar		= 0;
	fifo->block_mode		= block_mode;

	fifo->fifo_in.buffer		= malloc(fifo_in_size + 1);
	fifo->fifo_out.buffer		= malloc(fifo_out_size + 1);
}

void uart_fifo_push(struct uart_buf *fifo, char ch) {
	fifo->buffer[fifo->write_pos] = ch;
	fifo->write_pos = (fifo->write_pos + 1) % fifo->bufsize;
	fifo->nchar++;
}

int uart_fifo_pop(struct uart_buf *fifo, char *ch) {
	if (fifo->nchar > 0) {
		*ch = fifo->buffer[fifo->read_pos];
		fifo->read_pos = (fifo->read_pos + 1) % fifo->bufsize;
		fifo->nchar--;
		return 0;
	}

	return -1;
}

void uart2_send(char *buffer, UINT32 size) {
	if (size <= 0)
		return;

	for (; size; size--) {
		uart_fifo_push(&uart2_fifo.fifo_out, (char)*buffer);
		buffer++;
	}

	INTEnable(INT_SOURCE_UART_TX (UART2), INT_ENABLED);

	if(uart2_fifo.block_mode)
		while(uart2_fifo.fifo_out.nchar > 0)
			;

}

void uart2_print(const char *buffer) {
	uart2_send((char *)buffer, strlen(buffer));
}

void uart_putchar(char c) {
	uart2_send(&c, 1);
}
