#include "genetic_algorithm.h"

#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <set>
#include <utility>
#include <algorithm>
#include <cmath>

using namespace std;

// PARAMETRY GLOBALNE
mt19937 generator(42);

int POPULATION_SIZE = 200;
int GENERATIONS = 500;

double MUTATION_RATE = 0.08;
double RANDOM_RESET_RATE = 0.15;

int TOURNAMENT_SIZE = 7;

int MAX_WEIGHT = 150;
int MAX_VOLUME = 200;

int SAME_FITNESS_LIMIT = 50;

// OCENA OSOBNIKA
static Result evaluateIndividual(const vector<int>& individual, const vector<Item>& items)
{
    Result result;
    result.valueSum = 0;
    result.weightSum = 0;
    result.volumeSum = 0;

    for (int i = 0; i < static_cast<int>(individual.size()); i++)
    {
        if (individual[i] == 1)
        {
            result.valueSum += items[i].value;
            result.weightSum += items[i].weight;
            result.volumeSum += items[i].volume;
        }
    }

    return result;
}

static bool isCorrect(const vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    return result.weightSum <= MAX_WEIGHT && result.volumeSum <= MAX_VOLUME;
}

static double fitness(const vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    int extraWeight = max(0, result.weightSum - MAX_WEIGHT);
    int extraVolume = max(0, result.volumeSum - MAX_VOLUME);

    if (extraWeight > 0 || extraVolume > 0)
    {
        return -10000.0 - 100.0 * (extraWeight + extraVolume);
    }

    return static_cast<double>(result.valueSum);
}

// TWORZENIE POPULACJI
static vector<int> createIndividual(int numItems)
{
    vector<int> individual;
    individual.reserve(numItems);

    uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < numItems; i++)
    {
        individual.push_back(dist(generator) < 0.08 ? 1 : 0);
    }

    return individual;
}

static vector<vector<int>> createPopulation(int numItems, int populationSize)
{
    vector<vector<int>> population;
    population.reserve(populationSize);

    for (int i = 0; i < populationSize; i++)
    {
        population.push_back(createIndividual(numItems));
    }

    return population;
}

// NAPRAWA OSOBNIKA
static void repairIndividual(vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    while (result.weightSum > MAX_WEIGHT || result.volumeSum > MAX_VOLUME)
    {
        vector<int> selectedIndices;

        for (int i = 0; i < static_cast<int>(individual.size()); i++)
        {
            if (individual[i] == 1)
            {
                selectedIndices.push_back(i);
            }
        }

        if (selectedIndices.empty())
            break;

        uniform_int_distribution<int> selectedDist(0, static_cast<int>(selectedIndices.size()) - 1);
        int removeIndex = selectedIndices[selectedDist(generator)];

        individual[removeIndex] = 0;
        result = evaluateIndividual(individual, items);
    }
}

// SELEKCJA TURNIEJOWA
static int tournamentSelectionIndex(const vector<vector<int>>& population, const vector<Item>& items)
{
    uniform_int_distribution<int> indexDist(0, static_cast<int>(population.size()) - 1);

    int bestIndex = indexDist(generator);
    double bestFitness = fitness(population[bestIndex], items);

    for (int i = 1; i < TOURNAMENT_SIZE; i++)
    {
        int candidateIndex = indexDist(generator);
        double candidateFitness = fitness(population[candidateIndex], items);

        if (candidateFitness > bestFitness)
        {
            bestIndex = candidateIndex;
            bestFitness = candidateFitness;
        }
    }

    return bestIndex;
}

// KRZYŻOWANIA
static vector<int> uniformCrossover(const vector<int>& parent1, const vector<int>& parent2)
{
    vector<int> child;
    child.reserve(parent1.size());

    uniform_int_distribution<int> coin(0, 1);

    for (int i = 0; i < static_cast<int>(parent1.size()); i++)
    {
        if (coin(generator) == 0)
            child.push_back(parent1[i]);
        else
            child.push_back(parent2[i]);
    }

    return child;
}

static vector<int> twoPointCrossover(const vector<int>& parent1, const vector<int>& parent2)
{
    vector<int> child = parent1;

    uniform_int_distribution<int> indexDist(0, static_cast<int>(parent1.size()) - 1);
    int point1 = indexDist(generator);
    int point2 = indexDist(generator);

    while (point1 == point2)
    {
        point2 = indexDist(generator);
    }

    if (point1 > point2)
        swap(point1, point2);

    for (int i = point1; i <= point2; i++)
    {
        child[i] = parent2[i];
    }

    return child;
}

