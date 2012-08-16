#ifndef HAVE_UART_H
#define HAVE_UART_H

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

#define SystemClock()		(40000000ul)
#define GetPeripheralClock()	(SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()	(SystemClock())

void uart_fifo_init(struct uart_fifo *fifo, int fifo_in_size, int fifo_out_size, int block_mode);
void uart_fifo_push(struct uart_buf *fifo, char ch);
int uart_fifo_pop(struct uart_buf *fifo, char *ch);
void uart2_send(char *buffer, UINT32 size);
void uart2_print(const char *buffer);

void ToUART2Fifo_in(const char character);
void ToUART2Fifo_out(const char character);
int FromUART2Fifo_in(void);
inline int FromUART2Fifo_out(void);
void UART2SendTrigger(void);
void UART2PutStr(const char *buffer);
void SendDataBuffer(const char *buffer, UINT32 size);
int uart_init(UART_MODULE uart);
int uart_poll(void);

void uart_putchar(char c);

#endif
