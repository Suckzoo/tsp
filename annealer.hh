#ifndef __ANNEALER_HH__
#define __ANNEALER_HH__
#include <random>
using namespace std;

class Annealer {
private:
    std::random_device rd;
    std::mt19937_64 gen;
    std::uniform_real_distribution<double> distribution;
    double T;
public:
    Annealer(double _T=1000.0) : T(_T) {
        gen = std::mt19937_64(rd());
        distribution = uniform_real_distribution<double>(0,1.0);
    }
    bool P(double cost, double base) {
        if (cost < base) return true;
        double p = distribution(gen);
        double e = exp((base - cost) / T);
        if (e - p > 0) return true;
        return false;
    }
    void cool() {
        T -= 0.1;
    }
};
#endif