#include <p32xxxx.h>
#include <plib.h>

#include "p_queue.h"

int process_queue_init(struct process_queue **p_queue_head,
				P_FUNCP(p_func), char *name, int max_procs) {

	*p_queue_head = malloc(sizeof(struct process_queue));
	if (!*p_queue_head)
		return -1;

	(*p_queue_head)->max_processes = max_procs;

	(*p_queue_head)->p_queue = malloc(sizeof(struct process_queue_func));
	if (!(*p_queue_head)->p_queue)
		return -2;

	(*p_queue_head)->p_queue->next = NULL;
	(*p_queue_head)->p_queue->p_func = p_func;
	(*p_queue_head)->p_queue->func_name = name;
	(*p_queue_head)->num_processes = 1;

	return 0;
}

int process_queue_push(struct process_queue *p_queue_head,
				P_FUNCP(p_func), char *name, int prio) {
	struct process_queue_func *p;

	for (p = p_queue_head->p_queue; p->next; p = p->next)
		;

	p->next = malloc(sizeof(struct process_queue_func));
	if (!p->next)
		return -1;

	/* exceeded maximum processes in queue */
	if (p_queue_head->num_processes + 1 > p_queue_head->max_processes)
		return -2;

	p = p->next;
	p->p_func = p_func;
	p->func_name = name;
	p->priority = prio;
	p->next = NULL;
	p_queue_head->num_processes++;

	/* XXX: priority sorting */
	return 0;
}

int process_run_queue(struct process_queue *p_queue_head) {
	struct process_queue_func *p;

	for (p = p_queue_head->p_queue; p; p = p->next)
		p->p_func();

	return 0;
}
