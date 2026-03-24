#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <set>
#include <utility>
#include <algorithm>

using namespace std;

// ===========================
// STRUKTURY DANYCH
// ===========================
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

// Profesjonalny generator liczb losowych
mt19937 generator(time(0));

int NUM_ITEMS = 70;
int POPULATION_SIZE = 40;
int GENERATIONS = 100;

double MUTATION_RATE = 0.05;       // mutacja bitowa
double RANDOM_RESET_RATE = 0.10;   // random reset mutation

int TOURNAMENT_SIZE = 3;
double PENALTY = 10.0;

int MAX_WEIGHT = 30;
int MAX_VOLUME = 40;

int SAME_FITNESS_LIMIT = 10; // warunek stopu: ten sam najlepszy wynik powtarza sie tyle razy

// ===========================
// TWORZENIE LOSOWYCH PRZEDMIOTÓW
// ===========================
vector<Item> generateItems()
{
    vector<Item> items;

    uniform_int_distribution<int> valueDist(10, 50);
    uniform_int_distribution<int> weightDist(1, 10);
    uniform_int_distribution<int> volumeDist(1, 8);

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        Item item;
        item.value = valueDist(generator);
        item.weight = weightDist(generator);
        item.volume = volumeDist(generator);

        items.push_back(item);
    }

    return items;
}

// ===========================
// TWORZENIE POPULACJI
// ===========================
vector<int> createIndividual()
{
    vector<int> individual;
    uniform_int_distribution<int> bitDist(0, 1);

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        individual.push_back(bitDist(generator));
    }

    return individual;
}

vector<vector<int>> createPopulation()
{
    vector<vector<int>> population;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        population.push_back(createIndividual());
    }

    return population;
}

