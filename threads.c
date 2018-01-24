#include "threads.h"

int next_id = 0;

struct itimerval timer_interv = {{0,0}, {0,interv}};
struct itimerval timer_zero = {{0,0}, {0,0}};
struct itimerval timer_temp;
struct timespec clock_val;

void timer_off()
{
	setitimer(ITIMER_REAL, &timer_zero, &timer_temp);
}

void timer_on()
{
	setitimer(ITIMER_REAL, &timer_interv, NULL);
}

void timer_resume()
{
	setitimer(ITIMER_REAL, &timer_temp, NULL);
}

void handle_sleeping()
{
	struct timespec tmp_time;
	clock_gettime(CLOCK_REALTIME, &tmp_time);
	long long delta = (tmp_time.tv_sec - clock_val.tv_sec)*1000000000LL + (tmp_time.tv_nsec - clock_val.tv_nsec);
	clock_val.tv_sec = tmp_time.tv_sec;
	clock_val.tv_nsec = tmp_time.tv_nsec;
	//sum += delta;
	int curr_id = threads->thread.id;
	struct thread_list* tmp_thread = threads;
	do
	{
	    if (tmp_thread->thread.sleep)
	        tmp_thread->thread.sleep = delta > tmp_thread->thread.sleep ? 0 : tmp_thread->thread.sleep - delta;
	    //printf("%d\n", tmp_thread->thread.sleep);
	    tmp_thread = tmp_thread->next;
	} while(tmp_thread->thread.id != curr_id);
}

void schedule()
{
	timer_off();
	handle_sleeping();
	//printf("Schedule: %d : timer off\n", threads->thread.id);
	struct thread_list* curr = threads;
	threads = threads->next;
	while(threads->thread.wait != -1 || threads->thread.sem || threads->thread.sleep)
	{
	    threads = threads->next;
	    handle_sleeping();
	}
	//printf("%lu\n", (unsigned long int)(&threads->thread.context));
	//printf("Schedule: %d : timer on\n", threads->thread.id);
	//printf("Schedule: %d %d: swapping context\n", threads->thread.id, curr->thread.id);
	//printf("%ld\n", *(long*)(&threads->thread.context));
	timer_on();
    swapcontext(&curr->thread.context, &threads->thread.context);	
}

void thread_sleep(int microsec)
{
	timer_off();
	threads->thread.sleep = 1000LL*microsec;
	schedule();
}

void print(char *s)
{
	timer_off();
	printf("%s", s);
	timer_resume();
}

int thread_create(void (*func)(), void *args)
{
	timer_off();
	//printf("Thread_create: %d : timer off\n", threads->thread.id);
    ucontext_t *cont = malloc(sizeof(ucontext_t));
    getcontext(cont);
    cont->uc_link = 0;
    cont->uc_stack.ss_sp = malloc(SIGSTKSZ);
    cont->uc_stack.ss_size=SIGSTKSZ;
    cont->uc_stack.ss_flags=0;
    makecontext(cont, func, 1, args);
    struct thread_t *thread = malloc(sizeof(struct thread_t));
    thread->context = *cont;
    thread->id = next_id++;
    thread->wait = -1;
    thread->sem = 0;
    struct thread_list *new_thread = malloc(sizeof(struct thread_list));
    new_thread->thread = *thread;
    new_thread->next = threads->next;
    new_thread->prev = threads;
    new_thread->next->prev = new_thread;
    new_thread->prev->next = new_thread;
    //printf("Thread_create: %d : timer on\n", threads->thread.id);
    timer_on();
    return thread->id;
}

