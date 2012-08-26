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

#define SYS_FREQ		(80000000L)
#define PB_DIV			8
#define PRESCALE		256
#define TOGGLES_PER_SEC		1
#define T1_TICK			(SYS_FREQ/PB_DIV/PRESCALE/TOGGLES_PER_SEC)


#define MAX_BUF			128

int mprintf(const char *format, ...);

int cmd_Led1On(void);
int cmd_Led1Off(void);
int cmd_Led2On(void);
int cmd_Led2Off(void);
int cmd_Ps(void);
int cmd_Reset(void);
int cmd_Dump(void);
int cmd_timer_start(void);
int cmd_timer_stop(void);
int cmd_timer_inc(void);
int cmd_timer_dec(void);
int cmd_Help(void);

int timer1_tick = T1_TICK;

struct cmd_funcs cmd_head[] =		{
	{ "help",	cmd_Help	},
	{ "dump",	cmd_Dump	},
	{ "reset",	cmd_Reset	},
	{ "start timer", cmd_timer_start},
	{ "stop timer",	cmd_timer_stop	},
	{ "inc timer",	cmd_timer_inc	},
	{ "dec timer",	cmd_timer_dec	},
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

int cmd_timer_start(void) {
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);

	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	INTEnableSystemMultiVectoredInt();
	mprintf("timer started with T1_TICK = %d\r\n", T1_TICK);

	return 0;
}

int cmd_timer_dec(void) {
	CloseTimer1();

	timer1_tick = (int)(timer1_tick * 0.9);
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, timer1_tick);
	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
	INTEnableSystemMultiVectoredInt();

	mprintf("timer started with T1_TICK = %d\r\n", timer1_tick);

	return 0;
}

int cmd_timer_inc(void) {
	CloseTimer1();

	timer1_tick = (int)(timer1_tick * 1.1);
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, timer1_tick);
	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
	INTEnableSystemMultiVectoredInt();

	mprintf("timer started with T1_TICK = %d\r\n", timer1_tick);

	return 0;
}

int cmd_timer_stop(void) {
	CloseTimer1();

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