// ===========================
// OCENA OSOBNIKA
// ===========================
Result evaluateIndividual(const vector<int>& individual, const vector<Item>& items)
{
    Result result;
    result.valueSum = 0;
    result.weightSum = 0;
    result.volumeSum = 0;

    for (int i = 0; i < NUM_ITEMS; i++)
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

double fitness(const vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    int extraWeight = 0;
    int extraVolume = 0;

    if (result.weightSum > MAX_WEIGHT)
        extraWeight = result.weightSum - MAX_WEIGHT;

    if (result.volumeSum > MAX_VOLUME)
        extraVolume = result.volumeSum - MAX_VOLUME;

    double penaltyValue = PENALTY * (extraWeight + extraVolume);

    return result.valueSum - penaltyValue;
}

bool isCorrect(const vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    return result.weightSum <= MAX_WEIGHT && result.volumeSum <= MAX_VOLUME;
}

// ===========================
// SELEKCJA TURNIEJOWA
// ===========================
int tournamentSelectionIndex(const vector<vector<int>>& population, const vector<Item>& items)
{
    uniform_int_distribution<int> indexDist(0, POPULATION_SIZE - 1);

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

// ===========================
// KRZYŻOWANIA
// ===========================
vector<int> uniformCrossover(const vector<int>& parent1, const vector<int>& parent2)
{
    vector<int> child;
    uniform_int_distribution<int> coin(0, 1);

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        if (coin(generator) == 0)
            child.push_back(parent1[i]);
        else
            child.push_back(parent2[i]);
    }

    return child;
}

vector<int> twoPointCrossover(const vector<int>& parent1, const vector<int>& parent2)
{
    vector<int> child = parent1;

    uniform_int_distribution<int> indexDist(0, NUM_ITEMS - 1);
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

vector<int> crossoverMix(const vector<int>& parent1, const vector<int>& parent2)
{
    uniform_int_distribution<int> methodDist(0, 1);

    if (methodDist(generator) == 0)
        return uniformCrossover(parent1, parent2);
    else
        return twoPointCrossover(parent1, parent2);
}

// ===========================
// MUTACJE
// ===========================
void mutate(vector<int>& individual)
{
    uniform_real_distribution<double> prob(0.0, 1.0);

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        if (prob(generator) < MUTATION_RATE)
        {
            if (individual[i] == 0)
                individual[i] = 1;
            else
                individual[i] = 0;
        }
    }
}

void randomResetMutation(vector<int>& individual)
{
    uniform_real_distribution<double> prob(0.0, 1.0);

    if (prob(generator) < RANDOM_RESET_RATE)
    {
        uniform_int_distribution<int> indexDist(0, NUM_ITEMS - 1);
        uniform_int_distribution<int> bitDist(0, 1);

        int idx = indexDist(generator);
        individual[idx] = bitDist(generator);
    }
}

// ===========================
// SPRAWDZANIE DUPLIKATÓW DZIECI
// ===========================
bool existsInPopulation(const vector<vector<int>>& population, const vector<int>& individual)
{
    for (int i = 0; i < population.size(); i++)
    {
        if (population[i] == individual)
            return true;
    }

    return false;
}

// ===========================
// LEPSZY ELITYZM
// Jeśli istnieje poprawny osobnik, wybierz najlepszy poprawny.
// Jeśli nie istnieje żaden poprawny, wybierz najlepszy ogólnie.
// ===========================
int getBestEliteIndex(const vector<vector<int>>& population, const vector<Item>& items)
{
    int bestCorrectIndex = -1;
    double bestCorrectFitness = -1000000.0;

    int bestAnyIndex = 0;
    double bestAnyFitness = fitness(population[0], items);

    for (int i = 0; i < population.size(); i++)
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

// ===========================
// WYPISYWANIE
// ===========================
void printItems(const vector<Item>& items)
{
    cout << "PRZEDMIOTY:\n";
    cout << "ID  Value  Weight  Volume\n";

    for (int i = 0; i < items.size(); i++)
    {
        cout << i << "   "
             << items[i].value << "      "
             << items[i].weight << "       "
             << items[i].volume << "\n";
    }

    cout << "\n";
}

void printBestSolution(const vector<int>& best, const vector<Item>& items)
{
    Result result = evaluateIndividual(best, items);

    cout << "\nNAJLEPSZE ROZWIAZANIE:\n";
    cout << "Chromosom: ";

    for (int i = 0; i < best.size(); i++)
    {
        cout << best[i] << " ";
    }

    cout << "\nWybrane przedmioty: ";

    for (int i = 0; i < best.size(); i++)
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

// ===========================
// GŁÓWNA CZĘŚĆ PROGRAMU
// ===========================
int main()
{
    vector<Item> items = generateItems();
    vector<vector<int>> population = createPopulation();

    vector<int> bestSolution;
    double bestFitness = -1000000.0;

    double lastBestFitness = -1000000.0;
    int sameFitnessCounter = 0;

    printItems(items);

    for (int generation = 0; generation < GENERATIONS; generation++)
    {
        vector<vector<int>> newPopulation;

        // ===========================
        // LEPSZY ELITYZM
        // ===========================
        int eliteIndex = getBestEliteIndex(population, items);
        newPopulation.push_back(population[eliteIndex]);

        // ===========================
        // ZABEZPIECZENIE PRZED POWTARZANIEM PAR
        // ===========================
        set<pair<int, int>> usedPairs;

        int attempts = 0;
        int maxAttempts = 1000;

        // ===========================
        // TWORZENIE RESZTY NOWEJ POPULACJI
        // ===========================
        while (newPopulation.size() < POPULATION_SIZE && attempts < maxAttempts)
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
            {
                continue;
            }

            usedPairs.insert(parentPair);

            vector<int> parent1 = population[idx1];
            vector<int> parent2 = population[idx2];

            vector<int> child = crossoverMix(parent1, parent2);

            mutate(child);
            randomResetMutation(child);

            if (!existsInPopulation(newPopulation, child))
            {
                newPopulation.push_back(child);
            }
        }

        // ===========================
        // UZUPEŁNIANIE POPULACJI
        // najpierw próbujemy bez duplikatów, a jeśli dalej się nie da,
        // to na końcu pozwalamy na duplikaty, żeby uniknąć zapętlenia
        // ===========================
        int extraAttempts = 0;
        int maxExtraAttempts = 5000;

        while (newPopulation.size() < POPULATION_SIZE && extraAttempts < maxExtraAttempts)
        {
            extraAttempts++;

            int idx1 = tournamentSelectionIndex(population, items);
            int idx2 = tournamentSelectionIndex(population, items);

            while (idx1 == idx2)
            {
                idx2 = tournamentSelectionIndex(population, items);
            }

            vector<int> child = crossoverMix(population[idx1], population[idx2]);
            mutate(child);
            randomResetMutation(child);

            if (!existsInPopulation(newPopulation, child))
            {
                newPopulation.push_back(child);
            }
        }

        while (newPopulation.size() < POPULATION_SIZE)
        {
            int idx1 = tournamentSelectionIndex(population, items);
            int idx2 = tournamentSelectionIndex(population, items);

            while (idx1 == idx2)
            {
                idx2 = tournamentSelectionIndex(population, items);
            }

            vector<int> child = crossoverMix(population[idx1], population[idx2]);
            mutate(child);
            randomResetMutation(child);

            newPopulation.push_back(child);
        }

        population = newPopulation;

        // ===========================
        // SPRAWDZENIE POPRAWY
        // ===========================
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            double currentFitness = fitness(population[i], items);

            if (isCorrect(population[i], items) && currentFitness > bestFitness)
            {
                bestFitness = currentFitness;
                bestSolution = population[i];
            }
        }

        // ===========================
        // SPRAWDZENIE CZY TEN SAM WYNIK SIĘ POWTARZA
        // ===========================
        if (!bestSolution.empty())
        {
            if (bestFitness == lastBestFitness)
                sameFitnessCounter++;
            else
                sameFitnessCounter = 0;

            lastBestFitness = bestFitness;
        }

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

        // ===========================
        // WARUNEK STOPU
        // ===========================
        if (!bestSolution.empty() && sameFitnessCounter >= SAME_FITNESS_LIMIT)
        {
            cout << "\nNajlepszy wynik powtarza sie od "
                 << SAME_FITNESS_LIMIT
                 << " generacji. Koniec algorytmu.\n";
            break;
        }
    }

    if (bestSolution.empty())
    {
        cout << "\nNie znaleziono poprawnego rozwiazania.\n";
    }
    else
    {
        cout << "\nNajlepsze znalezione rozwiazanie:\n";
        printBestSolution(bestSolution, items);
    }

    return 0;
}