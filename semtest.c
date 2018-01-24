#include "threads.h"

struct sem_t sem1, sem2;

void producer(void* arg)
{
	int id = (long) arg;
	while(1)
	{
    	thread_sleep(1000000);
    	if (id == 1)
    		sem_signal(&sem1);
    	else
    		sem_signal(&sem2);
    }
}

void consumer(void* arg)
{
    int id = (long) arg;
	while(1)
	{
	    sem_wait(&sem1);
	    char s[50];
	    sprintf(s, "Consumer %d consumes sem1.\n", id);
	    print(s);
	    thread_sleep(1000000);
	    sem_wait(&sem2);
	    sprintf(s, "Consumer %d consumes sem2.\n", id);
	    print(s);
	}
}

int main()
{
    threads_init();
    sem_init(&sem1, 0);
    sem_init(&sem2, 0);
    
    thread_create(producer, (void*)1);
    thread_create(producer, (void*)2);
    
    thread_create(consumer, (void*)1);
    thread_create(consumer, (void*)2);
    thread_create(consumer, (void*)3);
    
    thread_join(1);
    
    
    return 0;
}
