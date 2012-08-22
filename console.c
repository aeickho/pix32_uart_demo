#include <p32xxxx.h>
#include <plib.h>
#include <string.h>

#include "p_queue.h"
#include "uart.h"
#include "console.h"

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

#define MAX_BUF			128

int mprintf(const char *format, ...);

int cmd_Led1On(void);
int cmd_Led1Off(void);
int cmd_Led2On(void);
int cmd_Led2Off(void);
int cmd_Ps(void);
int cmd_Reset(void);
int cmd_Dump(void);
int cmd_Timer(void);
int cmd_Help(void);

struct cmd_funcs cmd_head[] =		{
	{ "help",	cmd_Help	},
	{ "dump",	cmd_Dump	},
	{ "reset",	cmd_Reset	},
	{ "timer",	cmd_Timer	},
	{ "ps",		cmd_Ps		},
	{ "led1 on",	cmd_Led1On	},
	{ "led1 off",	cmd_Led1Off	},
	{ "led2 on",	cmd_Led2On	},
	{ "led2 off",	cmd_Led2Off	},
	{ NULL,		NULL		}
};


struct process_queue *p_main_queue;
struct uart_fifo uart2_fifo;

int cmd_Led1On(void) {
	mLED_1_On();
	return 0;
}

int cmd_Led1Off(void) {
	mLED_1_Off();
	return 0;
}

int cmd_Led2On(void) {
	mLED_2_On();
	return 0;
}

int cmd_Led2Off(void) {
	mLED_2_Off();
	return 0;
}

int cmd_Ps(void) {
	struct process_queue_func *p;

	for (p = p_main_queue->p_queue; p; p = p->next)
		mprintf("%s", p->func_name);

	mprintf("\r\n");
	return 0;
}

int cmd_Reset(void) {
	//__asm__()
	return 0;
}

int cmd_Dump(void) {
	char c;

	c = uart2_fifo.fifo_in.nchar + '0';
	mprintf("uart2_fifo.in_nchar = %c\r\n", c);

	c = uart2_fifo.fifo_out.nchar + '0';
	mprintf("uart2_fifo.out_nchar = %c\r\n");

	return 0;
}

int cmd_Timer(void) {
	//SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	INTEnableSystemMultiVectoredInt();

	return 0;
}

int cmd_Help(void) {
	struct cmd_funcs *cmd_ptr;

	mprintf("Available Commands:\r\n-------------------\r\n");
	for(cmd_ptr = cmd_head; cmd_ptr->cmd_name; cmd_ptr++)
		mprintf("       %s\r\n", cmd_ptr->cmd_name);

	mprintf("-------------------\r\n");

	return 0;
}


