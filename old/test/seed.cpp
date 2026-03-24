#include <iostream>
#include <vector>
#include <random>
#include <ctime>

using namespace std;

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

mt19937 generator(42);
int NUM_ITEMS = 70;
int POPULATION_SIZE = 40;
int GENERATIONS = 100;
double MUTATION_RATE = 0.05;
int TOURNAMENT_SIZE = 3;
double PENALTY = 10.0;

int MAX_WEIGHT = 30;
int MAX_VOLUME = 40;

// Tworzenie losowych przedmiotów
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
// Tworzenie jednego losowego osobnika
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
// Tworzenie całej populacji
vector<vector<int>> createPopulation()
{
    vector<vector<int>> population;

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        population.push_back(createIndividual());
    }

    return population;
}
// Obliczanie sum dla jednego rozwiązania
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

// Funkcja fitness
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

// Selekcja turniejowa
vector<int> tournamentSelection(const vector<vector<int>>& population, const vector<Item>& items)
{
    uniform_int_distribution<int> indexDist(0, POPULATION_SIZE - 1);

    vector<int> best = population[indexDist(generator)];
    double bestFitness = fitness(best, items);

    for (int i = 1; i < TOURNAMENT_SIZE; i++)
    {
        vector<int> candidate = population[indexDist(generator)];
        double candidateFitness = fitness(candidate, items);

        if (candidateFitness > bestFitness)
        {
            best = candidate;
            bestFitness = candidateFitness;
        }
    }

    return best;
}

// Krzyżowanie uniform
vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2)
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

// Mutacja
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

// Sprawdzenie poprawności rozwiązania
bool isCorrect(const vector<int>& individual, const vector<Item>& items)
{
    Result result = evaluateIndividual(individual, items);

    return result.weightSum <= MAX_WEIGHT && result.volumeSum <= MAX_VOLUME;
}

// Wypisanie przedmiotów
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

// Wypisanie rozwiązania
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

// Główna część programu
int main()
{
    vector<Item> items = generateItems();
    vector<vector<int>> population = createPopulation();

    vector<int> bestSolution;
    double bestFitness = -1000000.0;

    printItems(items);

    for (int generation = 0; generation < GENERATIONS; generation++)
    {
        vector<vector<int>> newPopulation;

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            vector<int> parent1 = tournamentSelection(population, items);
            vector<int> parent2 = tournamentSelection(population, items);

            vector<int> child = crossover(parent1, parent2);
            mutate(child);

            newPopulation.push_back(child);
        }

        population = newPopulation;

        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            double currentFitness = fitness(population[i], items);

            if (isCorrect(population[i], items) && currentFitness > bestFitness)
            {
                bestFitness = currentFitness;
                bestSolution = population[i];
            }
        }

        if (bestFitness == -1000000.0)
        {
            cout << "Generacja " << generation + 1
                 << " | jeszcze brak poprawnego rozwiazania\n";
        }
        else
        {
            cout << "Generacja " << generation + 1
                 << " | najlepszy fitness = " << bestFitness << "\n";
        }
    }

    if (bestSolution.empty())
    {
        cout << "\nNie znaleziono poprawnego rozwiazania.\n";
    }
    else
    {
        printBestSolution(bestSolution, items);
    }

    return 0;
}