#ifndef HAVE_UART_H
#define HAVE_UART_H

#define SystemClock()		(40000000ul)
#ifndef GetPeripheralClock
#define GetPeripheralClock()	(SystemClock()/(1 << OSCCONbits.PBDIV))
#endif
//#define GetInstructionClock()	(SystemClock())

#define UART_BLOCKING 1

struct uart_buf {
	char *buffer;
	int read_pos;
	int write_pos;
	int bufsize;
	volatile int nchar;
};

struct uart_fifo {
	struct uart_buf fifo_in;
	struct uart_buf fifo_out;
	int block_mode;
};

int uart_init(UART_MODULE uart);

void uart_fifo_init(struct uart_fifo *fifo, int fifo_in_size,
		int fifo_out_size, int block_mode);
int uart_poll(void);

void uart_fifo_push(struct uart_buf *fifo, char ch);
int uart_fifo_pop(struct uart_buf *fifo, char *ch);

void uart2_send(char *buffer, UINT32 size);
void uart2_print(const char *buffer);

void uart_putchar(char c);

#endif
