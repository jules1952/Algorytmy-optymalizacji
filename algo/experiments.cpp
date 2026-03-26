#include "genetic_algorithm.h"
#include "csv_utils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

int main()
{
    string filename;
    cout << "Podaj nazwe pliku CSV do badan: ";
    cin >> filename;

    vector<Item> items = loadItemsFromCSV(filename);

    if (items.empty())
    {
        cout << "Nie udalo sie wczytac danych.\n";
        return 1;
    }

    const int RUNS = 100;

    ofstream details("results_details.csv");
    if (!details)
    {
        cout << "Nie mozna utworzyc results_details.csv\n";
        return 1;
    }

    details << "run,found_correct,best_fitness,best_value,best_weight,best_volume,generations_used\n";

    double sumFitness = 0.0;
    double minFitness = numeric_limits<double>::max();
    double maxFitness = numeric_limits<double>::lowest();

    double sumValue = 0.0;
    int minValue = numeric_limits<int>::max();
    int maxValue = numeric_limits<int>::lowest();

    double sumGenerations = 0.0;
    int successCount = 0;

    for (int run = 1; run <= RUNS; run++)
    {
        AlgorithmStats stats = runAlgorithm(items, false);

        details << run << ","
                << (stats.foundCorrect ? 1 : 0) << ","
                << stats.bestFitness << ","
                << stats.bestValue << ","
                << stats.bestWeight << ","
                << stats.bestVolume << ","
                << stats.generationsUsed << "\n";

        if (stats.foundCorrect)
        {
            successCount++;
            sumFitness += stats.bestFitness;
            sumValue += stats.bestValue;
            sumGenerations += stats.generationsUsed;

            if (stats.bestFitness < minFitness) minFitness = stats.bestFitness;
            if (stats.bestFitness > maxFitness) maxFitness = stats.bestFitness;

            if (stats.bestValue < minValue) minValue = stats.bestValue;
            if (stats.bestValue > maxValue) maxValue = stats.bestValue;
        }

        cout << "Run " << run << "/" << RUNS << " zakonczony\n";
    }

    details.close();

    ofstream summary("results_summary.csv");
    if (!summary)
    {
        cout << "Nie mozna utworzyc results_summary.csv\n";
        return 1;
    }

    summary << "file_name,runs,success_count,success_rate,avg_fitness,min_fitness,max_fitness,avg_value,min_value,max_value,avg_generations\n";

    if (successCount > 0)
    {
        summary << filename << ","
                << RUNS << ","
                << successCount << ","
                << (100.0 * successCount / RUNS) << ","
                << (sumFitness / successCount) << ","
                << minFitness << ","
                << maxFitness << ","
                << (sumValue / successCount) << ","
                << minValue << ","
                << maxValue << ","
                << (sumGenerations / successCount) << "\n";
    }
    else
    {
        summary << filename << ","
                << RUNS << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << ","
                << 0 << "\n";
    }

    summary.close();

    ofstream report("results_report.txt");
    if (!report)
    {
        cout << "Nie mozna utworzyc results_report.txt\n";
        return 1;
    }

    report << "file_name: " << filename << "\n";
    report << "runs: " << RUNS << "\n";
    report << "success_count: " << successCount << "\n";
    report << "success_rate: " << (100.0 * successCount / RUNS) << "\n";

    if (successCount > 0)
    {
        report << "avg_fitness: " << (sumFitness / successCount) << "\n";
        report << "min_fitness: " << minFitness << "\n";
        report << "max_fitness: " << maxFitness << "\n";
        report << "avg_value: " << (sumValue / successCount) << "\n";
        report << "min_value: " << minValue << "\n";
        report << "max_value: " << maxValue << "\n";
        report << "avg_generations: " << (sumGenerations / successCount) << "\n";
    }
    else
    {
        report << "avg_fitness: 0\n";
        report << "min_fitness: 0\n";
        report << "max_fitness: 0\n";
        report << "avg_value: 0\n";
        report << "min_value: 0\n";
        report << "max_value: 0\n";
        report << "avg_generations: 0\n";
    }

    report.close();

    ofstream pretty("results_pretty.csv");
    if (!pretty)
    {
        cout << "Nie mozna utworzyc results_pretty.csv\n";
        return 1;
    }

    pretty << "metric,value\n";
    pretty << "file_name," << filename << "\n";
    pretty << "runs," << RUNS << "\n";
    pretty << "success_count," << successCount << "\n";
    pretty << "success_rate," << (100.0 * successCount / RUNS) << "\n";

    if (successCount > 0)
    {
        pretty << "avg_fitness," << (sumFitness / successCount) << "\n";
        pretty << "min_fitness," << minFitness << "\n";
        pretty << "max_fitness," << maxFitness << "\n";
        pretty << "avg_value," << (sumValue / successCount) << "\n";
        pretty << "min_value," << minValue << "\n";
        pretty << "max_value," << maxValue << "\n";
        pretty << "avg_generations," << (sumGenerations / successCount) << "\n";
    }
    else
    {
        pretty << "avg_fitness,0\n";
        pretty << "min_fitness,0\n";
        pretty << "max_fitness,0\n";
        pretty << "avg_value,0\n";
        pretty << "min_value,0\n";
        pretty << "max_value,0\n";
        pretty << "avg_generations,0\n";
    }

    pretty.close();

    cout << "\nZapisano:\n";
    cout << "- results_details.csv\n";
    cout << "- results_summary.csv\n";
    cout << "- results_report.txt\n";
    cout << "- results_pretty.csv\n";

    return 0;
}