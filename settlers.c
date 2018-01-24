
#include <stdlib.h>
#include <stdio.h>
#include "threads.h"

struct sem_t kitchen, pantry; 
int food, meat;
const int night_length = 5000;
const int days = 365;

int people_left;

void eat() {
	sem_wait(&kitchen);
	if(food) {
		food--;
		sem_signal(&kitchen);
	}
	else {
		sem_signal(&kitchen);
		people_left--;
		thread_exit();
	}
}

void cook(void *name) {
	for(int i=0; i<days; i++) {
		sem_wait(&pantry);
		if(meat) {
			sem_wait(&kitchen);
			food += rand()%6 + 1;
			sem_signal(&kitchen);
			meat--;
		}
		sem_signal(&pantry);
		eat();
		thread_sleep(night_length);
	}
	thread_exit();
}

void hunt(void *name) {
	for(int i=0; i<days; i++) {
		if(rand()%6 > rand()%6) {		
			sem_wait(&pantry);
			meat++;
			sem_signal(&pantry);
		}
		eat();
		thread_sleep(night_length);
	}
	thread_exit();
}

int main(int argc, char *argv[]) {
	if(argc != 5) {
		print("Niepoprawne parametry programu\n");
		return 0;
	}
	threads_init();
	sem_init(&pantry, 1);
	sem_init(&kitchen, 1);
	srand(time(0));
	
	int no_hunters = atoi(argv[1]), no_cooks = atoi(argv[2]);
	people_left = no_hunters + no_cooks;
	
	meat = atoi(argv[3]);
	food = atoi(argv[4]);
	
	int hunters[no_hunters], cooks[no_cooks];
	
	for(int i=0; i<no_hunters; i++){
		hunters[i] = thread_create(hunt, NULL);
	}
	for(int i=0; i<no_cooks; i++){
		cooks[i] = thread_create(cook, NULL);
	}
	
	for(int i=0; i<no_hunters; i++) {
		thread_join(hunters[i]);
	}
	for(int i=0; i<no_cooks; i++) {
		thread_join(cooks[i]);
	}
	char s[50];
	sprintf(s,"Na koniec: %d miesa, %d jedzenia, %d ludzi\n\n", meat, food, people_left);
	print(s);
	return 0;
}
