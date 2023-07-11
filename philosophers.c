#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#define N 7
#define THINKING_TIME_RANGE 3
#define EATING_TIME_RANGE 2

int eaten_id;
int sem_set_id;

void grab_forks( int left_fork_id ){
    struct sembuf sem_op[2]; // create an array for two semaphore operations

    // parameters for grabing the left fork
    sem_op[0].sem_num = left_fork_id;
    sem_op[0].sem_op = -1; 
    sem_op[0].sem_flg = 0;

    // parameters for grabing the right fork
    sem_op[1].sem_num = ( left_fork_id + 1 ) % N;
    sem_op[1].sem_op = -1; 
    sem_op[1].sem_flg = 0;

    semop(sem_set_id, sem_op, 2); // performing this operation simultaneusly on two semaphors (grabing two forks)
};

void put_away_forks( int left_fork_id ){
    struct sembuf sem_op[2]; // create an array for two semaphore operations

    // parameters for putting away the left fork
    sem_op[0].sem_num = left_fork_id;
    sem_op[0].sem_op = 1;   
    sem_op[0].sem_flg = 0;

    // parameters for putting away the right fork
    sem_op[1].sem_num = ( left_fork_id + 1 ) % N;
    sem_op[1].sem_op = 1; 
    sem_op[1].sem_flg = 0;

    semop(sem_set_id, sem_op, 2); // performing this operation simultaneusly on two semaphors
};

void addMeal(int i){
    struct sembuf op[1];
    op[0].sem_num = i;
    op[0].sem_flg = 0;
    op[0].sem_op = 1;
    semop(eaten_id, op, 1);
}

void philosopher(int philosopher_id) {
    int nb_of_meals = 0;

    while(1){
        printf("Philosopher %d is thinking.\n", philosopher_id);
        //sleep(rand() % THINKING_TIME_RANGE);

        printf("Philosopher %d wants to eat.\n", philosopher_id);

        while(nb_of_meals > semctl(eaten_id, philosopher_id, GETVAL) || nb_of_meals > semctl(eaten_id, (philosopher_id + N - 1) % N , GETVAL) ) {}
        grab_forks(philosopher_id); // left fork has id the same as a philosopher

        nb_of_meals++;
        printf("Philosopher %d is eating. Meal number %d\n", philosopher_id, nb_of_meals);

        addMeal(philosopher_id);

        put_away_forks(philosopher_id);
    }
}

int main(){
    printf("hello");

    int i;
    pid_t pid[N];

    // creating N semaphores
    sem_set_id = semget (IPC_PRIVATE, N, 0666 | IPC_CREAT); // IPC_PRIVATE- ensures that the semaphore set is private to the process and cannot be accessed by other processes directly.
                                                // 0666- permissions for read and write
                                                // sem_set_id is an identifier of these semaphores

    eaten_id = semget (IPC_PRIVATE, N, 0666 | IPC_CREAT);
    
    if(sem_set_id < 0) {
        printf("Error while creating semaphores!");
        exit(1);
    }

    // initializing semaphores
    for (i = 0; i < N; i ++){ // set all semaphores to value 1 (forks available)
        if (semctl (sem_set_id, i , SETVAL, 1) < 0){
            printf("Error while initializing the semaphores!");
            exit(1);
        }
    }
    // create child processes 
    for (i = 0; i < N; i++){
         pid[i] = fork();

        if (pid[i] < 0) {
            printf("Error: Fork failed.\n");
            exit(1);
        } 

        else if (pid[i] == 0){
            philosopher(i);
        }
    }
        // wait for all processes to terminate

        for (i = 0; i < N; i++){
            wait(NULL);
        }
    return 0;
}
