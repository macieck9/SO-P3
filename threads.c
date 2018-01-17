#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<signal.h>
#include<unistd.h>
#include<sys/time.h>


#define max_cnt 2
int cnt = max_cnt;
int N = 0;
int next_id = 0;

// UWAGA: korzystanie z wirtualnego timera sprawia, że nie można korzystać
// ze sleep() czy usleep(); trzeba by podpiąć się w tym celu pod inny timer,
// wysyłający inny sygnał (może być ważne przy testowaniu)

// korzystamy z timerów, a nie ualarm, bo wg mana to jest bezpieczniejsze.

const struct itimerval timer_interv = {{0,0}, {0,1}};
const struct itimerval timer_zero = {{0,0}, {0,0}};

void timer_off() {
	setitimer(ITIMER_VIRTUAL, &timer_zero, NULL);
}

void timer_on() {
	setitimer(ITIMER_VIRTUAL, &timer_interv, NULL);
}

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
ucontext_t main_cont;

void schedule()
{
	//cnt--;
	timer_off();
	printf("Schedule: %d : timer off\n", threads->thread.id);
	struct thread_list* curr = threads;
	threads = threads->next;
	while(threads->thread.wait != -1 || threads->thread.sem != 0)
	{
	    threads = threads->next;
	}
	//timer_on();
	//printf("%lu\n", (unsigned long int)(&threads->thread.context));
	printf("Schedule: %d : timer on\n", threads->thread.id);
	printf("Schedule: %d : swapping context\n", threads->thread.id);
    swapcontext(&curr->thread.context, &threads->thread.context);
    /*
    dopóki kontekst zmienia się tym swapem, ustawianie timera po nim jest sensowne,
    jednak czasem zmienia się go na świeżutki z thread_create. Co właściwie się
    wtedy dzieje? Wtedy wywołuje się fcja wskazana w makecontext. Więc nie
    włącza się timer?
    */
	timer_on();
	
}

void aaa()
{
	/// ??? jeśli ani razu nie wywoła się aaa, jest fpe przy exicie.
	timer_off();
	printf("AAA\n");
	schedule();
}

int thread_create(void (*func)())
{
	timer_off();
	printf("Thread_create: %d : timer off\n", threads->thread.id);
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
    printf("Thread_create: %d : timer on\n", threads->thread.id);
    timer_on();
    //printf("%d\n", thread.id);
    //printf("%ld\n", *((long*)(&threads->thread.context)));
    return thread->id;
}

void thread_exit()
{
	//printf("Exit %d\n", threads->thread.id);
	timer_off();
	printf("Thread_exit: %d : timer off\n", threads->thread.id);
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
	printf("Thread_exit: %d : setting context\n", threads->thread.id);
	struct thread_list* tmp = threads;
	do
	{
		printf("%d\n", tmp->thread.id);
		tmp = tmp->next;
	}
	while (tmp->thread.id != threads->thread.id);
	timer_on();
	setcontext(&threads->thread.context);
	//printf("Exit2 %d\n", threads->thread.id);
	//printf("Thread_exit: %d : schedule\n", threads->thread.id);
}

void thread_join(int id)
{
	timer_off();
	printf("Thread_join: %d : timer off\n", threads->thread.id);
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
        printf("Thread_join: %d : schedule\n", threads->thread.id);
        schedule();
    }
    else
    {
		printf("Thread_join: %d : timer on\n", threads->thread.id);
		timer_on();
	}
}

void sem_wait(struct sem_t* sem)
{
	timer_off();
	printf("Sem_wait: %d : timer off\n", threads->thread.id);
    sem->cnt--;
    if (sem->cnt < 0)
    {
        threads->thread.sem = sem;
        printf("Sem_wait: %d : schedule\n", threads->thread.id);
        schedule();
    }
    else
    {
    	printf("Sem_wait: %d : timer on\n", threads->thread.id);
    	timer_on();
    	
    }
}

void sem_signal(struct sem_t* sem)
{
	timer_off();
	printf("Sem_signal: %d : timer_off\n", threads->thread.id);
    sem->cnt++;
    if (sem->cnt >= 0)
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
    printf("Sem_signal: %d : timer_on\n", threads->thread.id);
    timer_on();
    // a może wznowić odliczanie od miejsca, w którym było przy wejściu do fcji?
}


void init()
{
	threads = malloc(sizeof(struct thread_list));
	threads->thread.id = next_id++;
	threads->thread.wait = -1;
	threads->thread.sem = 0;
	threads->next = threads->prev = threads;
	//printf("Init\n");
	signal(SIGVTALRM, aaa);
	//ucontext_t cont;
    getcontext(&main_cont);
    main_cont.uc_link = 0;
	main_cont.uc_stack.ss_sp = malloc(SIGSTKSZ);
	main_cont.uc_stack.ss_size=SIGSTKSZ;
	main_cont.uc_stack.ss_flags=0;
    //cont.uc_link = 0;
    //cont.uc_stack.ss_sp = malloc(SIGSTKSZ);
    //cont.uc_stack.ss_size=SIGSTKSZ;
    //cont.uc_stack.ss_flags=0;
	threads->thread.context=main_cont;
	printf("Init: %d : timer on\n", threads->thread.id);
	timer_on();
}

void f1()
{
	printf("f1\n");
	for (int i = 0; i < 1000000; i++)
	{
		//sem_wait(&sem);
		//if(i%1000 == 0)
		fprintf(stderr, "\tThread 1: %d\n", i);
		//sem_signal(&sem);
	}
	//sem_signal(&sem);
	thread_join(2);
	printf("Exiting 1\n");
	thread_exit();
}

void f2()
{
	printf("f2\n");
    //sem_wait(&sem);
	for (int i = 0; i < 1000000; i++)
	{
		//sem_wait(&sem);
		//if(i%1000 == 0)
		fprintf(stderr, "\tThread 2: %d\n", i);
		//sem_signal(&sem);
		//schedule();
	}
	//sem_signal(&sem);
	//thread_join(1);
	thread_exit();
}

int main()
{
    init();
    sem.cnt = 1;
    thread_create(f1);
    puts("Created 1");
    //schedule();
    thread_create(f2);
    puts("Created 2");
    //schedule();
    for (int i = 0; i < 1000000; i++)
    {
    	//if(i%1000 == 0)
    	//	printf("\tMain: %d\n", i);
    	//sem_signal(&sem);
    }
    
    thread_join(2);
    puts("\tjoin 2");
    //schedule();
    thread_join(1);
    puts("\tjoin 1");
    
    printf("\t\tHALOOOO\n");
    struct thread_list* beg = threads;
    struct thread_list* end = threads;
    int len = 0;
    while(beg != end->next)
    {
    	len++;
    	end = end->next;
    }
    printf("\t\t%d\n", len);
    	
    return 0;
}



