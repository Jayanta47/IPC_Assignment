#ifndef PASSENGER_H
#define PASSENGER_H

#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include<string> 
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<math.h>
#include<random>
#include<vector>

#define passengers_per_hr 10
#define total_sim_time 60 // in minutes
#define loose_bpass_percent 20
#define percent_VIP 20
#define psnger_create_diff 1 // the difference in time between each passenger creation process
#define system_time_sim true 

struct Passenger {
    int id;
    bool isVIP;
    bool gotBoardingPass;
};

#endif