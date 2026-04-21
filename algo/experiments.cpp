#include "genetic_algorithm.h"
#include "csv_utils.h"
#include <omp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <mutex>

using namespace std;
using namespace chrono;

std::mutex csvMutex;





long long nowMs() {
    return duration_cast<milliseconds>(
        high_resolution_clock::now().time_since_epoch()
    ).count();
}



void printETA(int done, int total, long long startMs) {
    if (done == 0) return;

    long long now = nowMs();
    double elapsed = (now - startMs) / 1000.0;

    double rate = done / elapsed;
    double remaining = (total - done) / rate;

    cout << "\nETA: " << remaining / 60.0 << " min\n";
}


string crossoverToStr(CrossoverType t) {
    return t == UNIFORM ? "uniform" : "two_point";
}

string mutationToStr(MutationType t) {
    return t == BIT_FLIP ? "bit_flip" : "random_reset";
}


void showProgress(const string& label, int i, int total) {
    if (i % 10 == 0) {
        cout << label
             << " progress: " << i << "/" << total
             << " (" << (i * 100 / total) << "%)\r"
             << flush;
    }
}


int main() {

    ios::sync_with_stdio(false);

    vector<string> testFiles = {
        "items_70.csv",
        "items_100.csv",
        "items_150.csv",
        "items_200.csv",
        "items_500.csv"
    };

    const int RUNS = 100;

    vector<int> populations = {50, 100, 200, 500};
    vector<CrossoverType> crossovers = {UNIFORM, TWO_POINT};
    vector<MutationType> mutations = {BIT_FLIP, RANDOM_RESET};

    vector<double> mutationRates = {0.01, 0.05, 0.1, 0.2};
    vector<int> tournamentSizes = {3, 5, 7, 10};
    vector<int> stopLimits = {20, 50, 100};

    ofstream summary("results_summary.csv", ios::out);
    summary << "experiment,file,param1,param2,success_rate,avg_value,avg_generations,time_ms\n";

    long long globalStart = nowMs();

    int done = 0;
    int totalTasks = testFiles.size() * 6 * 4; // approx (możesz doprecyzować)

    for (int fi = 0; fi < (int)testFiles.size(); fi++) {

        string filename = testFiles[fi];
        vector<Item> items = loadItemsFromCSV(filename);
        if (items.empty()) continue;

        cout << "\nFILE: " << filename << endl;

        for (int pop : populations) {

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{pop, UNIFORM, BIT_FLIP};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }

                if (i % 10 == 0)
                    showProgress("[POP]", i, RUNS);
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);
                summary << "population," << filename << "," << pop << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";
                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);

        }


        for (auto cross : crossovers) {

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{200, cross, BIT_FLIP};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);
                summary << "crossover," << filename << "," << crossoverToStr(cross) << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";
                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);
        }

        cout << "\n[MUTATION TYPE]\n";

        for (auto mut : mutations) {

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{200, UNIFORM, mut};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }

                if (i % 10 == 0)
                    showProgress("[MUT]", i, RUNS);
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);

                summary << "mutation_type," << filename << "," << mutationToStr(mut) << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";

                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);
        }

        cout << "\n[MUTATION RATE]\n";

        for (double rate : mutationRates) {

            MUTATION_RATE = rate;

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{200, UNIFORM, BIT_FLIP};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }

                if (i % 10 == 0)
                    showProgress("[MUTRATE]", i, RUNS);
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);

                summary << "mutation_rate," << filename << "," << rate << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";

                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);
        }


        cout << "\n[TOURNAMENT]\n";

        for (int t : tournamentSizes) {

            TOURNAMENT_SIZE = t;

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{200, UNIFORM, BIT_FLIP};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }

                if (i % 10 == 0)
                    showProgress("[TURN]", i, RUNS);
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);

                summary << "tournament," << filename << "," << t << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";

                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);
        }



        cout << "\n[STOP CONDITION]\n";

        for (int limit : stopLimits) {

            SAME_FITNESS_LIMIT = limit;

            auto start = nowMs();

            double totalValue = 0;
            double totalGen = 0;
            int success = 0;

            GAConfig config{200, UNIFORM, BIT_FLIP};

            #pragma omp parallel for reduction(+:success,totalValue,totalGen)
            for (int i = 0; i < RUNS; i++) {

                auto stats = runAlgorithm(items, false, config);

                if (stats.foundCorrect) {
                    success++;
                    totalValue += stats.bestValue;
                    totalGen += stats.generationsUsed;
                }

                if (i % 10 == 0)
                    showProgress("[STOP]", i, RUNS);
            }

            auto end = nowMs();

            {
                lock_guard<mutex> lock(csvMutex);

                summary << "stop_condition," << filename << "," << limit << ",-,"
                        << (100.0 * success / RUNS) << ","
                        << (success ? totalValue / success : 0) << ","
                        << (success ? totalGen / success : 0) << ","
                        << (end - start) << "\n";

                summary.flush();
            }

            done++;
            printETA(done, totalTasks, globalStart);
        }
    }

    cout << "\nDONE\n";
}
