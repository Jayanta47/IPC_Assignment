#include "TimeObj.h"
#include "passenger.h"
#include "timer.cpp"
#include "poisson_dist.cpp"
#include "kiosk.cpp"
#include "security.cpp"
#include "boarding_gate.cpp"
#include "vip_channel.cpp"

using namespace std;


// airport specifics holding variables

int n_kiosk; // number of kiosks, M
int n_belts; // number of security belts, N
int n_pass_per_belt; // number of passengers each belt can serve

// time units for different tasks

int t_w; // self check in at a kiosk
int t_x; // security check
int t_y; // boarding at the gate
int t_z; // walking on VIP channel on either direction

// variable to record the starting time of the program
time_t t_begin;

int *passengerFreq;
int passengerID = 0;
int time_counter = 0;
int current_kiosk = 1;
int vip_forward_cnt = 0;
int vip_backward_cnt = 0;
int t_pass;
int global_pssnger_id;

// time extracting read and write count
int writeCount;
int readCount;

// pointer to extract the time from TimeObj Class
TimeObj *t_ctr;

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

// all the necessary binary semaphores for time count read and write

sem_t time_lock_mtx_1; 
sem_t time_lock_mtx_2; 
sem_t time_lock_mtx_3; 
sem_t time_lock_mtx_w;
sem_t time_lock_mtx_r;

// semaphore to restrict trhe access to Passengers distribution
sem_t psnger_dist_mtx;
 

void * passengerThread(void* arg);
void readInputFile(string fileName);
void *createNewPassengerThread(void *arg);

// time reading function that uses global variable as time counter
// int readTimeCount() {
//     sem_wait(&time_lock_mtx_1);
//     sem_post(&time_lock_mtx_1);

//     sem_wait(&time_lock_mtx_2);
//     int timeCnt = t_ctr->readTime();
//     sem_post(&time_lock_mtx_2);
    
//     return timeCnt;
// }

// time reading function that uses system time
int readTimeCount() {
    sem_wait(&time_lock_mtx_3);
    time_t end = time(NULL);
    int timeCnt = static_cast<int> (end-t_begin);
    sem_post(&time_lock_mtx_3);

    return timeCnt;
}



void * passengerThread(void* arg) {
    Passenger* p = ((Passenger*)arg);
    arg = nullptr;

    kiosk_check(p);

    // Inside the security check
    if (!p->isVIP) {
        security_check(p);
    }
    else {
        // move through the vip belt
        vip_channel_forward(p);
    }
    boarding_gate_check(p);

}


int main(void) {

    readInputFile("input.txt");
    cout<<"====================================="<<endl;
    cout<<"Airport Simulation"<<endl;
    cout<<"====================================="<<endl;
    cout<<"Relevant Info:"<<endl;
    cout<<"# Self check-in at a kiosk, t_w: "<<t_w<<endl;
    cout<<"# Security check, t_x: "<<t_x<<endl;
    cout<<"# Boarding at the gate, t_y: "<<t_x<<endl;
    cout<<"# Walking on VIP channel, t_z: "<<t_z<<endl;
    cout<<"# Passenger arrival rate: "<<passengers_per_hr<<" /hour"<<endl;
    cout<<"# Total simulation time: "<<total_sim_time<<" minutes"<<endl;

    poisson_dist_func(passengers_per_hr, total_sim_time, psnger_create_diff);

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

        // timer lock semaphores
    sem_init(&time_lock_mtx_1, 0, 1);
    sem_init(&time_lock_mtx_2, 0, 1);
    sem_init(&time_lock_mtx_3, 0, 1);
    sem_init(&time_lock_mtx_w, 0, 1);
    sem_init(&time_lock_mtx_r, 0, 1);

        // passenger dist lock
    sem_init(&psnger_dist_mtx, 0, 1);

        // initializing time lock based reader and writer count 
    writeCount = 0;
    readCount = 0;

    global_pssnger_id = 1;

    srand(time(nullptr));

        // record the beginning time of the simulation
    t_begin = time(NULL);

    if (!system_time_sim) {
        t_ctr = new TimeObj();
        Timer t(true);
        t.setTimeout(total_sim_time*1000*3);
        t.setInterval(1000);
    }


        // Timer to create passengers
    Timer t2(false);
    t2.setTimeout(total_sim_time*1000);
    t2.setInterval(psnger_create_diff*1000);

    // while(1);

    pthread_exit(NULL);

    return 0;
}


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

void *createNewPassengerThread(void *arg) {
    int *timeslot = (int*) arg;
    arg = nullptr;
    // cout<<"threading here "<<*timeslot<<endl;
    if (*timeslot < (total_sim_time/psnger_create_diff)) {
        sem_wait(&psnger_dist_mtx);
        int fx = *(passengerFreq + *timeslot);
        // cout<<fx<<endl;
        pthread_t *psnger;
        for(int i=global_pssnger_id; i<global_pssnger_id+fx;i++) {
            psnger = new pthread_t;
            Passenger *p = new Passenger;
            p->id = i;
            p->isVIP = ( (abs(random())%100+1)>=(100-percent_VIP))? true:false;
            pthread_create(psnger, NULL, passengerThread, (void*)p);
        }
        global_pssnger_id+=fx;
        sem_post(&psnger_dist_mtx);
    }
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



// int readTimeCount() {
//     sem_wait(&time_lock_mtx_3);
//     sem_wait(&time_lock_mtx_r);
//     sem_wait(&time_lock_mtx_1);
//     readCount++;
//     if (readCount==1) sem_wait(&time_lock_mtx_w);
//     sem_post(&time_lock_mtx_1);
//     sem_post(&time_lock_mtx_r);
//     sem_post(&time_lock_mtx_3);

//     int timeCnt = t_ctr->readTime();

//     sem_wait(&time_lock_mtx_1);
//     readCount--;
//     if (readCount==0) sem_post(&time_lock_mtx_w);
//     sem_post(&time_lock_mtx_1);
//     return timeCnt;

// }