static vector<int> crossoverMix(const vector<int>& parent1, const vector<int>& parent2)
{
    uniform_int_distribution<int> methodDist(0, 1);

    if (methodDist(generator) == 0)
        return uniformCrossover(parent1, parent2);
    else
        return twoPointCrossover(parent1, parent2);
}

// MUTACJE
static void mutate(vector<int>& individual)
{
    uniform_real_distribution<double> prob(0.0, 1.0);

    for (int i = 0; i < static_cast<int>(individual.size()); i++)
    {
        if (prob(generator) < MUTATION_RATE)
        {
            individual[i] = 1 - individual[i];
        }
    }
}

static void randomResetMutation(vector<int>& individual)
{
    uniform_real_distribution<double> prob(0.0, 1.0);

    if (prob(generator) < RANDOM_RESET_RATE)
    {
        uniform_int_distribution<int> indexDist(0, static_cast<int>(individual.size()) - 1);
        uniform_int_distribution<int> bitDist(0, 1);

        int idx = indexDist(generator);
        individual[idx] = bitDist(generator);
    }
}

// SPRAWDZANIE DUPLIKATÓW DZIECI
static bool existsInPopulation(const vector<vector<int>>& population, const vector<int>& individual)
{
    for (int i = 0; i < static_cast<int>(population.size()); i++)
    {
        if (population[i] == individual)
            return true;
    }

    return false;
}

// ELITYZM
static int getBestEliteIndex(const vector<vector<int>>& population, const vector<Item>& items)
{
    int bestCorrectIndex = -1;
    double bestCorrectFitness = -1e18;

    int bestAnyIndex = 0;
    double bestAnyFitness = fitness(population[0], items);

    for (int i = 0; i < static_cast<int>(population.size()); i++)
    {
        double currentFitness = fitness(population[i], items);

        if (currentFitness > bestAnyFitness)
        {
            bestAnyFitness = currentFitness;
            bestAnyIndex = i;
        }

        if (isCorrect(population[i], items) && currentFitness > bestCorrectFitness)
        {
            bestCorrectFitness = currentFitness;
            bestCorrectIndex = i;
        }
    }

    if (bestCorrectIndex != -1)
        return bestCorrectIndex;

    return bestAnyIndex;
}

// WYPISYWANIE
void printItems(const vector<Item>& items)
{
    cout << "PRZEDMIOTY:\n";
    cout << "ID  Value  Weight  Volume\n";

    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        cout << i << "   "
             << items[i].value << "      "
             << items[i].weight << "       "
             << items[i].volume << "\n";
    }

    cout << "\n";
}

static void printBestSolution(const vector<int>& best, const vector<Item>& items)
{
    Result result = evaluateIndividual(best, items);

    cout << "\nNAJLEPSZE ROZWIAZANIE:\n";
    cout << "Chromosom: ";

    for (int i = 0; i < static_cast<int>(best.size()); i++)
    {
        cout << best[i] << " ";
    }

    cout << "\nWybrane przedmioty: ";

    for (int i = 0; i < static_cast<int>(best.size()); i++)
    {
        if (best[i] == 1)
            cout << i << " ";
    }

    cout << "\nSuma wartosci: " << result.valueSum;
    cout << "\nSuma wagi: " << result.weightSum << " / " << MAX_WEIGHT;
    cout << "\nSuma objetosci: " << result.volumeSum << " / " << MAX_VOLUME;

    if (isCorrect(best, items))
        cout << "\nRozwiazanie poprawne: TAK\n";
    else
        cout << "\nRozwiazanie poprawne: NIE\n";
}

