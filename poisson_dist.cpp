#include<random>
using namespace std;

extern int* passengerFreq;

void poisson_dist_func(int psnger_rate, int sim_time) {
    // const int nrolls = 10000; // number of experiments
    const int nstars = psnger_rate;   // maximum number of stars to distribute

    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(sim_time/2);

    passengerFreq = new int[sim_time] {};

    for (int i=0; i<nstars; ++i) {
        int number = distribution(generator);
        // cout<<number<<endl;
        if (number<sim_time) ++passengerFreq[number];
    }

}