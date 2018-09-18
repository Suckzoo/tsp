#ifndef __RANDOMMACHINE_HH__
#define __RANDOMMACHINE_HH__
#include <random>
using namespace std;

class RandomMachine {
private:
    std::random_device rd;
    std::mt19937_64 gen;
    std::uniform_real_distribution<double> distribution;
    double T;
public:
    RandomMachine(double _T=1000.0) : T(_T) {
        gen(rd());
        distribution = uniform_real_distribution<double>(0,1.0);
    }
    bool P(double cost, double base) {
        if (cost < base) return true;
        double p = distribution(gen);
        double e = exp((cost - base) / T);
        if (e - p > 0) return true;
        return false;
    }
    void cool() {
        T -= 0.1;
    }
};
#endif