#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<string> 
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<math.h>
#include<random>
#include<vector>

using namespace std;

#define passengers_per_hr 200
#define total_sim_time 60 // in minutes
#define loose_bpass_percent 20

// airport specifics holding variables

int n_kiosk; // number of kiosks, M
int n_belts; // number of security belts, N
int n_pass_per_belt; // number of passengers each belt can serve

// time units for different tasks

int t_w; // self check in at a kiosk
int t_x; // security check
int t_y; // boarding at the gate
int t_z; // walking on VIP channel on either direction


int *passengerFreq;
int passengerID = 0;
int time_counter = 0;
int current_kiosk = 1;
int vip_forward_cnt = 0;
int vip_backward_cnt = 0;

vector<bool> kiosk_slots;

// semaphores

// mutex for output purpose so that no two threads can 
// print at the same time
sem_t print_mutex;

// kiosk
// there are in total M kiosk
// used as a counting semaphore
sem_t kiosk_mtx;

// binary semaphore to lock the current kiosk number asssigned to a passenger
sem_t kiosk_slot_mtx;

// security check 
// N security belts with each belt having ability to serve P passengers at a time

vector<sem_t> security_check_semaphores; 


// boarding gate mutex
// allows only one poassenger at a time
sem_t boarding_gate_mtx;

// vip forward direction passenger count mutex
// binary semaphore
sem_t forward_cnt_mtx;

// vip backward direction passenger count mutex
// binary semaphore
sem_t backward_cnt_mtx;

// semaphore to control the access of right to left passenger's into VIP channel
// bianry semaphore
sem_t allow_r2l;

// semaphore to lock the change of direction of VIP channel
sem_t dir_lock;

// special kiosk mutex
sem_t special_kiosk_mtx;

struct Passenger {
    int id;
    bool isVIP;
    bool gotBoardingPass;
};


void readInputFile(string fileName) {
    ifstream InputFile(fileName);
    int num;
    
    if (!InputFile.is_open()) {
        printf("Failed to Open File: %s\n", fileName.c_str());
        return;
    }

    vector<int> values;

    while(InputFile>>num) {
        values.push_back(num);
    }

    n_kiosk = values[0];
    n_belts = values[1];
    n_pass_per_belt = values[2];

    t_w = values[3];
    t_x = values[4];
    t_y = values[5];
    t_z = values[6];
    
    // printf("%d %d %d %d %d %d %d\n", 
    //             n_kiosk, n_belts, n_pass_per_belt, t_w, t_x, t_y, t_z);

}


void poisson_dist_func() {
    // const int nrolls = 10000; // number of experiments
    const int nstars = passengers_per_hr;   // maximum number of stars to distribute

    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(total_sim_time/2);

    passengerFreq = new int[total_sim_time] {};

    for (int i=0; i<nstars; ++i) {
        int number = distribution(generator);
        // cout<<number<<endl;
        if (number<total_sim_time) ++passengerFreq[number];
    }


}

void generatePassengerFreq() {
    poisson_dist_func();
    if (passengerFreq == nullptr){
        cout<<"null"<<endl;
    }
    // std::cout << "poisson_distribution:" << std::endl;
    // for (int i=0; i<total_sim_time; ++i)
    //     std::cout << i << ": " << std::string(passengerFreq[i],'*') << std::endl;
}


void kiosk_check(int passenger_id) {

    sem_wait(&print_mutex);
    printf("Passenger %d has arrived at the airport at time %d\n", passenger_id, time_counter);
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
    int curr_time = time_counter;

// should this be done outside the critical region??
    sem_wait(&print_mutex);
    printf("Passenger %d has started self check in at kiosk %d at time %d\n", passenger_id, current_kiosk+1, curr_time);
    sem_post(&print_mutex);


    sleep(t_w);

    curr_time = curr_time+t_w; // NOTICE THIS

    sem_wait(&kiosk_slot_mtx);
    if (current_kiosk==0 && kiosk_slots[0]==false) {
        kiosk_slots[0] = true;
    }
    else {
        kiosk_slots[current_kiosk] = true;
    }
    sem_post(&kiosk_slot_mtx);

    sem_post(&kiosk_mtx);

    sem_wait(&print_mutex);
    printf("Passenger %d has finished check in at time %d\n", passenger_id, curr_time);
    sem_post(&print_mutex);
}

