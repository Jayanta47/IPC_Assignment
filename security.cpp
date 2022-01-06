#include<random>
#include "passenger.h"
#include<semaphore.h>
#include<vector>

extern int n_belts;
extern int t_x;
extern vector<sem_t> security_check_semaphores; 

extern int readTimeCount();

extern sem_t print_mutex;


void security_check(Passenger *p) {
    // if the passenger is not a VIP, s/he has to go through the security check
    // randomly select any belt to keep in line
    int index = abs(random())%n_belts;
    sem_t *security_mtx = &security_check_semaphores.at(index);

    int curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d has started waiting for security check in belt %d from time %d\n", p->id, index, curr_time);
    sem_post(&print_mutex);
    sem_wait(security_mtx);
    
    curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d has started the security check at time %d\n", p->id, curr_time);
    sem_post(&print_mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(t_x*1000));
    // sleep(t_x);

    sem_post(security_mtx);

    p->gotBoardingPass = true;

    curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d has crossed the security check at time %d\n", p->id, curr_time);
    sem_post(&print_mutex);
}