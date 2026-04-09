#include "genetic_algorithm.h"
#include "csv_utils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <iomanip>

using namespace std;

// Funkcje pomocnicze do formatowania nazw w CSV
string crossoverToStr(CrossoverType t) {
    return t == UNIFORM ? "uniform" : "two_point";
}

string mutationToStr(MutationType t) {
    return t == BIT_FLIP ? "bit_flip" : "random_reset";
}

int main() {
    // Lista plików wygenerowanych przez generate_data.cpp
    vector<string> testFiles = {
        "items_70.csv",
        "items_100.csv",
        "items_150.csv",
        "items_200.csv",
        "items_500.csv"
    };

    const int RUNS = 100; // Liczba powtórzeń dla każdego testu
    vector<int> populations = {50, 100, 200, 500}; // Eksperyment 2
    vector<CrossoverType> crossovers = {UNIFORM, TWO_POINT}; // Eksperyment 1
    vector<MutationType> mutations = {BIT_FLIP, RANDOM_RESET}; // Eksperyment 1

    // Plik z surowymi danymi (wszystkie 100 przebiegów)
    ofstream details("results_details.csv");
    // Plik ze średnimi (idealny do tabel w LaTeX)
    ofstream summary("results_summary.csv");

    if (!details || !summary) {
        cout << "Blad tworzenia plikow wynikowych!\n";
        return 1;
    }

    // Nagłówki plików CSV
    details << "file,population,crossover,mutation,run,found_correct,best_value,generations_used\n";
    summary << "file,population,crossover,mutation,success_rate,avg_value,avg_generations,max_value\n";

    for (const string& filename : testFiles) {
        vector<Item> items = loadItemsFromCSV(filename);
        if (items.empty()) continue;

        cout << "\n>>> ANALIZA PLIKU: " << filename << " <<<\n";

        for (int pop : populations) {
            for (auto cross : crossovers) {
                for (auto mut : mutations) {

                    cout << "Test: Pop=" << pop << " Cross=" << crossoverToStr(cross) << " Mut=" << mutationToStr(mut) << endl;

                    double totalValue = 0.0;
                    double totalGenerations = 0.0;
                    int successCount = 0;
                    int maxVal = 0;

                    GAConfig config;
                    config.populationSize = pop;
                    config.crossoverType = cross;
                    config.mutationType = mut;

                    for (int run = 1; run <= RUNS; run++) {
                        AlgorithmStats stats = runAlgorithm(items, false, config);

                        // Zapis surowych danych
                        details << filename << "," << pop << "," << crossoverToStr(cross) << ","
                                << mutationToStr(mut) << "," << run << ","
                                << (stats.foundCorrect ? 1 : 0) << "," << stats.bestValue << ","
                                << stats.generationsUsed << "\n";

                        if (stats.foundCorrect) {
                            successCount++;
                            totalValue += stats.bestValue;
                            totalGenerations += stats.generationsUsed;
                            if (stats.bestValue > maxVal) maxVal = stats.bestValue;
                        }

                        if (run % 10 == 0) cout << "Progress: " << run << "%\r" << flush;
                    }

                    // Obliczanie średnich i zapis do podsumowania
                    double successRate = (double)successCount / RUNS * 100.0;
                    double avgValue = (successCount > 0) ? totalValue / successCount : 0;
                    double avgGens = (successCount > 0) ? totalGenerations / successCount : 0;

                    summary << filename << "," << pop << "," << crossoverToStr(cross) << ","
                            << mutationToStr(mut) << "," << fixed << setprecision(2)
                            << successRate << "," << avgValue << "," << avgGens << "," << maxVal << "\n";
                }
            }
        }
    }

    details.close();
    summary.close();

    cout << "\nZakonczono! Wyniki zapisano w results_summary.csv i results_details.csv\n";
    return 0;
}
