#ifndef __TSP_HH__
#define __TSP_HH__
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <queue>
#define MAX_N 20000
using namespace std;

inline void swap(int &x, int &y)
{
    x ^= y;
    y ^= x;
    x ^= y;
}

class Point {
public:
    int i;
    double x;
    double y;
    double operator - (const Point &p) const {
        return sqrt(((x - p.x) * (x - p.x)) + ((y - p.y) * (y - p.y)));
    }
    bool operator < (const Point &p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
};

class TSPPath {
public:
    vector <int> up;
    vector <int> down;
    TSPPath() {
    }

    void shufflePath(vector <int> &v) {
        int n = v.size();
        int s = rand() % n;
        int e = rand() % n;
        if (s > e) {
            swap(s,e);
        }
        auto it_begin = v.begin() + s;
        auto it_end = v.begin() + e;
        random_shuffle(it_begin, it_end);
    }

    TSPPath* mutateUppath() {
        TSPPath* newPath = new TSPPath;
        newPath->up = up;
        newPath->down = down;
        shufflePath(newPath->up);
        return newPath;
    }

    TSPPath* mutateDownpath() {
        TSPPath* newPath = new TSPPath;
        newPath->up = up;
        newPath->down = down;
        shufflePath(newPath->down);
        return newPath;
    }

    TSPPath* crossoverPath() {
        TSPPath* newPath = new TSPPath;
        int up_size = up.size(), down_size = down.size();

        int s_up, e_up;
        s_up = rand() % up_size;
        e_up = rand() % up_size;
        if (s_up > e_up) {
            swap(s_up, e_up);
        }

        int s_down, e_down;
        s_down = rand() % down_size;
        e_down = rand() % down_size;
        if (s_down > e_down) {
            swap(s_down, e_down);
        }

        vector <int> crossover_subset;

        int i;
        for(i = s_up; i <= e_up; i++) {
            crossover_subset.push_back(up[i]);
        }
        for(i = s_down; i <= e_down; i++) {
            crossover_subset.push_back(down[i]);
        }

        random_shuffle(crossover_subset.begin(), crossover_subset.end());

        int c = 0;
        for(i=0;i<up_size;i++) {
            if (i < s_up || i > e_up) {
                newPath->up.push_back(up[i]);
            } else {
                int coin_toss = rand() % 2;
                if (coin_toss) {
                    newPath->up.push_back(crossover_subset[c++]);
                }
            }
        }

        for(i=0;i<down_size;i++) {
            if (i < s_down || i > e_down) {
                newPath->down.push_back(down[i]);
            } else {
                while(c < crossover_subset.size()) {
                    newPath->down.push_back(crossover_subset[c++]);
                }
                i = e_down;
            }
        }
        return newPath;
    }
};

struct TSPPathDistPair {
    double dist;
    TSPPath *path;
    bool operator < (const TSPPathDistPair &p) const {
        return dist > p.dist; // STL priority_queue only supports max queue
    }
};

class TSPInstance {
private:
    int n;
    Point *a;
    int maxFitness;
    int maxPopulation;
    priority_queue <TSPPathDistPair> currentPopulation;
    priority_queue <TSPPathDistPair> nextPopulation;
    void _skipLine(FILE *fi, int numLine) {
        char _buf[100];
        for(int i=0;i<numLine;i++) {
            fgets(_buf,100,fi);
        }
    }
    void _parseN(FILE *fi) {
        int i;
        char _buf[100];
        fgets(_buf, 100, fi);
        this->n = 0;
        for(i=12;i<strlen(_buf)-1;i++) {
            this->n *= 10;
            this->n += (_buf[i] - '0');
        }
    }
public:
    TSPInstance(string filename, int _maxFitness=0, int _maxPopulation=10)
     : maxFitness(_maxFitness), maxPopulation(_maxPopulation)
    {
        FILE *fi = fopen(filename.c_str(), "r");
        int i;
        
        _skipLine(fi, 3);
        _parseN(fi);
        _skipLine(fi, 2);

        this->a = new Point[this->n];
        for(i=0;i<n;i++) {
            fscanf(fi, "%d %lf %lf", &a[i].i, &a[i].x, &a[i].y);
        }
        sort(a, a+n);
        fclose(fi);
        printf("instance initiated. (n = %d)\n", this->n);
    }
    ~TSPInstance() {
        delete[] this->a;
    }
    
