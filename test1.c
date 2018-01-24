#include "threads.h"

void f1(void* arg)
{
	print("f1\n");
	int x = (long)arg;
	for (int i = 0; i < 100; i++)
	{
		char s[20];
		thread_sleep(100000);
		sprintf(s, "\tThread 1: %d %d\n", i, x);
		print(s);
	}
	thread_exit();
}

void f2()
{
	print("f2\n");
	for (int i = 0; i < 15; i++)
	{
		thread_sleep(1000000);
		char s[20];
		sprintf(s, "\tThread 2: %d\n", i);
		print(s);
	}
	thread_exit();
}

int main()
{
    threads_init();
    thread_create(f1, (void*)56);
    print("Created 1\n");
    thread_create(f2, NULL);
    print("Created 2\n");
    
    thread_join(2);
    print("\tjoin 2\n");
    thread_join(1);
    print("\tjoin 1\n");
    return 0;
}

