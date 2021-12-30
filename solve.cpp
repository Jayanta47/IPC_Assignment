#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<string> 
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<math.h>
#include<random>

using namespace std;

#define passengers_per_hr 200
#define total_sim_time 60 // in minutes

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

struct Passenger {
    int id;
    bool isVIP;
    bool gotBoardingPass;
};

void * passengerThread(void* arg) {
    Passenger* p = ((Passenger*)arg);

    arg = nullptr;
    printf("passenger id : %d\n", p->id);
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
    std::cout << "poisson_distribution:" << std::endl;
    for (int i=0; i<total_sim_time; ++i)
        std::cout << i << ": " << std::string(passengerFreq[i],'*') << std::endl;
        // cout<<passengerFreq[i]<<endl;
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

int main(void) {

    readInputFile("input.txt");
    generatePassengerFreq();
    // testFunc();
    pthread_t pt1;
    Passenger *p1 = new Passenger;
    p1->id = 10;
    p1->isVIP = false;
    pthread_create(&pt1, NULL, passengerThread, (void*)p1);

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