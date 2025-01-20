#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 80

void fcfs();
void rr();

#define PROC_NEW    0
#define PROC_STOPPED 1
#define PROC_RUNNING 2
#define PROC_EXITED 3

int active_procs = 0;
int numOfCpus = 1;
struct single_queue running_q;  // Track running processes

typedef struct proc_desc {
    struct proc_desc *next;
    char name[80];
    int pid;
    int status;
    int reqCores;
    double t_submission, t_start, t_end;
} proc_t;

struct single_queue {
    proc_t *first;
    proc_t *last;
    long members;
};

struct single_queue global_q;

#define proc_queue_empty(q) ((q)->first == NULL)

void proc_queue_init(register struct single_queue *q) {
    q->first = q->last = NULL;
    q->members = 0;
}

void proc_to_rq_end(register proc_t *proc, struct single_queue *q) {
    if (proc_queue_empty(q)) {
        q->first = q->last = proc;
    } else {
        q->last->next = proc;
        q->last = proc;
        proc->next = NULL;
    }
}

proc_t *proc_rq_dequeue() {
    register proc_t *proc;

    proc = global_q.first;
    if (proc == NULL) return NULL;

    global_q.first = proc->next;
    proc->next = NULL;

    return proc;
}

void print_queue(struct single_queue *q) {
    proc_t *proc = q->first;
    while (proc != NULL) {
        printf("proc: [name:%s pid:%d]\n", proc->name, proc->pid);
        proc = proc->next;
    }
}

double proc_gettime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)(tv.tv_sec + tv.tv_usec / 1000000.0);
}

#define FCFS 0
#define RR   1

int policy = FCFS;
int quantum = 100; /* ms */
proc_t *running_proc;
double global_t;

