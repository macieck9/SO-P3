#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<signal.h>

#define max_cnt 2
int cnt = max_cnt;
int N = 0;
int next_id = 0;

struct sem_t
{
    int cnt;
};

struct sem_t sem;

struct thread_t
{
    ucontext_t context;
    int id;
    int wait;
    struct sem_t* sem;
};


struct thread_list
{
	struct thread_t thread;
	struct thread_list *next;
	struct thread_list *prev;
};

struct thread_list* threads;

void schedule()
{
	cnt--;
	struct thread_list* curr = threads;
	if (cnt == 0)
	{
	    cnt = max_cnt;
	    threads = threads->next;
	}
	while(threads->thread.wait != -1 || threads->thread.sem != 0)
	{
	    cnt = max_cnt;
	    threads = threads->next;
	}
    swapcontext(&curr->thread.context, &threads->thread.context);
}

int thread_create(void (*func)())
{
    ucontext_t cont;
    getcontext(&cont);
    cont.uc_link = 0;
    cont.uc_stack.ss_sp = malloc(SIGSTKSZ);
    cont.uc_stack.ss_size=SIGSTKSZ;
    cont.uc_stack.ss_flags=0;
    makecontext(&cont, func, 0);
    struct thread_t *thread = malloc(sizeof(struct thread_t));
    thread->context = cont;
    thread->id = next_id++;
    thread->wait = -1;
    thread->sem = 0;
    struct thread_list *new_thread = malloc(sizeof(struct thread_list));
    new_thread->thread = *thread;
    new_thread->next = threads->next;
    new_thread->prev = threads;
    new_thread->next->prev = new_thread;
    new_thread->prev->next = new_thread;
    //printf("%d\n", thread.id);
    return thread->id;
}

void thread_exit()
{
	//printf("Exit %d\n", threads->thread.id);
	int curr_id = threads->thread.id;
	struct thread_list* tmp_thread = threads;
	
	do
	{
	    if (tmp_thread->thread.wait == curr_id)
	        tmp_thread->thread.wait = -1;
	    tmp_thread = tmp_thread->next;
	} while(tmp_thread->thread.id != curr_id);
	
	threads->prev->next = threads->next;
	threads->next->prev = threads->prev;
	threads = threads->next;
	//printf("Exit2 %d\n", threads->thread.id);
	setcontext(&threads->thread.context);
}

void thread_join(int id)
{
    int curr_id = threads->thread.id;
    if (curr_id == id)
        return;
	struct thread_list* tmp_thread = threads;
	int exists = 0;
	do
	{
	    if (tmp_thread->thread.id == id)
	        exists = 1;
	    tmp_thread = tmp_thread->next;
	} while(tmp_thread->thread.id != curr_id);
	
	if (exists)
	{
        threads->thread.wait = id;
        schedule();
    }
}

void sem_wait(struct sem_t* sem)
{
    sem->cnt--;
    if (sem->cnt < 0)
    {
        threads->thread.sem = sem;
        schedule();
    }
}

void sem_signal(struct sem_t* sem)
{
    sem->cnt++;
    if (sem->cnt <= 0)
    {
	    struct thread_list* tmp_thread = threads;
	    int curr_id = threads->thread.id;
	    do
	    {
	        if (tmp_thread->thread.sem == sem)
	        {
	            tmp_thread->thread.sem = 0;
	            break;
	        }
	        tmp_thread = tmp_thread->next;
	    } while(tmp_thread->thread.id != curr_id);
    }
}


void init()
{
	threads = malloc(sizeof(struct thread_list));
	threads->thread.id = next_id++;
	threads->thread.wait = -1;
	threads->thread.sem = 0;
	threads->next = threads->prev = threads;
	getcontext(&threads->thread.context);
}

void f1()
{
    sem_wait(&sem);
	for (int i = 0; i < 7; i++)
	{
		printf("Thread 1: %d\n", i);
		schedule();
	}
	sem_signal(&sem);
	thread_exit();
}

void f2()
{
    sem_wait(&sem);
	for (int i = 0; i < 5; i++)
	{
		printf("Thread 2: %d\n", i);
		schedule();
	}
	sem_signal(&sem);
	thread_exit();
}

int main()
{
    init();
    sem.cnt = 1;
    thread_create(f1);
    puts("Created 1");
    schedule();
    thread_create(f2);
    puts("Created 2");
    schedule();
    
    thread_join(2);
    puts("T");
    schedule();
    thread_join(1);
    puts("N");
    
    /*
    for (int i = 0; i < 5; i++)
    {
    	puts("Main");
    	schedule();
    }
    */
    return 0;
}



