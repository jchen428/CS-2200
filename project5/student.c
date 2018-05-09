/*
 * student.c
 * Multithreaded OS Simulation for CS 2200, Project 5
 * Spring 2016
 *
 * This file contains the CPU scheduler for the simulation.
 * Name:
 * GTID:
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os-sim.h"

typedef struct node {
  struct node* next;
  pcb_t* proc;
} node;

typedef struct readyQ {
    node* head;
    node* tail;
    int size;
    pthread_mutex_t ready_mutex;
} readyQ;

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;
static pthread_cond_t wake_cond;
readyQ* ready;
pcb_t* head;
pcb_t* tail;
int cpu_count;
int preemptive;
int quantum;
int sp;

void enqueue(pcb_t* proc) {
    node* n;

    n = malloc(sizeof(node));
    n->next = NULL;
    n->proc = proc;

    pthread_mutex_lock(&ready->ready_mutex);

    if (!ready->head) {
        ready->head = n;
        ready->tail = n;
    } else {
        ready->tail->next = n;
        ready->tail = n;
    }
    ready->size++;
    pthread_cond_broadcast(&wake_cond);

    pthread_mutex_unlock(&ready->ready_mutex);
}

pcb_t* dequeue() {
    if (ready->size <= 0)
        return NULL;

    pthread_mutex_lock(&ready->ready_mutex);

    node* n = ready->head;
    pcb_t* proc = n->proc;
    if (!n->next) {
        ready->head = NULL;
        ready->tail = NULL;
    } else {
        ready->head = n->next;
        if (ready->size == 2)
            ready->tail = n->next;
    }
    ready->size--;

    pthread_mutex_unlock(&ready->ready_mutex);

    free(n);

    return proc;
}

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running
 *	process indexed by the cpu id. See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t* next = dequeue(); 
    if (!next) {
        context_switch(cpu_id, NULL, quantum);
        return;
    }

    next->state = PROCESS_RUNNING;

    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = next;
    pthread_mutex_unlock(&current_mutex);

    context_switch(cpu_id, next, quantum);
}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    pthread_mutex_lock(&ready->ready_mutex);
    while (ready->size <= 0)
        pthread_cond_wait(&wake_cond, &ready->ready_mutex);
    pthread_mutex_unlock(&ready->ready_mutex);

    schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    //mt_safe_usleep(1000000);
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_READY;
    enqueue(current[cpu_id]);
    pthread_mutex_unlock(&current_mutex);

    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);

    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);

    schedule(cpu_id);
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is static priority, wake_up() may need
 *      to preempt the CPU with the lowest priority process to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a higher priority than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    process->state = PROCESS_READY;
    enqueue(process);

    if (sp) {
        int minPriority = 11;
        int minIndex = -1;

        pthread_mutex_lock(&current_mutex);
        for (int i = 0; i < cpu_count; i++) {
            if (!current[i]) {
                pthread_mutex_unlock(&current_mutex);
                return;
            } else if (current[i]->static_priority < minPriority) {
                minPriority = current[i]->static_priority;
                minIndex = i;
            }
        }
        pthread_mutex_unlock(&current_mutex);

        if (minPriority > process->static_priority)
            return;

        force_preempt(minIndex);
    }
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -p command-line parameters.
 */
int main(int argc, char *argv[])
{
    //int cpu_count;

    /* Parse command-line arguments */
    /*if (argc != 2)
    {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -p : Static Priority Scheduler\n\n");
        return -1;
    }
    cpu_count = atoi(argv[1]);*/

    /* FIX ME - Add support for -r and -p parameters*/
    if (argc == 2) {
        preemptive = 0;
        quantum = -1;
        sp = 0;
    } else if (argc == 3 && strcmp(argv[2], "-p") == 0) {
        preemptive = 1;
        quantum = -1;
        sp = 1;
    } else if (argc == 4 && strcmp(argv[2], "-r") == 0) {
        preemptive = 1;
        quantum = atoi(argv[3]);
        sp = 0;
    } else {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -p : Static Priority Scheduler\n\n");
        return -1;
    }
    cpu_count = atoi(argv[1]);

    ready = malloc(sizeof(readyQ));
    ready->head = NULL;
    ready->tail = NULL;
    ready->size = 0;
    pthread_mutex_init(&ready->ready_mutex, NULL);
    pthread_cond_init(&wake_cond, NULL);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}


