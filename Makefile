CC = gcc
F = -Wall -Werror -std=gnu99

threads.o: threads.c threads.h Makefile
	$(CC) $(F) threads.c -o threads.o -c 
	
test1.o: test1.c Makefile
	$(CC) $(F) test1.c -o test1.o -c 
	
test1: test1.o threads.o threads.h Makefile
	$(CC) $(F) test1.o threads.o -o test1 
