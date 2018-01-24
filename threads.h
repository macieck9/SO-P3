#ifndef _THREADS123
#define _THREADS123 1

#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<signal.h>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>

#define interv 100



void timer_off();
void timer_on();
void timer_resume();

struct sem_t
{
    int cnt;
};

struct thread_t
{
    ucontext_t context;
    int id;
    int wait;
    long long sleep;
    struct sem_t* sem;
};

struct thread_list
{
	struct thread_t thread;
	struct thread_list *next;
	struct thread_list *prev;
};

struct thread_list* threads;

void schedule();
void handle_sleeping();

void threads_init();
int thread_create(void (*func)(), void*);
void thread_exit();
void thread_join(int);
void thread_sleep(int);

void print(char*);

void sem_init(struct sem_t*, int);
void sem_wait(struct sem_t*);
void sem_signal(struct sem_t*);


#endif
