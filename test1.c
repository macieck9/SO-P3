#include "threads.h"

void f1(void* arg)
{
	printf("f1\n");
	//sem_wait(&sem);
	int x = (long)arg;
	for (int i = 0; i < 100; i++)
	{
		//if(i%1000 == 0)
		char s[20];
		thread_sleep(100000);
		sprintf(s, "\tThread 1: %d %d\n", i, x);
		print(s);
	}
	//sem_signal(&sem);
	//thread_join(2);
	//printf("Exiting 1\n");
	thread_exit();
}

void f2()
{
	printf("f2\n");
    //sem_wait(&sem);
	for (int i = 0; i < 15; i++)
	{
		//if(i%1000 == 0)
		thread_sleep(1000000);
		char s[20];
		sprintf(s, "\tThread 2: %d\n", i);
		print(s);
	}
	//sem_signal(&sem);
	thread_exit();
}

int main()
{
    init();
    //sem.cnt = 1;
    thread_create(f1, (void*)56);
    puts("Created 1");
    thread_create(f2, NULL);
    puts("Created 2");
    for (int i = 0; i < 1000000; i++)
    {
    	//if(i%1000 == 0)
    	//	printf("\tMain: %d\n", i);
    }
    
    thread_join(2);
    puts("\tjoin 2");
    thread_join(1);
    puts("\tjoin 1");
	//printf("%lld\n", sum);
    return 0;
}