    TSPPath* getBitonicPath() {
        printf("calculating bitonic path...\n");
        double **dy = new double*[this->n];
        pair<int, int> **back = new pair<int,int>*[this->n];
        int i,j;
        for(i=0;i<n;i++) {
            dy[i] = new double[this->n];
            back[i] = new pair<int,int>[this->n];
            for(j=0;j<n;j++) dy[i][j] = -1;
        }
        dy[0][0] = 0;
        for(i=0;i<n;i++) {
            for(j=0;j<n;j++) {
                if (i == j && i != 0) continue;
                int nextV = (i > j ? i : j) + 1;
                if (i == this->n-1 && j == this->n-1) {
                    continue;
                } else if (i == this->n-1) {
                    nextV = this->n-1;
                    if (dy[i][nextV] == -1 || dy[i][nextV] > dy[i][j] + (a[j] - a[nextV])) {
                        dy[i][nextV] = dy[i][j] + (a[j] - a[nextV]);
                        back[i][nextV] = pair<int,int>(i, j);
                    }
                } else if (j == this->n-1) {
                    nextV = this->n-1;
                    if (dy[nextV][j] == -1 || dy[nextV][j] > dy[i][j] + (a[i] - a[nextV])) {
                        dy[nextV][j] = dy[i][j] + (a[i] - a[nextV]);
                        back[nextV][j] = pair<int,int>(i, j);
                    }
                } else {
                    if (dy[i][nextV] == -1 || dy[i][nextV] > dy[i][j] + (a[j] - a[nextV])) {
                        dy[i][nextV] = dy[i][j] + (a[j] - a[nextV]);
                        back[i][nextV] = pair<int,int>(i, j);
                    }
                    if (dy[nextV][j] == -1 || dy[nextV][j] > dy[i][j] + (a[i] - a[nextV])) {
                        dy[nextV][j] = dy[i][j] + (a[i] - a[nextV]);
                        back[nextV][j] = pair<int,int>(i, j);
                    }
                }
            }
        }
        TSPPath *bitonic = new TSPPath;
        int cur_x = this->n-1;
        int cur_y = this->n-1;
        bitonic->up.push_back(this->n-1);
        for(i=0;i<n;i++) {
            pair<int,int> &p = back[cur_x][cur_y];
            if (cur_x == p.first) {
                bitonic->down.push_back(p.second);
            } else {
                bitonic->up.push_back(p.first);
            }
            cur_x = p.first;
            cur_y = p.second;
        }
        reverse(bitonic->up.begin(), bitonic->up.end());
        for(i=0;i<n;i++) {
            delete[] dy[i];
            delete[] back[i];
        }
        delete[] dy;
        delete[] back;
        printf("path calculated.\n");
        return bitonic;
    }

    double evaluatePath(TSPPath *path) {
        if (maxFitness > 0) maxFitness--;
        else if (maxFitness == 0) return -1;

        int i;
        int up_size = path->up.size();
        int down_size = path->down.size();
        double dist = 0;
        for(i=1;i<up_size;i++) {
            dist += (a[path->up[i]] - a[path->up[i-1]]);
        }
        dist += (a[path->up[up_size-1]] - a[path->down[0]]);
        for(i=1;i<down_size;i++) {
            dist += (a[path->down[i]] - a[path->down[i-1]]);
        }
        return dist;
    }

    void printPath(FILE *fo, TSPPath *path) {
        int i;
        int ups = path->up.size();
        int downs = path->down.size();
        for(i=0;i<ups;i++) {
            fprintf(fo, "%d\n", a[path->up[i]].i);
        }
        for(i=0;i<downs-1;i++) {
            fprintf(fo,"%d\n", a[path->down[i]].i);
        }
    }

    bool isFitnessAvailable() {
        return maxFitness != 0;
    }

    void stop() {
        maxFitness = 0;
    }

    bool nextCandidateAvailable() {
        return !currentPopulation.empty();
    }

    TSPPathDistPair getNextCandidate() {
        TSPPathDistPair _pair = currentPopulation.top();
        currentPopulation.pop();
        return _pair;
    }

    void pushCandidate(TSPPathDistPair _pair) {
        nextPopulation.push(_pair);
    }

    void managePopulation() {
        int i;
        for(i=0;i<maxPopulation;i++) {
            if (nextPopulation.empty()) break;
            currentPopulation.push(nextPopulation.top());
            nextPopulation.pop();
        }
        while(!nextPopulation.empty()) {
            TSPPathDistPair _pair = nextPopulation.top();
            delete _pair.path;
            nextPopulation.pop();
        }
    }

    int poolSize() {
        return currentPopulation.size();
    }
};
#endif
