#include<iostream>
#include "TimeObj.h"
#include<pthread.h>
#include<semaphore.h>
#include<thread>

using namespace std;

/**
 * @brief there is an implementation of reader writer problem 
 * taken from the paper 
 * https://www.researchgate.net/publication/234787964_Concurrent_control_with_readers_and_writers
 * 
 */


extern sem_t time_lock_mtx_2; 
extern sem_t time_lock_mtx_1;
extern sem_t time_lock_mtx_r;

extern int writeCount;
extern int t_pass;
extern void *createNewPassengerThread(void* arg);
extern int readTimeCount();
extern TimeObj *t_ctr;

pthread_t *psngr_create;


class Timer {
     bool clear;
     bool passengerCreateType;

public:
    Timer(bool countType) {
        this->clear = false;
        this->passengerCreateType = !countType;
    }
    void setTimeout(int delay);

    void setInterval(int interval);

    void stop();
};

void Timer::setTimeout(int delay) {
    this->clear = false;
    std::thread t([=]() {
        if(this->clear) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(this->clear) return;
        cout<<"stopping thread"<<endl;
        clear = true;
    });
    t.detach();
}

void Timer::setInterval(int interval) {
    this->clear = false;
    std::thread t([=]() {
        while(true) {
            if(this->clear) return;
            // t_pass = t_ctr->readTime();
            // psngr_create = new pthread_t;
            // pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
            // if (pthread_detach(*psngr_create)) {
            //     cerr<<"error"<<endl;
            // }
            
            if (this->passengerCreateType) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(this->clear) return;
                t_pass = readTimeCount()-1;
                psngr_create = new pthread_t;
                // cout<<"start"<<endl;
                pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
                if (pthread_detach(*psngr_create)) {
                    cerr<<"error"<<endl;
                }
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval-5));
                if(this->clear) return;
                sem_wait(&time_lock_mtx_1);
                sem_wait(&time_lock_mtx_2);
                t_ctr->incCounter();
                
                sem_post(&time_lock_mtx_2);
                sem_post(&time_lock_mtx_1);
            }
            

        }
    });
    t.detach();
}



void Timer::stop() {
    this->clear = true;
}


// int main(void) {
//     Timer t = Timer();

//     t.setTimeout(5000);

//     t.setInterval(1000);

//     while(1);
// }


/*

class Timer {
    bool clear;
    TimeObj *t_ctr;

public:
    Timer() {
        this->clear = false;
        t_ctr = new TimeObj();
    }
    template<typename Function>
    void setTimeout(Function function, int delay);

    template<typename Function>
    void setInterval(Function function, int interval);

    void stop();
};

void Timer::setTimeout(auto function, int delay) {
    this->clear = false;
    std::thread t([=]() {
        if(this->clear) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(this->clear) return;
        function();
    });
    t.detach();
}

void Timer::setInterval(auto function, int interval) {
    this->clear = false;
    std::thread t([=]() {
        while(true) {
            if(this->clear) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if(this->clear) return;
            function();
        }
    });
    t.detach();
}

void Timer::stop() {
    this->clear = true;
}

*/




// void Timer::setInterval(int interval) {
//     this->clear = false;
//     std::thread t([=]() {
//         while(true) {
//             if(this->clear) return;
//             // t_pass = t_ctr->readTime();
//             // psngr_create = new pthread_t;
//             // pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
//             // if (pthread_detach(*psngr_create)) {
//             //     cerr<<"error"<<endl;
//             // }
//             std::this_thread::sleep_for(std::chrono::milliseconds(interval));
//             if(this->clear) return;
//             sem_wait(&time_lock_mtx_2);
//             writeCount++;
//             if (writeCount==1) sem_wait(&time_lock_mtx_r);
//             sem_post(&time_lock_mtx_2);
//             sem_wait(&time_lock_mtx_w);

//             t_ctr->incCounter();
//             // t_pass = t_ctr->readTime()-1;
//             // psngr_create = new pthread_t;
//             // pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
//             // if (pthread_detach(*psngr_create)) {
//             //     cerr<<"error"<<endl;
//             // }

//             sem_post(&time_lock_mtx_w);
//             sem_wait(&time_lock_mtx_2);
//             writeCount--;
//             if (writeCount == 0) sem_post(&time_lock_mtx_r);
//             sem_post(&time_lock_mtx_2);

//             // createNewPassengerThread();

//             t_pass = t_ctr->readTime()-1;
//             psngr_create = new pthread_t;
//             // cout<<"start"<<endl;
//             pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
//             if (pthread_detach(*psngr_create)) {
//                 cerr<<"error"<<endl;
//             }
//             // cout<<"done"<<endl;
//         }
//     });
//     t.detach();
// }


// std::thread *Timer::setInterval(int interval) {
//     this->clear = false;
//     std::thread *t = new std::thread([=]() {
//         while(true) {
//             if(this->clear) return;
//             // t_pass = t_ctr->readTime();
//             // psngr_create = new pthread_t;
//             // pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
//             // if (pthread_detach(*psngr_create)) {
//             //     cerr<<"error"<<endl;
//             // }
            
//             if (this->passengerCreateType) {
//                 std::this_thread::sleep_for(std::chrono::milliseconds(interval));
//                 if(this->clear) return;
//                 t_pass = readTimeCount()-1;
//                 psngr_create = new pthread_t;
//                 // cout<<"start"<<endl;
//                 pthread_create(psngr_create, NULL, createNewPassengerThread, (void*)&t_pass);
//                 if (pthread_detach(*psngr_create)) {
//                     cerr<<"error"<<endl;
//                 }
//             }
//             else {
//                 std::this_thread::sleep_for(std::chrono::milliseconds(interval-5));
//                 if(this->clear) return;
//                 sem_wait(&time_lock_mtx_1);
//                 sem_wait(&time_lock_mtx_2);
//                 t_ctr->incCounter();
                
//                 sem_post(&time_lock_mtx_2);
//                 sem_post(&time_lock_mtx_1);
//             }
            

//         }
//     });
//     t->detach();

//     return t;
// }
