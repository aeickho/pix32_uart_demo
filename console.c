#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <diskio.h>
#include <spi.h>


#include "p_queue.h"
#include "uart.h"
#include "console.h"
/*
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
*/

#define SYS_FREQ		(80000000L)
#define PB_DIV			8
#define PRESCALE		256
#define TOGGLES_PER_SEC		1
#define T1_TICK			(SYS_FREQ/PB_DIV/PRESCALE/TOGGLES_PER_SEC)
#define MAX_BUF			128

int mprintf(const char *format, ...);

int cmd_Led1On(char *arg);
int cmd_Led1Off(char *arg);
int cmd_Led2On(char *arg);
int cmd_Led2Off(char *arg);
int cmd_ps(char *arg);
int cmd_exec(char *arg);
int cmd_read(char *arg);
int cmd_timer_start(char *arg);
int cmd_timer_stop(char *arg);
int cmd_timer_inc(char *arg);
int cmd_timer_dec(char *arg);
int cmd_help(char *arg);
int cmd_sdinit(char *arg);

int timer1_tick = T1_TICK;

struct cmd_funcs cmd_head[] =		{
	{ "help",	cmd_help	},
	{ "read",	cmd_read	},
	{ "exec",	cmd_exec	},
	{ "sd_init",	cmd_sdinit	},
/*	{ "start timer", cmd_timer_start},
	{ "stop timer",	cmd_timer_stop	},
	{ "inc timer",	cmd_timer_inc	},
	{ "dec timer",	cmd_timer_dec	},
	{ "ps",		cmd_ps		},
	{ "led1 on",	cmd_Led1On	},
	{ "led1 off",	cmd_Led1Off	},
	{ "led2 on",	cmd_Led2On	},
	{ "led2 off",	cmd_Led2Off	},*/
	{ NULL,		NULL		}
};


struct process_queue *p_main_queue;
struct uart_fifo uart2_fifo;

int cmd_sdinit(char *arg) {
	unsigned int ret;
	spi_sd_init();

	ret = disk_initialize();

	mprintf("disk_initialization: %d\n", ret);

	return 0;
}

int cmd_Led1On(char *arg) {
	mLED_1_On();
	return 0;
}

int cmd_Led1Off(char *arg) {
	mLED_1_Off();
	return 0;
}

int cmd_Led2On(char *arg) {
	mLED_2_On();
	return 0;
}

int cmd_Led2Off(char *arg) {
	mLED_2_Off();
	return 0;
}

int cmd_ps(char *arg) {
	struct process_queue_func *p;

	for (p = p_main_queue->p_queue; p; p = p->next)
		mprintf("%s", p->func_name);

	mprintf("\r\n");
	return 0;
}

int cmd_exec(char *arg) {
	//__asm__()
	return 0;
}

int cmd_read(char *arg) {
	char buf[512];
	unsigned int addr;

	addr = atoi(buf);
	mprintf("reading string from %d: %s\r\n", addr, buf);
	disk_read(0, (PF_BYTE *)buf, 70000, 1);

	return 0;
}

int cmd_timer_start(char *arg) {
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);

	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	mprintf("timer started with T1_TICK = %d\r\n", T1_TICK);

	return 0;
}

int cmd_timer_dec(char *arg) {
	CloseTimer1();

	timer1_tick = (int)(timer1_tick * 0.9);
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, timer1_tick);
	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	mprintf("timer started with T1_TICK = %d\r\n", timer1_tick);

	return 0;
}

int cmd_timer_inc(char *arg) {
	CloseTimer1();

	timer1_tick = (int)(timer1_tick * 1.1);
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, timer1_tick);
	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

	mprintf("timer started with T1_TICK = %d\r\n", timer1_tick);

	return 0;
}

int cmd_timer_stop(char *arg) {
	CloseTimer1();

	return 0;
}

int cmd_help(char *arg) {
	struct cmd_funcs *cmd_ptr;

	mprintf("Available Commands: %s\r\n-------------------\r\n", !arg?"(null)":arg);
	for(cmd_ptr = cmd_head; cmd_ptr->cmd_name; cmd_ptr++)
		mprintf(" %s\r\n", cmd_ptr->cmd_name);

	mprintf("-------------------\r\n");

	return 0;
}


