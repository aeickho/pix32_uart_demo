#include <p32xxxx.h>
#include <plib.h>
#include <string.h>

#include "p_queue.h"
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

#define SYS_FREQ		(80000000L)
#define PB_DIV			8
#define PRESCALE		256
#define TOGGLES_PER_SEC		1
#define T1_TICK			(SYS_FREQ/PB_DIV/PRESCALE/TOGGLES_PER_SEC)

#define MAX_BUF			128

int mprintf(const char *format, ...);

int send = 0;

struct process_queue *p_main_queue;
struct uart_fifo uart2_fifo;

int CmdHelp(void);
int CmdTimer(void);
int CmdDump(void);

int CmdLed1On(void) {
	mLED_1_On();
	return 0;
}

int CmdLed1Off(void) {
	mLED_1_Off();
	return 0;
}

int CmdLed2On(void) {
	mLED_2_On();
	return 0;
}

int CmdLed2Off(void) {
	mLED_2_Off();
	return 0;
}

int CmdPs(void) {
	struct process_queue_func *p;

	for (p = p_main_queue->p_queue; p; p = p->next)
		mprintf("%s", p->func_name);

	mprintf("\r\n");
	return 0;
}

int CmdReset(void) {
	//__asm__()
	return 0;
}

struct CmdFunc {
	char *cmdName;
	int (*funcPtr)(void);
} Cmds[] = {
	{ "help",	CmdHelp		},
	{ "dump",	CmdDump		},
	{ "reset",	CmdReset	},
	{ "timer",	CmdTimer	},
	{ "ps",		CmdPs		},
	{ "led1 on",	CmdLed1On	},
	{ "led1 off",	CmdLed1Off	},
	{ "led2 on",	CmdLed2On	},
	{ "led2 off",	CmdLed2Off	},
	{ NULL,		NULL		}
};

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
	mT1ClearIntFlag();

	mprintf(".");
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

void SendDataBuffer(const char *buffer, UINT32 size) {
	while (size) {
		while (!UARTTransmitterIsReady(UART2))
			;

		UARTSendDataByte(UART2, *buffer);
		mPORTAToggleBits(BIT_10);
		buffer++;
		size--;
	}
	
	while (!UARTTransmissionHasCompleted(UART2))
		;
}

int uart_poll(void) {
	static char cmd[MAX_BUF], *p = cmd;
	char ch;

	*p = 0;
	if (uart2_fifo.fifo_in.nchar == 0)
		return 0;

	if (uart_fifo_pop(&uart2_fifo.fifo_in, &ch) < 0)
		return 0;

	*p++ = ch;
	uart2_send(&ch, 1);
	*p = 0;

	if ((int)(p - cmd) >= 100) {
		p = cmd;
		*p = 0;
		mprintf("\r\nerror: command too long... resetting\r\n> ");
	}

	if ((unsigned char)ch == '\r') {
		struct CmdFunc *CmdPtr;

		*--p = 0;
		mprintf("\n");
		for (CmdPtr = Cmds; CmdPtr->cmdName; CmdPtr++) {
			if (!strncmp(cmd, CmdPtr->cmdName, MAX_BUF)) {
				CmdPtr->funcPtr();
				goto ok;
			}
		}

		mprintf("%s: command not found\r\n", cmd);

ok:
		mprintf("> ");
		p = cmd;
		*p = 0;
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
	/* mPORTASetPinsDigitalOut (BIT_10); */

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

int CmdDump(void) {
	char c;

	c = uart2_fifo.fifo_in.nchar + '0';
	mprintf("uart2_fifo.in_nchar = %c\r\n", c);

	c = uart2_fifo.fifo_out.nchar + '0';
	mprintf("uart2_fifo.out_nchar = %c\r\n");

	return 0;
}

int CmdTimer(void) {
	//SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	INTEnableSystemMultiVectoredInt();

	return 0;
}

int CmdHelp(void) {
	struct CmdFunc *CmdPtr;

	mprintf("Available Commands:\r\n-------------------\r\n");
	for(CmdPtr = Cmds; CmdPtr->cmdName; CmdPtr++) {
		mprintf("       ");
		mprintf(CmdPtr->cmdName);
		mprintf("\r\n");
	}
	mprintf("-------------------\r\n");

	return 0;
}
