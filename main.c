#include <p32xxxx.h>
#include <plib.h>
#include <string.h>

#include "p_queue.h"
#include "console.h"
#include "uart.h"

#define mLED_1			LATBbits.LATB15
#define mLED_2			LATAbits.LATA10
#define mGetLED_1()		mLED_1
#define mGetLED_2()		mLED_2
#define mLED_1_On()		mLED_1 = 1
#define mLED_2_On()		mLED_2 = 1
#define mLED_1_Off()		mLED_1 = 0
#define mLED_2_Off()		mLED_2 = 0
#define mLED_1_Toggle()		mLED_1 = !mLED_1
#define mLED_2_Toggle()		mLED_2 = !mLED_2

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_8

#define MAX_BUF			128

int mprintf(const char *format, ...);

int send = 0;

struct process_queue *p_main_queue;
struct uart_fifo uart2_fifo;

extern struct cmd_funcs cmd_head[];

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
	mT1ClearIntFlag();

	mLED_1 = !mLED_1;
}

void __ISR (_UART2_VECTOR, IPL2SOFT) IntUart2Handler(void) {
	char ch;

	/* RX interrupt */
	if (INTGetFlag (INT_SOURCE_UART_RX(UART2))) {
		ch = (char)UARTGetDataByte(UART2);
		uart_fifo_push(&uart2_fifo.fifo_in, (char)ch);
		INTClearFlag(INT_SOURCE_UART_RX(UART2));
	}

	/* TX interrupt */
	if (INTGetFlag(INT_SOURCE_UART_TX(UART2))) {
		if (uart_fifo_pop(&uart2_fifo.fifo_out, &ch) < 0)
			INTEnable(INT_SOURCE_UART_TX (UART2), INT_DISABLED);
		else {
			while (!UARTTransmitterIsReady(UART2))
				;

			UARTSendDataByte(UART2, ch);
			INTClearFlag(INT_SOURCE_UART_TX(UART2));
		}
	}
}

/* strip leading, tailing spaces routine taken from linux kernel */
char *strstrip(char *s) {
	int size;
	char *end;
	
	size = strlen(s);
	
	if (!size)
		return s;
	
	end = s + size - 1;
	while (end >= s && *end == ' ')
		end--;
	*(end + 1) = '\0';
	
	while (*s && *s == ' ')
		s++;
	
	return s;
}

int uart_poll(void) {
	struct cmd_funcs *cmd_ptr;
	static char cmd[MAX_BUF], *p = cmd;
	static int escape_code = 0;
	char ch;

	*p = 0;
	if (uart2_fifo.fifo_in.nchar == 0)
		return 0;

	if (uart_fifo_pop(&uart2_fifo.fifo_in, &ch) < 0)
		return 0;

/*	if (ch >= 32 && ch < 127) {
		*p++ = ch;
		uart2_send(&ch, 1);
		*p = 0;
	}
       
	if ((int)(p - cmd) >= MAX_BUF-1) {
		p = cmd;
		*p = 0;
		mprintf("\r\nerror: command too long... resetting\r\n> ");
	}
*/

	switch(ch) {
		/* control characters */
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 9:
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			break;
		case 27:
			escape_code=1;
			mprintf("escape\r\n");
			break;
		case 28:
		case 29:
		case 30:
		case 31:
			mprintf("recevied code: %d\n", ch);
			break;
		/* carriage return */
		case '\r':
			*p = 0;
			p = strstrip(cmd);
			mprintf("\r\n");
			for (cmd_ptr = cmd_head; cmd_ptr->cmd_name; cmd_ptr++) {
				if (!strncmp(p, cmd_ptr->cmd_name, MAX_BUF)) {
					cmd_ptr->func_ptr();
					goto ok;
				}
			}

			if (strlen(p))
				mprintf("%s: command not found\r\n", p);
ok:
			mprintf("> ");
			p = cmd;
			*p = 0;

			break;
		/* backspace */
		case 8:
		case 127:
			if (p - cmd >= 1) {
				*--p = 0;
				mprintf("\b \b");
			}
			break;
		/* readable characters */
		default:
			if (escape_code == 0) {
				*p++ = ch;
				uart2_send(&ch, 1);
				*p = 0;
			} else {
				if (escape_code == 1 && ch == '[') {
					mprintf("escape increased\r\n");
					escape_code++;
					break;
				}
				if (escape_code == 2) {
					switch (ch) {
						case 'A':
							mprintf("up move\r\n");
							escape_code = 0;
							break;
						case 'B':
							mprintf("down move\r\n");
							escape_code = 0;
							break;
						case 'D':
							mprintf("left move\r\n");
							escape_code = 0;
							break;
						case 'C':
							mprintf("right move\r\n");
							escape_code = 0;
							break;
						/* XXX check for tailing ~ */
						case '1':
							//mprintf("home\r\n");
						case '2':
							//mprintf("insert\r\n");
						case '3':
							//mprintf("delete\r\n");
						case '4':
							//mprintf("end\r\n");
						case '5':
							//mprintf("PgUp\r\n");
						case '6':
							mprintf("unhandled escape_char: %c\r\n", ch);
							//mprintf("PgDn\r\n");
							escape_code++;
							break;
						default:
							escape_code=0;
							mprintf("unhandled escape_char: %c\r\n", ch);
							break;
					}

					break;
				}
			}
			break;
	}

	return 0;
}

int main(void) {

	uart_fifo_init(&uart2_fifo, MAX_BUF, MAX_BUF, UART_BLOCKING);
	uart_init(UART2);

	/* Configure PB frequency and wait states */
	SYSTEMConfigPerformance(40000000L);
	RPC9R = 2;
	U2RXR = 6;
	/*
	TRISCbits.TRISC8 = 1;
	TRISCbits.TRISC9 = 0;
	*/

	/* configure for multi-vectored mode */
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

	/* enable interrupts */
	INTEnableInterrupts();
	LATAbits.LATA10   = 0;
	TRISAbits.TRISA10 = 0;
	LATBbits.LATB15   = 0;
	TRISBbits.TRISB15 = 0;

#define DELAY 2156
	T1CON = 0x8030;
	PR1 = 0xffff;
	TMR1 = 0;
	while (TMR1 < DELAY)
		;

	mprintf("\r\nHallo NOKLAB!\r\n> ");

	if (process_queue_init(&p_main_queue, uart_poll, "uart_poll", 10) < 0)
		mprintf("error: unable to allocate memory\r\n> ");

	while (1)
		process_run_queue(p_main_queue);
}
