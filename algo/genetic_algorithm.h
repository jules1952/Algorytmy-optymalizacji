#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include <vector>

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
void runAlgorithm(const std::vector<Item>& items);

#endif