void thread_exit()
{
	timer_off();
	//printf("Thread_exit: %d : timer off\n", threads->thread.id);
	int curr_id = threads->thread.id;
	struct thread_list* tmp_thread = threads;
	do
	{
	    if (tmp_thread->thread.wait == curr_id)
	    {
	    	//printf("Uwalniam %d\n", tmp_thread->thread.id);
	        tmp_thread->thread.wait = -1;
	    }
	    tmp_thread = tmp_thread->next;
	} while(tmp_thread->thread.id != curr_id);
	
	threads->prev->next = threads->next;
	threads->next->prev = threads->prev;
	threads = threads->next;
	while(threads->thread.wait != -1 || threads->thread.sem || threads->thread.sleep)
	{
	    threads = threads->next;
	    handle_sleeping();
	}
	//printf("Thread_exit: %d : setting context\n", threads->thread.id);
	timer_on();
    setcontext(&threads->thread.context);
}

void thread_join(int id)
{
	timer_off();
	//printf("Thread_join: %d : timer off\n", threads->thread.id);
    int curr_id = threads->thread.id;
    if (curr_id == id)
        return;
	struct thread_list* tmp_thread = threads;
	int exists = 0;
	//printf("Istniejace:\n");
	do
	{
		//printf("%d %d, ", tmp_thread->thread.id, tmp_thread->thread.wait);
	    if (tmp_thread->thread.id == id)
	        exists = 1;
	    tmp_thread = tmp_thread->next;
	} while(tmp_thread->thread.id != curr_id);
	//printf(" - i to tyle\n");
	if (exists)
	{
        threads->thread.wait = id;
        //printf("Thread_join: %d : schedule\n", threads->thread.id);
        schedule();
    }
    else
    {
		//printf("Thread_join: %d : timer on\n", threads->thread.id);
		timer_on();
	}
}

void sem_init(struct sem_t* sem, int val)
{
	sem->cnt = val;
	sem->min = sem->max = 0;
}

void sem_wait(struct sem_t* sem)
{
	timer_off();
	//printf("Sem_wait: %d : timer off\n", threads->thread.id);
    sem->cnt--;
    if (sem->cnt < 0)
    {
        threads->thread.sem = sem;
        threads->thread.sem_num = sem->max;
        sem->max++;
        //printf("Sem_wait: %d : schedule\n", threads->thread.id);
        schedule();
    }
    else
    {
    	//printf("Sem_wait: %d : timer on\n", threads->thread.id);
    	timer_on();
    	
    }
}

void sem_signal(struct sem_t* sem)
{
	timer_off();
	//printf("Sem_signal: %d : timer_off\n", threads->thread.id);
    sem->cnt++;
    if (sem->cnt <= 0)
    {
	    struct thread_list* tmp_thread = threads;
	    int curr_id = threads->thread.id;
	    do
	    {
	        if (tmp_thread->thread.sem == sem && tmp_thread->thread.sem_num == sem->min)
	        {
	            tmp_thread->thread.sem = 0;
	            //printf("Signal: %d %d\n", tmp_thread->thread.id, tmp_thread->thread.sem_num);
	            sem->min++;
	            break;
	        }
	        //printf("Signal2: %d %d %d\n", tmp_thread->thread.id, tmp_thread->thread.sem_num, sem->min);
	        tmp_thread = tmp_thread->next;
	    } while(tmp_thread->thread.id != curr_id);
    }
    //printf("Sem_signal: %d : timer_on\n", threads->thread.id);
    timer_resume();
}

void threads_init()
{
	threads = malloc(sizeof(struct thread_list));
	threads->thread.id = next_id++;
	threads->thread.wait = -1;
	threads->thread.sem = 0;
	threads->thread.sem_num = 0;
	threads->next = threads->prev = threads;
	signal(SIGALRM, schedule);
	ucontext_t *main_cont = malloc(sizeof(ucontext_t));
    getcontext(main_cont);
    main_cont->uc_link = 0;
	main_cont->uc_stack.ss_sp = malloc(SIGSTKSZ);
	main_cont->uc_stack.ss_size=SIGSTKSZ;
	main_cont->uc_stack.ss_flags=0;
	threads->thread.context=*main_cont;
	//printf("Init: %d : timer on\n", threads->thread.id);
	clock_gettime(CLOCK_REALTIME, &clock_val);
	timer_on();
}

