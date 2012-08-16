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

#define SystemClock()		(40000000ul)
#define GetPeripheralClock()	(SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()	(SystemClock())

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

#define MAX_BUF			100

int send = 0;

struct UARTFifo {
	int in_read_pos;
	int in_write_pos;
	int out_read_pos;
	int out_write_pos;
	volatile int in_nchar;
	volatile int out_nchar;
	int bufsize;
	char *in;
	char *out;
	int out_block;
} UART2Fifo;

int CmdHelp(void);
int CmdLs(void);
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

struct CmdFunc {
	char *cmdName;
	int (*funcPtr)(void);
} Cmds[] = {
	{ "help",	CmdHelp		},
	{ "dump",	CmdDump		},
	{ "ls",		CmdLs		},
	{ "led1 on",	CmdLed1On	},
	{ "led1 off",	CmdLed1Off	},
	{ "led2 on",	CmdLed2On	},
	{ "led2 off",	CmdLed2Off	},
	{ NULL,		NULL		}
};

void UART2FifoInit(int out_block) {
	UART2Fifo.in_read_pos	= 0;
	UART2Fifo.in_write_pos	= 0;
	UART2Fifo.out_read_pos	= 0;
	UART2Fifo.out_write_pos	= 0;
	UART2Fifo.in_nchar	= 0;
	UART2Fifo.out_nchar	= 0;
	UART2Fifo.out_block	= out_block;
	UART2Fifo.bufsize	= MAX_BUF;

	UART2Fifo.in = malloc(UART2Fifo.bufsize+1);
	UART2Fifo.out = malloc(UART2Fifo.bufsize+1);
}


// Add one character to output fifo 
void ToUART2Fifo_in(const char character) {
	UART2Fifo.in[UART2Fifo.in_write_pos] = character;
	UART2Fifo.in_write_pos = (UART2Fifo.in_write_pos + 1) % UART2Fifo.bufsize;
	UART2Fifo.in_nchar++;
}

void ToUART2Fifo_out(const char character) {
	UART2Fifo.out[UART2Fifo.out_write_pos] = character;
	UART2Fifo.out_write_pos = (UART2Fifo.out_write_pos + 1) % UART2Fifo.bufsize;
	UART2Fifo.out_nchar++;
}

int FromUART2Fifo_in(void) {
	int in = -1;

	if (UART2Fifo.in_nchar > 0) {
		in = UART2Fifo.in[UART2Fifo.in_read_pos];
		UART2Fifo.in_read_pos =
			(UART2Fifo.in_read_pos + 1) % UART2Fifo.bufsize;
		UART2Fifo.in_nchar--;
	}

	return in;
}

inline int FromUART2Fifo_out(void) {
	int in = -1;

	if (UART2Fifo.out_nchar > 0) {
		in = UART2Fifo.out[UART2Fifo.out_read_pos];
		UART2Fifo.out_read_pos =
			(UART2Fifo.out_read_pos + 1) % UART2Fifo.bufsize;
		UART2Fifo.out_nchar--;
	}

	return in;
}

void UART2SendTrigger(void) {
	//if (send == 0) {
	INTEnable(INT_SOURCE_UART_TX (UART2), INT_ENABLED);
	send = 1;
	//}
}

void UART2Send(const char *buffer, UINT32 size) {
	while (size) {
		ToUART2Fifo_out(*buffer);
		buffer++;
		size--;
	}
	UART2SendTrigger();

	if(UART2Fifo.out_block)
		while(UART2Fifo.out_nchar > 0)
			;
}

void UART2PutStr(const char *buffer) {
	UART2Send(buffer, strlen (buffer));
}

void __ISR (_UART2_VECTOR, IPL2SOFT) IntUart2Handler(void) {
	int val;

	// Is this an RX interrupt?
	if (INTGetFlag (INT_SOURCE_UART_RX(UART2))) {
		//mLED_1_On();
		val = UARTGetDataByte(UART2);
		ToUART2Fifo_in((const char)val);
		INTClearFlag(INT_SOURCE_UART_RX(UART2));
	}

	// We don't care about TX interrupt
	if (INTGetFlag(INT_SOURCE_UART_TX(UART2))) {
		val = FromUART2Fifo_out();
		if (val > 0) {
			// darf eigendlich nie anliegen
			while (!UARTTransmitterIsReady(UART2))
				;

			UARTSendDataByte(UART2, val);
			INTClearFlag(INT_SOURCE_UART_TX(UART2));
			send = 1;
		} else {
			send = 0;
			INTEnable(INT_SOURCE_UART_TX (UART2), INT_DISABLED);
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

int UARTInit(UART_MODULE uart) {
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

int main(void) {
	int val;
	char cmd[MAX_BUF], *p = cmd;
	struct CmdFunc *CmdPtr;

	UART2FifoInit(1);

	UARTInit(UART2);

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

	UART2PutStr("\r\nHallo NOKLAB!\r\n> ");

/*#define DELAYA 22156
	T1CON = 0x8030;
	PR1 = 0xffff;
	TMR1 = 0;
	while (TMR1 < DELAYA)
		;
*/
	*p = 0;
	while (1) {
		if (UART2Fifo.in_nchar == 0)
			continue;
		
		val = FromUART2Fifo_in();
		if (val < 0)
			continue;

		*p++ = (unsigned char)val;
		UART2Send((char *)&val, 1);
		*p = 0;

		if ((int)(p - val) >= 100) {
			p = cmd;
			*p = 0;
			UART2PutStr("error: command too long... resetting\r\n");
		}

		if ((unsigned char)val == '\r') {
			*--p = 0;
			UART2PutStr("\n");
			for (CmdPtr = Cmds; CmdPtr->cmdName; CmdPtr++) {
				if (!strncmp(cmd, CmdPtr->cmdName, MAX_BUF)) {
					CmdPtr->funcPtr();
					goto ok;
				}
			}

			UART2PutStr(cmd);
			UART2PutStr(": command not found\r\n");

ok:
			UART2PutStr("> ");
			p = cmd;
			*p = 0;
		}
	}
}

int CmdDump(void) {
	char c;

	c = UART2Fifo.in_nchar + '0';
	UART2PutStr("UART2Fifo.in_nchar = ");
	UART2Send(&c, 1);
	UART2PutStr("\r\n");

	c = UART2Fifo.out_nchar + '0';
	UART2PutStr("UART2Fifo.out_nchar = ");
	UART2Send(&c, 1);
	UART2PutStr("\r\n");

	return 0;
}

int CmdLs(void) {
	UART2PutStr(". ..\r\n");

	return 0;
}

int CmdHelp(void) {
	struct CmdFunc *CmdPtr;

	UART2PutStr("available commands:\r\n");
	for(CmdPtr = Cmds; CmdPtr->cmdName; CmdPtr++) {
		UART2PutStr("       ");
		UART2PutStr(CmdPtr->cmdName);
		UART2PutStr("\r\n");
	}

	return 0;
}
