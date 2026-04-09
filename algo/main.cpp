#include "genetic_algorithm.h"
#include "csv_utils.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    string filename;
    cout << "Podaj nazwe pliku CSV: ";
    cin >> filename;

    vector<Item> items = loadItemsFromCSV(filename);

    if (items.empty()) {
        cout << "Nie udalo sie wczytac danych.\n";
        return 1;
    }

    // Dodaj to:
    GAConfig config;
    config.populationSize = 200;
    config.crossoverType = UNIFORM;
    config.mutationType = BIT_FLIP;

    // Przekaż config jako trzeci argument:
    runAlgorithm(items, true, config);

    return 0;
}
