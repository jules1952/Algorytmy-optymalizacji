#include "csv_utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<Item> loadItemsFromCSV(const string& filename)
{
    vector<Item> items;
    ifstream file(filename);

    if (!file)
    {
        cerr << "Nie mozna otworzyc pliku: " << filename << "\n";
        return items;
    }

    string line;
    getline(file, line); // pomijamy naglowek

    while (getline(file, line))
    {
        stringstream ss(line);
        string part;

        Item item;
        int id;

        getline(ss, part, ',');
        id = stoi(part);

        getline(ss, part, ',');
        item.value = stoi(part);

        getline(ss, part, ',');
        item.weight = stoi(part);

        getline(ss, part, ',');
        item.volume = stoi(part);

        items.push_back(item);
    }

    return items;
}