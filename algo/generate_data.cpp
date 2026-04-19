#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;

struct Item
{
    int value;
    int weight;
    int volume;
};

mt19937 generator(42);

// Funkcja generuje tyle przedmiotow, ile podasz w argumencie
vector<Item> generateItems(int numItems)
{
    vector<Item> items;
    items.reserve(numItems);

    uniform_int_distribution<int> valueDist(10, 50);
    uniform_int_distribution<int> weightDist(1, 10);
    uniform_int_distribution<int> volumeDist(1, 8);

    for (int i = 0; i < numItems; i++)
    {
        Item item;
        item.value = valueDist(generator);
        item.weight = weightDist(generator);
        item.volume = volumeDist(generator);

        items.push_back(item);
    }

    return items;
}

void saveItemsToCSV(const vector<Item>& items, const string& filename)
{
    ofstream file(filename);

    if (!file)
    {
        cout << "Nie mozna zapisac pliku: " << filename << "\n";
        return;
    }

    file << "id,value,weight,volume\n";

    for (int i = 0; i < static_cast<int>(items.size()); i++)
    {
        file << i << ","
             << items[i].value << ","
             << items[i].weight << ","
             << items[i].volume << "\n";
    }

    file.close();
}

int main()
{
    vector<int> sizes = {70, 100, 150, 200, 500};

    for (int size : sizes)
    {
        vector<Item> items = generateItems(size);
        string filename = "items_" + to_string(size) + ".csv";
        saveItemsToCSV(items, filename);
    }

    cout << "Wygenerowano pliki: items_70.csv, items_100.csv, items_150.csv, items_200.csv, items_500.csv\n";
    return 0;
}