void err_exit(char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

int main(int argc, char **argv) {
    FILE *input;
    char exec[80];
    int c;
    proc_t *proc;

    if (argc < 2) {
        err_exit("invalid usage");
    }

    if (!strcmp(argv[1], "FCFS")) {
        policy = FCFS;
        input = fopen(argv[2], "r");
        if (argc > 3) numOfCpus = atoi(argv[3]);
        if (input == NULL) err_exit("invalid input file name");
    } else if (!strcmp(argv[1], "RR")) {
        policy = RR;
        quantum = atoi(argv[2]);
        input = fopen(argv[3], "r");
        if (argc > 4) numOfCpus = atoi(argv[4]);
        if (input == NULL) err_exit("invalid input file name");
    } else {
        err_exit("invalid usage");
    }

    /* Read input file */
    while ((c = fscanf(input, "%s", exec)) != EOF) {
        proc = malloc(sizeof(proc_t));
        if (!proc) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        proc->next = NULL;
        strcpy(proc->name, exec);
        proc->pid = -1;
        proc->status = PROC_NEW;
        proc->t_submission = proc_gettime();

        int numCores;
        if (fscanf(input, "%d", &numCores) == 1) {
            proc->reqCores = numCores;
        } else {
            proc->reqCores = 1;
        }

        proc_to_rq_end(proc, &global_q);
    }

    global_t = proc_gettime();
    switch (policy) {
        case FCFS:
            fcfs();
            break;

        case RR:
            rr();
            break;

        default:
            err_exit("Unimplemented policy");
            break;
    }

    printf("WORKLOAD TIME: %.2lf secs\n", proc_gettime() - global_t);
    printf("scheduler exits\n");
    return 0;
}
void fcfs() {
    proc_t *proc;
    int status;
    proc_queue_init(&running_q);

    while (active_procs > 0 || !proc_queue_empty(&global_q)) {
        while (active_procs < numOfCpus && (proc = proc_rq_dequeue()) != NULL) {
            if (proc->status == PROC_NEW) {
                proc->t_start = proc_gettime();
                int pid = fork();
                if (pid == -1) {
                    err_exit("fork failed!");
                }
                if (pid == 0) {
                    printf("executing %s\n", proc->name);
                    execl(proc->name, proc->name, NULL);
                } else {
                    proc->pid = pid;
                    proc->status = PROC_RUNNING;
                    active_procs++;
                    proc_to_rq_end(proc, &running_q);
                    printf("process %d begins\n", proc->pid);
                }
            }
        }

        int finished_pid = waitpid(-1, &status, 0);
        if (finished_pid > 0) {
            active_procs--;

            proc_t *finished_proc = running_q.first;
            proc_t *prev_proc = NULL;

            while (finished_proc) {
                if (finished_proc->pid == finished_pid) {
                    finished_proc->status = PROC_EXITED;
                    finished_proc->t_end = proc_gettime();
                    printf("PID %d - CMD: %s\n", finished_proc->pid, finished_proc->name);
                    printf("\tElapsed time = %.2lf secs\n", finished_proc->t_end - finished_proc->t_submission);
                    printf("\tExecution time = %.2lf secs\n", finished_proc->t_end - finished_proc->t_start);
                    printf("\tWorkload time = %.2lf secs\n", finished_proc->t_end - global_t);

                    if (prev_proc) {
                        prev_proc->next = finished_proc->next;
                    } else {
                        running_q.first = finished_proc->next;
                    }

                    if (finished_proc == running_q.last) {
                        running_q.last = prev_proc;
                    }

                    free(finished_proc);
                    break;
                }
                prev_proc = finished_proc;
                finished_proc = finished_proc->next;
            }

            if (running_q.first == NULL) {
                running_q.last = NULL;
            }
        }
    }
}


void sigchld_handler(int signo, siginfo_t *info, void *context)
{
	printf("child %d exited\n", info->si_pid);
	if (running_proc == NULL) {
		printf("warning: running_proc==NULL\n");
	} else if (running_proc->pid == info->si_pid) {
		running_proc->status = PROC_EXITED;
		proc_t *proc = running_proc;
		proc->t_end = proc_gettime();
		printf("PID %d - CMD: %s\n", proc->pid, proc->name);
		printf("\tElapsed time = %.2lf secs\n", proc->t_end-proc->t_submission);
		printf("\tExecution time = %.2lf secs\n", proc->t_end-proc->t_start);
		printf("\tWorkload time = %.2lf secs\n", proc->t_end-global_t);

	} else {
		printf("warning: running %d exited %d\n", running_proc->pid, info->si_pid);
	}
}

void rr()
{
	struct sigaction sig_act;
	proc_t *proc;
	int pid;
	struct timespec req, rem;

	req.tv_sec = quantum / 1000;
	req.tv_nsec = (quantum % 1000)*1000000;

	printf("tv_sec = %ld\n", req.tv_sec);
	printf("tv_nsec = %ld\n", req.tv_nsec);

	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_handler = 0;
	sig_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	sig_act.sa_sigaction = sigchld_handler;
	sigaction (SIGCHLD,&sig_act,NULL);

	while ((proc=proc_rq_dequeue()) != NULL) {
		// printf("Dequeue process with name %s and pid %d\n", proc->name, proc->pid);
		if (proc->status == PROC_NEW) {
			proc->t_start = proc_gettime();
			pid = fork();
			if (pid == -1) {
				err_exit("fork failed!");
			}
			if (pid == 0) {
				printf("executing %s\n", proc->name);
				execl(proc->name, proc->name, NULL);
				printf("%s has exited\n", proc->name);
			}
			else {
				proc->pid = pid;
				running_proc = proc;
				proc->status = PROC_RUNNING;

				nanosleep(&req, &rem);
				if (proc->status == PROC_RUNNING) {
					kill(proc->pid, SIGSTOP);
					proc->status = PROC_STOPPED;
					printf("%s has stopped executing\n", proc->name);
					proc_to_rq_end(proc, &global_q);
				}
				else if (proc->status == PROC_EXITED) {
				}

			}
		}
		else if (proc->status == PROC_STOPPED) {
			proc->status = PROC_RUNNING;
			printf("%s has resumed executing\n", proc->name);
			running_proc = proc;
			kill(proc->pid, SIGCONT);

			nanosleep(&req, &rem);
			if (proc->status == PROC_RUNNING) {
				kill(proc->pid, SIGSTOP);
				proc_to_rq_end(proc,&global_q);
				proc->status = PROC_STOPPED;
				printf("%s has stopped executing\n", proc->name);
			}
			else if (proc->status == PROC_EXITED) {
			}

		}
		else if (proc->status == PROC_EXITED) {
			printf("process has exited\n");
		}
		else if (proc->status == PROC_RUNNING) {
			printf("WARNING: Already running process\n");
		}
		else {
			err_exit("Unknown process status");
		}
	}
}
