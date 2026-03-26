#include "genetic_algorithm.h"
#include "csv_utils.h"

#include <iostream>
#include <string>

using namespace std;

int main()
{
    string filename;
    cout << "Podaj nazwe pliku CSV: ";
    cin >> filename;

    vector<Item> items = loadItemsFromCSV(filename);

    if (items.empty())
    {
        cout << "Nie udalo sie wczytac danych.\n";
        return 1;
    }

    runAlgorithm(items);

    return 0;
}