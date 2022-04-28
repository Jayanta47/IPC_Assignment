#include<random>
using namespace std;

extern int* passengerFreq;

void poisson_dist_func(int psnger_rate, int sim_time, int interval_time) {
    const int nstars = static_cast<int> (psnger_rate*(sim_time/60.0));   // maximum number of stars to distribute
    srand(time(nullptr));
    int mean_time = (int)(sim_time/5) + abs(rand())%(int)(sim_time-(sim_time/2)); 
    cout<<"# Mean Time: "<< mean_time<< " min"<<endl;
    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(mean_time);

    passengerFreq = new int[(int)(sim_time/interval_time)] {};
    // cout<<(int)(sim_time/interval_time)<<endl;
    for (int i=0; i<nstars; ) {
        int number = distribution(generator);
        if (number < sim_time) 
        {
            ++passengerFreq[number/interval_time];
            i++;
        }
        
    }

    // for(int i=0;i<(int)(sim_time/interval_time);i++) {
    //     cout<<i<<" : "<<passengerFreq[i]<<endl;
    // }

}