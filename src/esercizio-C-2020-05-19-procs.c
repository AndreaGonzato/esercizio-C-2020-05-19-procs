#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include <pthread.h>
#include <errno.h>



void * thread_start(int thread_index);


#define N 10

int countdown;
int shutdown_i;
int * process_counter;


sem_t semaphore;

int main(void) {

	countdown = -1;
	shutdown_i = 0;

	process_counter = calloc(N, sizeof(int));
	if(process_counter == NULL){
		perror("calloc()");
		exit(1);
	}

	int res = sem_init(&semaphore,
			0, // 1 => il semaforo è condiviso tra processi, 0 => il semaforo è condiviso tra threads del processo
			1 // valore iniziale del semaforo
		  );
	if(res == -1){
		perror("sem_init()");
		exit(1);
	}

	pthread_t * thread_id = calloc(N , sizeof(pthread_t));
	if(thread_id == NULL){
		perror("calloc()");
		exit(1);
	}

	// create N thread
	for (int i = 0; i < N; i++) {
	    int res = pthread_create(&thread_id[i], NULL,
	                       &thread_start, i);
	    if (res != 0){
			perror("pthread_create()");
			exit(1);
	    }
	}

	// dopo avere avviato i thread, il thread main dorme 1 secondo
	int sec = 1;
	printf("wait %d second\n", sec);
	sleep(sec);
	countdown = 100000;
	printf("countdown set to: %d\n", countdown);

	while(1){
		if( countdown == 0){
			shutdown_i = 1;
			break;
		}
	}


	// wait N threads
	for(int i=0 ; i<N ; i++){
		int res = pthread_join(thread_id[i], NULL);
	    if (res != 0){
			perror("pthread_join()");
			exit(1);
	    }
	}

	printf("countdown at the end: %d\n", countdown);

	for(int i=0 ; i<N ; i++){
	  printf("thread %d has decrement countdown %d times\n", i, process_counter[i]);
	}

	printf("end\n");

	return 0;
}

void * thread_start(int thread_index) {

	while(1){
		if (sem_wait(&semaphore) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}



		// sezione critica
		if( countdown > 0 ){

			countdown--;
			process_counter[thread_index]++;

		}


		if (sem_post(&semaphore) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}


		if( shutdown_i != 0){
			return NULL;
		}
	}
	return NULL;
}
