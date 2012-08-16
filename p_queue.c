#define P_FUNCP(x) int (*x)(void)
#define MAX_PROCS 10

struct process_queue_func {
	struct process_queue_func *next;
	int (*p_func)(void);
	int priority;
};

struct process_queue {
	int max_processes;
	int num_processes;
	struct process_queue_func *p_queue;
};

int process_queue_init(struct process_queue **p_queue_head,
				P_FUNCP(p_func), int max_procs) {

	*p_queue_head = malloc(sizeof(struct process_queue));
	if (!*p_queue_head)
		return -1;

	(*p_queue_head)->max_processes = max_procs;

	(*p_queue_head)->p_queue = malloc(sizeof(struct process_queue_func));
	if (!(*p_queue_head)->p_queue)
		return -1;

	(*p_queue_head)->p_queue->next = NULL;
	(*p_queue_head)->p_queue->p_func = p_func;
	(*p_queue_head)->num_processes = 1;

	return 0;
}

int process_queue_push(struct process_queue *p_queue_head,
				P_FUNCP(p_func), int prio) {
	struct process_queue_func *p;

	for (p = p_queue_head->p_queue; p->next; p = p->next)
		;

	p->next = malloc(sizeof(struct process_queue_func));
	if (!p->next)
		return -1;

	/* exceeded maximum processes in queue */
	if (p_queue_head->num_processes + 1 > p_queue_head->max_processes)
		return -1;

	p = p->next;
	p->p_func = p_func;
	p->priority = prio;
	p->next = NULL;
	p_queue_head->num_processes++;

	/* XXX: priority sorting */
	return 0;
}
