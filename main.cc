#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <ctime>
#include <string>
#include <random>
#include "tsp.hh"
#include "randommachine.hh"

TSPInstance *instance;
RandomMachine *annealer;

void parseArgs(int argc, char **argv) {
    int maxFitness = -1; // infinite mode
    int maxPopulation = 10;
    int initialTemperature = 1000;
    if (argc <= 1) {
        printf("Usage: ./tsp [-f maxFitness] [-p maxPopulation] [-T initialTemperature] [filename]\n");
        exit(1);
    }
    int i;
    for (i=1;i<argc;i++) {
        if(!strcmp(argv[i], "-f")) {
            i++;
            maxFitness = atoi(argv[i]);
        } else if(!strcmp(argv[i], "-p")) {
            i++;
            maxPopulation = atoi(argv[i]);
        } else if(!strcmp(argv[i], "-T")) {
            i++;
            initialTemperature = atoi(argv[i]);
        } else {
            instance = new TSPInstance(argv[i], maxFitness, maxPopulation);
            annealer = new RandomMachine((double)initialTemperature);
            return;
        }
    }
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("SIGINT caught.\n");
        printf("Now gracefully stopping optimizer...\n");
        instance->stop();
    }
}

void report(TSPPathDistPair path) {
    const char* fmt = "%Y-%m-%dT%H:%M";
    char datetime_now[80], filename[100];
    time_t now = time(0);
    tm* lc_now = localtime(&now);
    strftime(datetime_now, 80, fmt, lc_now);
    printf("[%s] New distance caught: %.2lf\n", datetime_now, path.dist);
    sprintf(filename, "log/%s.csv", datetime_now);
    FILE *log = fopen(filename, "w");
    instance->printPath(log, path.path);
    fclose(log);
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handler);
    parseArgs(argc, argv);
    TSPPath *seedPath = instance->getBitonicPath();
    double seedDist = instance->evaluatePath(seedPath);
    TSPPathDistPair bestPath = {seedDist, seedPath};
    TSPPathDistPair candidatePath, newCandidatePath;
    report(bestPath);
    while(instance->isFitnessAvailable()) {
        while(instance->nextCandidateAvailable()) {
            candidatePath = instance->getNextCandidate();
            instance->pushCandidate(candidatePath);

            TSPPath *originalPath = candidatePath.path;

            //mutate uppath only
            TSPPath *newPath = nullptr;
            newPath = originalPath->mutateUppath();
            double newDist = instance->evaluatePath(newPath);
            if (newDist == -1) break;
            if (newDist < bestPath.dist) {
                bestPath = {newDist, newPath};
                report(bestPath);
            }
            if (annealer->P(newDist, candidatePath.dist)) {
                newCandidatePath.dist = newDist;
                newCandidatePath.path = newPath;
                instance->pushCandidate(newCandidatePath);
            } else {
                delete newPath;
            }

            //mutate downpath only
            newPath = originalPath->mutateDownpath();
            double newDist = instance->evaluatePath(newPath);
            if (newDist == -1) break;
            if (newDist < bestPath.dist) {
                bestPath = {newDist, newPath};
                report(bestPath);
            }
            if (annealer->P(newDist, candidatePath.dist)) {
                newCandidatePath.dist = newDist;
                newCandidatePath.path = newPath;
                instance->pushCandidate(newCandidatePath);
            } else {
                delete newPath;
            }

            newPath = originalPath->crossoverPath();
            double newDist = instance->evaluatePath(newPath);
            if (newDist == -1) break;
            if (newDist < bestPath.dist) {
                bestPath = {newDist, newPath};
                report(bestPath);
            }
            if (annealer->P(newDist, candidatePath.dist)) {
                newCandidatePath.dist = newDist;
                newCandidatePath.path = newPath;
                instance->pushCandidate(newCandidatePath);
            } else {
                delete newPath;
            }
        }
        instance->managePopulation();
        annealer->cool();
    }
    return 0;
}