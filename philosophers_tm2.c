#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


#define N	5
#define LEFT	( i + N - 1 ) % N
#define RIGHT	( i + 1 ) % N

#define THINKING 0
#define HUNGRY 1
#define EATING 2


pthread_mutex_t m ;	            // mutex
pthread_mutex_t s[N];	        //	array of mutexes
pthread_mutex_t nb_meals_mutex; // mutex for fair eating

int	state[N];	// thinking/hungry/eating
int p_meals[N]; // array for nbs of meals of each philosopher
int avg;        // variable for storing avarage nb of meals

void test(int i) {      // function for unlocking the mutex if this philosopher can eat
    if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        pthread_mutex_unlock(&s[i]);
    }
}

void grab_forks( int p_id ) 
{
    pthread_mutex_lock(&m); // wait as long as other philosopher is grabbing or putting away forks
    state[p_id] = HUNGRY;
    test(p_id);
    pthread_mutex_unlock(&nb_meals_mutex); 
    pthread_mutex_unlock(&m);
    pthread_mutex_lock(&s[p_id]);
}

void put_away_forks( int p_id)
{
    pthread_mutex_lock(&m); // wait as long as other philosopher is grabbing or putting away forks
    state[p_id] = THINKING;
    int i = p_id;
    test(LEFT);
    test(RIGHT);
    pthread_mutex_unlock(&m);
}

void  sigint_handler(int sig){ // handler for displaying summary after ctrl+c
   // usleep(100000);
    printf("\nSigint. Exiting...\n");
    pthread_mutex_destroy(&m);
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&s[i]);
        printf("Philosopher %d ate %d meals.\n", i, p_meals[i]);
    }
    exit(0);
}

void* philosopher(void* philosopher_id){    // philosophers processes
    int i = *(int*)philosopher_id;

    while (1){
        printf("Philosopher %d is thinking.\n", i);
        usleep(1000);

        grab_forks(i); // left fork has id the same as a philosopher
        printf("Philosopher %d is eating.\n", i);

        pthread_mutex_lock(&nb_meals_mutex); // wait if other philosopher is here, lock if not
        p_meals[i]++; // increament nb of this philosophers meals

        int total_nb_meals = 0, j;
        for (j = 0; j < N; j++){    // calculate total numer of meals eaten
            total_nb_meals = total_nb_meals + p_meals[j];
        }

        int temp = p_meals[i] - (total_nb_meals / N); // calculate the difference between current philosophers meals and an avare nb of meals

        if (temp > 1){ //   if the difeerence is more than 1
            avg = p_meals[i]; // this philosophers nb of meals becomes global avg
            pthread_mutex_unlock(&nb_meals_mutex);
            printf("Philosopher %d ate. Meal number %d\n", i, p_meals[i]);
            usleep(1000);
        }

        else{
            pthread_mutex_unlock(&nb_meals_mutex);
            printf("Philosopher %d ate. Meal number %d\n", i, p_meals[i]);
        }
        put_away_forks(i);

        pthread_mutex_lock(&nb_meals_mutex);
        if (p_meals[i] >= avg){
            pthread_mutex_unlock(&nb_meals_mutex);
        }
    }
}

int main(){
    signal(SIGINT, sigint_handler);
    pthread_t philosophers[N]; 
    int i;
    pthread_mutex_init (&m, NULL); // initiate mutex m
    for (i = 0; i < N; i++)  {
        pthread_mutex_init(&s[i], NULL);    // initiate mutex s[]
        pthread_mutex_lock(&s[i]);          // lock it
        state[i] = THINKING;
        p_meals[i] = 0;                     // initial number of meals eaten
    }
    avg = 0;

    pthread_mutex_init(&nb_meals_mutex, NULL);  // initialize mutex for fair eating

    for (i = 0; i < N; i++) {
        int *arg = malloc(sizeof(*arg)); // allocating memory for i
        *arg = i;                          // putting ther i
        pthread_create(&philosophers[i], NULL, philosopher, arg);
    }
    int j;  


    for (i = 0; i < N; i++)
    {
        pthread_join(philosophers[i], NULL);
    }

    return 0;
}