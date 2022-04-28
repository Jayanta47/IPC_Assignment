#include "passenger.h"

extern int readTimeCount();

extern sem_t print_mutex;
extern sem_t forward_cnt_mtx;
extern sem_t allow_r2l;
extern sem_t dir_lock;
extern sem_t backward_cnt_mtx;

extern int vip_forward_cnt;
extern int vip_backward_cnt;
extern int t_z;

void vip_channel_forward(Passenger *p) {
    // have higher priority
    // vip channel left-right
    int curr_time  = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d%shas started waiting for walking on VIP channel(left-to-right) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    sem_wait(&forward_cnt_mtx);
    vip_forward_cnt++;

    curr_time = readTimeCount();

    if (vip_forward_cnt == 1) {
        sem_wait(&allow_r2l);
        sem_wait(&dir_lock);
    }
    sem_post(&forward_cnt_mtx);

    curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d%shas started walking on VIP channel(left-to-right) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(t_z*1000));
    // sleep(t_z);

    curr_time = readTimeCount();

    sem_wait(&print_mutex);
    printf("Passenger %d%shas finished walking on VIP channel(left-to-right) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);


    sem_wait(&forward_cnt_mtx);
    vip_forward_cnt--;
    if (vip_forward_cnt == 0) {
        sem_post(&dir_lock);
        sem_post(&allow_r2l);
    }
    sem_post(&forward_cnt_mtx);
}

void vip_channel_backward(Passenger *p) {
    int curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d%shas started waiting for walking on VIP channel(right-to-left) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);
    sem_wait(&allow_r2l);
    sem_post(&allow_r2l);

    sem_wait(&backward_cnt_mtx);
    vip_backward_cnt++;
    if (vip_backward_cnt==1) {
        sem_wait(&dir_lock);
    }
    sem_post(&backward_cnt_mtx);

    curr_time = readTimeCount();

    sem_wait(&print_mutex);
    printf("Passenger %d%shas started walking on VIP channel(right-to-left) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(t_z*1000));
    // sleep(t_z);

    curr_time = readTimeCount();

    sem_wait(&backward_cnt_mtx);
    vip_backward_cnt--;
    if (vip_backward_cnt==0) {
        sem_post(&dir_lock);
    }
    sem_post(&backward_cnt_mtx);

    sem_wait(&print_mutex);
    printf("Passenger %d%shas finished walking on VIP channel(right-to-left) at time %d\n",
                 p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    

}