// GŁÓWNA LOGIKA ALGORYTMU
AlgorithmStats runAlgorithm(const vector<Item>& items, bool printLogs, const GAConfig& config)
{
    int numItems = static_cast<int>(items.size());

    vector<vector<int>> population = createPopulation(numItems, config.populationSize);

    for (int i = 0; i < static_cast<int>(population.size()); i++)
    {
        repairIndividual(population[i], items);
    }

    vector<int> bestSolution;
    double bestFitness = -1e18;

    double lastBestFitness = -1e18;
    int sameFitnessCounter = 0;
    int generationsUsed = 0;

    if (printLogs)
        printItems(items);

    for (int generation = 0; generation < GENERATIONS; generation++)
    {
        generationsUsed = generation + 1;

        vector<vector<int>> newPopulation;
        newPopulation.reserve(config.populationSize);

        int eliteIndex = getBestEliteIndex(population, items);
        newPopulation.push_back(population[eliteIndex]);

        set<pair<int, int>> usedPairs;

        int attempts = 0;
        int maxAttempts = 1000;

        while (static_cast<int>(newPopulation.size()) < config.populationSize && attempts < maxAttempts)
        {
            attempts++;

            int idx1 = tournamentSelectionIndex(population, items);
            int idx2 = tournamentSelectionIndex(population, items);

            while (idx1 == idx2)
            {
                idx2 = tournamentSelectionIndex(population, items);
            }

            pair<int, int> parentPair = {min(idx1, idx2), max(idx1, idx2)};

            if (usedPairs.count(parentPair) > 0)
                continue;

            usedPairs.insert(parentPair);

            vector<int> child;

            if (config.crossoverType == UNIFORM)
                child = uniformCrossover(population[idx1], population[idx2]);
            else
                child = twoPointCrossover(population[idx1], population[idx2]);
            if (config.mutationType == BIT_FLIP)
                mutate(child);
            else
                randomResetMutation(child);
            repairIndividual(child, items);

            if (!existsInPopulation(newPopulation, child))
                newPopulation.push_back(child);
        }

        int extraAttempts = 0;
        int maxExtraAttempts = 5000;

        while (static_cast<int>(newPopulation.size()) < config.populationSize && extraAttempts < maxExtraAttempts)
        {
            extraAttempts++;

            int idx1 = tournamentSelectionIndex(population, items);
            int idx2 = tournamentSelectionIndex(population, items);

            while (idx1 == idx2)
            {
                idx2 = tournamentSelectionIndex(population, items);
            }

            vector<int> child;

            if (config.crossoverType == UNIFORM)
                child = uniformCrossover(population[idx1], population[idx2]);
            else
                child = twoPointCrossover(population[idx1], population[idx2]);
            if (config.mutationType == BIT_FLIP)
                mutate(child);
            else
                randomResetMutation(child);
            repairIndividual(child, items);

            if (!existsInPopulation(newPopulation, child))
                newPopulation.push_back(child);
        }

        while (static_cast<int>(newPopulation.size()) < config.populationSize)
        {
            int idx1 = tournamentSelectionIndex(population, items);
            int idx2 = tournamentSelectionIndex(population, items);

            while (idx1 == idx2)
            {
                idx2 = tournamentSelectionIndex(population, items);
            }

            vector<int> child;

            if (config.crossoverType == UNIFORM)
                child = uniformCrossover(population[idx1], population[idx2]);
            else
                child = twoPointCrossover(population[idx1], population[idx2]);
            if (config.mutationType == BIT_FLIP)
                mutate(child);
            else
                randomResetMutation(child);
            repairIndividual(child, items);

            newPopulation.push_back(child);
        }

        population = newPopulation;

        for (int i = 0; i < static_cast<int>(population.size()); i++)
        {
            double currentFitness = fitness(population[i], items);

            if (isCorrect(population[i], items) && currentFitness > bestFitness)
            {
                bestFitness = currentFitness;
                bestSolution = population[i];
            }
        }

        if (!bestSolution.empty())
        {
            if (fabs(bestFitness - lastBestFitness) < 1e-9)
                sameFitnessCounter++;
            else
                sameFitnessCounter = 0;

            lastBestFitness = bestFitness;
        }

        if (printLogs)
        {
            if (bestSolution.empty())
            {
                cout << "Generacja " << generation + 1
                     << " | brak poprawnego rozwiazania\n";
            }
            else
            {
                cout << "Generacja " << generation + 1
                     << " | najlepszy fitness = " << bestFitness
                     << " | ten sam wynik od " << sameFitnessCounter
                     << " generacji\n";
            }
        }

        if (!bestSolution.empty() && sameFitnessCounter >= SAME_FITNESS_LIMIT)
        {
            if (printLogs)
            {
                cout << "\nNajlepszy wynik powtarza sie od "
                     << SAME_FITNESS_LIMIT
                     << " generacji. Koniec algorytmu.\n";
            }
            break;
        }
    }

    AlgorithmStats stats{};

    if (bestSolution.empty())
    {
        stats.foundCorrect = false;
        stats.bestFitness = -1e18;
        stats.bestValue = 0;
        stats.bestWeight = 0;
        stats.bestVolume = 0;
        stats.generationsUsed = generationsUsed;

        if (printLogs)
            cout << "\nNie znaleziono poprawnego rozwiazania.\n";
    }
    else
    {
        Result result = evaluateIndividual(bestSolution, items);

        stats.foundCorrect = true;
        stats.bestFitness = bestFitness;
        stats.bestValue = result.valueSum;
        stats.bestWeight = result.weightSum;
        stats.bestVolume = result.volumeSum;
        stats.generationsUsed = generationsUsed;

        if (printLogs)
        {
            cout << "\nNajlepsze znalezione rozwiazanie:\n";
            printBestSolution(bestSolution, items);
        }
    }

    return stats;
}
