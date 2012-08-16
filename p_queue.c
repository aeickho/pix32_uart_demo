#define P_FUNCP(x) int (*x)(void)

struct process_queue {
	struct process_queue *next;
	int (*p_func)(void);
};


void process_queue_init(struct process_queue **p_queue_head, P_FUNCP(p_func)) {
	*p_queue_head = malloc(sizeof(struct process_queue));

	(*p_queue_head)->next = NULL;
	(*p_queue_head)->p_func = p_func;
}

void process_queue_push(struct process_queue *p_queue_head, P_FUNCP(p_func)) {
	struct process_queue *p = p_queue_head;

	for (p = p_queue_head; p->next; p = p->next)
		;

	p->next = malloc(sizeof(struct process_queue));
	p = p->next;
	p->p_func = p_func;
	p->next = NULL;
}
