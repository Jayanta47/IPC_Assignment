#include "passenger.h"

extern int readTimeCount();
extern void vip_channel_backward(Passenger *p);
extern void goto_special_kiosk(Passenger *p);

extern sem_t print_mutex;
extern sem_t boarding_gate_mtx;

extern int t_y;

bool boarding_gate_check(Passenger *p) {
    // changing the availability of boarding pass
    // letting lose_bpass_percent number of passengers to loose boarding pass

    int r_num = abs(random())%100+1;
    if (r_num>=100-loose_bpass_percent) p->gotBoardingPass = false;

    int curr_time = readTimeCount();
    
    if (p->gotBoardingPass) {
        sem_wait(&print_mutex);
        printf("Passenger %d%shas started waiting to be boarded at time %d\n", p->id, p->isVIP?" (VIP) ":" ",curr_time);
        sem_post(&print_mutex);

        sem_wait(&boarding_gate_mtx);

        curr_time = readTimeCount();
        sem_wait(&print_mutex);
        printf("Passenger %d%shas started boarding the plane at time %d\n", p->id, p->isVIP?" (VIP) ":" ",curr_time);
        sem_post(&print_mutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(t_y*1000));
        // sleep(t_y);

        sem_post(&boarding_gate_mtx);

        curr_time = readTimeCount();

        sem_wait(&print_mutex);
        printf("Passenger %d%shas boarded the plane at time %d\n", p->id, p->isVIP?" (VIP) ":" ",curr_time);
        sem_post(&print_mutex);

        return true;
    }
    else {
        sem_wait(&print_mutex);
        printf("Passenger %d%shas lost his/her boarding pass\n", p->id, p->isVIP?" (VIP) ":" ");
        sem_post(&print_mutex);
        vip_channel_backward(p);
        goto_special_kiosk(p);


    }

    return false;


}