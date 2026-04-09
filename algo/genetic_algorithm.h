#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <vector>
#include <string>
struct Item
{
    int value;
    int weight;
    int volume;
};

struct Result
{
    int valueSum;
    int weightSum;
    int volumeSum;
};

struct AlgorithmStats
{
    bool foundCorrect;
    double bestFitness;
    int bestValue;
    int bestWeight;
    int bestVolume;
    int generationsUsed;
};

enum CrossoverType {
    UNIFORM,
    TWO_POINT
};

enum MutationType {
    BIT_FLIP,
    RANDOM_RESET
};

struct GAConfig {
    int populationSize;
    CrossoverType crossoverType;
    MutationType mutationType;
};

// Parametry algorytmu
extern int POPULATION_SIZE;
extern int GENERATIONS;

extern double MUTATION_RATE;
extern double RANDOM_RESET_RATE;

extern int TOURNAMENT_SIZE;

extern int MAX_WEIGHT;
extern int MAX_VOLUME;

extern int SAME_FITNESS_LIMIT;

// Funkcje
void printItems(const std::vector<Item>& items);
AlgorithmStats runAlgorithm(const std::vector<Item>& items, bool printLogs, const GAConfig& config);
#endif
