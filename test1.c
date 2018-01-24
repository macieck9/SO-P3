#include "threads.h"

void f1(void* arg)
{
	print("f1\n");
	char* x = arg;
	for (int i = 0; i < 50; i++)
	{
		char s[20];
		thread_sleep(100000);
		sprintf(s, "%s: %d\n",x, i);
		print(s);
	}
	thread_exit();
}

void f2()
{
	print("f2\n");
	for (int i = 0; i < 10; i++)
	{
		thread_sleep(1000000);
		char s[20];
		sprintf(s, "Thread 2: %d\n", i);
		print(s);
	}
	thread_exit();
}

int main()
{
    threads_init();
    char* s = "Thread 1";
    thread_create(f1, (void*)s);
    print("Created 1\n");
    thread_create(f2, NULL);
    print("Created 2\n");
    
    thread_join(2);
    print("\tjoin 2\n");
    thread_join(1);
    print("\tjoin 1\n");
    return 0;
}

