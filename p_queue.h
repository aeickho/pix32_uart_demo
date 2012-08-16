#define P_FUNCP(x) int (*x)(void)
#define MAX_PROCS 10

struct process_queue_func {
	char *func_name;
	struct process_queue_func *next;
	P_FUNCP(p_func);
	int priority;
};

struct process_queue {
	int max_processes;
	int num_processes;
	struct process_queue_func *p_queue;
};

int process_queue_init(struct process_queue **p_queue_head,
				P_FUNCP(p_func), char *name, int max_procs);

int process_queue_push(struct process_queue *p_queue_head,
				P_FUNCP(p_func), char *name, int prio);

int process_run_queue(struct process_queue *p_queue_head);
