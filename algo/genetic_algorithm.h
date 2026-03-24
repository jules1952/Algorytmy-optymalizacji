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
extern int NUM_ITEMS;
extern int POPULATION_SIZE;
extern int GENERATIONS;

extern double MUTATION_RATE;
extern double RANDOM_RESET_RATE;

extern int TOURNAMENT_SIZE;
extern double PENALTY;

extern int MAX_WEIGHT;
extern int MAX_VOLUME;

extern int SAME_FITNESS_LIMIT;

// Funkcje
std::vector<Item> generateItems();
void printItems(const std::vector<Item>& items);
void runAlgorithm(const std::vector<Item>& items);

#endif