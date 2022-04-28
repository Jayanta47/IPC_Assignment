#include "passenger.h"


using namespace std;

extern int n_kiosk;
extern vector<bool> kiosk_slots;
extern int t_w;

extern int readTimeCount();
extern void vip_channel_forward(Passenger *p);
bool boarding_gate_check(Passenger *p);

extern sem_t print_mutex;
extern sem_t kiosk_mtx;
extern sem_t kiosk_slot_mtx;
extern sem_t special_kiosk_mtx;


void kiosk_check(Passenger *p) {
    int curr_time;

    curr_time = readTimeCount();
    sem_wait(&print_mutex);
    printf("Passenger %d%shas arrived at the airport at time %d\n", p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    int current_kiosk;
    // waiting for the kiosk
    sem_wait(&kiosk_mtx);
    sem_wait(&kiosk_slot_mtx);
    for( current_kiosk=0;current_kiosk<n_kiosk;current_kiosk++) {
        if (kiosk_slots[current_kiosk]) {
            kiosk_slots[current_kiosk] = false;
            break;
        } 
    }
    sem_post(&kiosk_slot_mtx);
    if (current_kiosk == n_kiosk) current_kiosk=0;
    curr_time = readTimeCount();

// should this be done outside the critical region??
    sem_wait(&print_mutex);
    printf("Passenger %d%shas started self check in at kiosk %d at time %d\n", p->id, p->isVIP?" (VIP) ":" ",current_kiosk+1, curr_time);
    sem_post(&print_mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(t_w*1000));
    // sleep(t_w);

    // curr_time = curr_time+t_w; // NOTICE THIS

    sem_wait(&kiosk_slot_mtx);
    if (current_kiosk==0 && kiosk_slots[0]==false) {
        kiosk_slots[0] = true;
    }
    else {
        kiosk_slots[current_kiosk] = true;
    }
    sem_post(&kiosk_slot_mtx);

    sem_post(&kiosk_mtx);

    curr_time = readTimeCount();

    sem_wait(&print_mutex);
    printf("Passenger %d%shas finished check in at time %d\n",p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);
}


void goto_special_kiosk(Passenger *p) {
    int curr_time;

    sem_wait(&special_kiosk_mtx);

    curr_time = readTimeCount();

    sem_wait(&print_mutex);
    printf("Passenger %d%shas started self-check in at special kiosk at time %d\n", p->id, p->isVIP?" (VIP) ":" ", curr_time);
    sem_post(&print_mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(t_w*1000));
    // sleep(t_w);
    p->gotBoardingPass = true;

    sem_post(&special_kiosk_mtx);

    sem_wait(&print_mutex);
    printf("Passenger %d%shas finished check in at special kiosk at time %d\n", p->id, p->isVIP?" (VIP) ":" ", curr_time+t_w);
    sem_post(&print_mutex);

    vip_channel_forward(p);
    boarding_gate_check(p); // notice, should it not be implemented in vip gate forward?


}