void vip_channel_forward(int passenger_id) {
    // have higher priority
    // vip channel left-right
    int curr_time  = time_counter;
    sem_wait(&print_mutex);
    printf("Passenger %d has started waiting for walking on VIP channel(left-to-right) at time %d\n",
                 passengerID, curr_time);
    sem_post(&print_mutex);

    sem_wait(&forward_cnt_mtx);
    vip_forward_cnt++;

    curr_time = time_counter;

    if (vip_backward_cnt == 1) {
        sem_wait(&allow_r2l);
        sem_wait(&dir_lock);
    }
    sem_post(&forward_cnt_mtx);

    curr_time = time_counter;
    sem_wait(&print_mutex);
    printf("Passenger %d has started walking on VIP channel(left-to-right) at time %d\n",
                 passengerID, curr_time);
    sem_post(&print_mutex);

    sleep(t_z);

    sem_wait(&print_mutex);
    printf("Passenger %d has finished walking on VIP channel(left-to-right) at time %d\n",
                 passengerID, curr_time+t_z);
    sem_post(&print_mutex);


    sem_wait(&forward_cnt_mtx);
    vip_forward_cnt--;
    if (vip_forward_cnt == 0) {
        sem_post(&dir_lock);
        sem_post(&allow_r2l);
    }
    sem_post(&forward_cnt_mtx);
}

void vip_channel_backward(int passenger_id) {
    sem_wait(&allow_r2l);
    sem_post(&allow_r2l);
    int curr_time = time_counter;

    sem_wait(&print_mutex);
    printf("Passenger %d has started waiting for walking on VIP channel(right-to-left) at time %d\n",
                 passengerID, curr_time);
    sem_post(&print_mutex);

    sem_wait(&backward_cnt_mtx);
    vip_backward_cnt++;
    if (vip_backward_cnt==1) {
        sem_wait(&dir_lock);
    }
    sem_post(&backward_cnt_mtx);

    curr_time = time_counter;

    sem_wait(&print_mutex);
    printf("Passenger %d has started walking on VIP channel(right-to-left) at time %d\n",
                 passengerID, curr_time);
    sem_post(&print_mutex);

    sleep(t_z);

    sem_wait(&print_mutex);
    printf("Passenger %d has finished walking on VIP channel(right-to-left) at time %d\n",
                 passengerID, curr_time+t_z);
    sem_post(&print_mutex);

    sem_wait(&backward_cnt_mtx);
    vip_backward_cnt--;
    if (vip_backward_cnt==0) {
        sem_post(&dir_lock);
    }
    sem_post(&backward_cnt_mtx);



}

void goto_special_kiosk(Passenger *p) {
    int curr_time;

    sem_wait(&special_kiosk_mtx);

    curr_time = time_counter;

    sem_wait(&print_mutex);
    printf("Passenger %d has started self-check in at special kiosk at time %d\n", p->id, curr_time);
    sem_post(&print_mutex);

    sleep(t_w);
    p->gotBoardingPass = true;

    sem_post(&special_kiosk_mtx);

    sem_wait(&print_mutex);
    printf("Passenger %d has finished check in at special kiosk at time %d\n", p->id, curr_time+t_w);
    sem_post(&print_mutex);

    vip_channel_forward(p->id);
    boarding_gate_check(p); // notice, should it not be implemented in vip gate forward?


}

bool boarding_gate_check(Passenger *p) {
    // changing the availability of boarding pass
    // letting lose_bpass_percent number of passengers to loose boarding pass

    int r_num = abs(random())%100+1;
    if (r_num>=100-loose_bpass_percent) p->gotBoardingPass = false;

    int curr_time = time_counter;
    
    if (p->gotBoardingPass) {
        sem_wait(&print_mutex);
        printf("Passenger %d has started waiting to be boarded at time %d\n", p->id, curr_time);
        sem_post(&print_mutex);

        sem_wait(&boarding_gate_mtx);

        curr_time = time_counter;
        sem_wait(&print_mutex);
        printf("Passenger %d has started boarding the plane at time %d\n", p->id, curr_time);
        sem_post(&print_mutex);

        sleep(t_y);

        sem_post(&boarding_gate_mtx);

        sem_wait(&print_mutex);
        printf("Passenger %d has boarded the plane at time %d\n", p->id, curr_time+t_y);
        sem_post(&print_mutex);

        return true;
    }
    else {
        vip_channel_backward(p->id);
        goto_special_kiosk(p);


    }

    return false;


}

void * passengerThread(void* arg) {
    Passenger* p = ((Passenger*)arg);
    arg = nullptr;

    kiosk_check(p->id);

    // Inside the security check
    if (!p->isVIP) {
        // if the passenger is not a VIP, s/he has to go through the security check
        // randomly select any belt to keep in line
        int index = abs(random())%n_belts;
        sem_t *security_mtx = &security_check_semaphores.at(index);

        int curr_time = time_counter;
        sem_wait(&print_mutex);
        printf("Passenger %d has started waiting for security check in belt %d from time %d\n", p->id, index, curr_time);
        sem_post(&print_mutex);
        sem_wait(security_mtx);
        
        curr_time = time_counter;
        sem_wait(&print_mutex);
        printf("Passenger %d has started the security check at time %d\n", p->id, curr_time);
        sem_post(&print_mutex);

        sleep(t_x);

        sem_post(security_mtx);

        p->gotBoardingPass = true;

        sem_wait(&print_mutex);
        printf("Passenger %d has crossed the security check at time %d\n", p->id, curr_time+t_x);
        sem_post(&print_mutex);


    }
    else {
        // move through the vip belt
        vip_channel_forward(p->id);
    }
    
    

    boarding_gate_check(p);

}


int main(void) {

    readInputFile("input.txt");
    generatePassengerFreq();

    // initializing semaphores
    sem_init(&print_mutex, 0, 1);

    sem_init(&kiosk_mtx, 0, n_kiosk);

        // total number of security belts is n_belts(N) and each has passes n_pass_per_belts(P)
    for(int i=0;i<n_belts;i++) {
        sem_t *sec_booth_mtx = new sem_t;
        sem_init(sec_booth_mtx, 0, n_pass_per_belt);
        security_check_semaphores.push_back(*sec_booth_mtx);
    }

    sem_init(&kiosk_slot_mtx, 0, 1);

    // kiosk slot initialization
    for(int i=-0;i<n_kiosk;i++) {
        kiosk_slots.push_back(true);
    }

        // boarding gate mutex 
    
    sem_init(&boarding_gate_mtx, 0, 1);

        // forward count mutex init
    
    sem_init(&forward_cnt_mtx, 0, 1);

        // backward count mutex init

    sem_init(&backward_cnt_mtx, 0, 1);

    sem_init(&allow_r2l, 0, 1);

    sem_init(&dir_lock, 0, 1);

    sem_init(&special_kiosk_mtx, 0, 1);
    

    // testFunc();
    pthread_t pt1;
    Passenger *p1 = new Passenger;
    p1->id = 10;
    p1->isVIP = false;
    pthread_create(&pt1, NULL, passengerThread, (void*)p1);

    while(1);
    return 0;
}















// float nextTime()
// {
//     return -logf(1.0f - (float) random() / RAND_MAX ) / (passengers_per_hr/60.0);
// }

// void generatePassengerFreq() {
//     int p[20] = {};

//     for(int i=0;i<passengers_per_hr;i++) {
//         int slot = round(nextTime());
//         cout<<slot<<endl;
//         p[slot]++;
//     }

//     for(int i=0;i<20;i++) {
//         cout<<i<<" : "<<p[i]<<endl;
//     }
// }