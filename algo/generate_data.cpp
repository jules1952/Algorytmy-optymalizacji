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

mt19937 generator(time(0));

int NUM_ITEMS = 70;

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

void saveItemsToCSV(const vector<Item>& items, const string& filename)
{
    ofstream file(filename);

    file << "id,value,weight,volume\n";

    for (int i = 0; i < items.size(); i++)
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
    for (int test = 1; test <= 5; test++)
    {
        vector<Item> items = generateItems();
        string filename = "items" + to_string(test) + ".csv";
        saveItemsToCSV(items, filename);
    }

    cout << "Wygenerowano 5 plikow: items1.csv ... items5.csv\n";
    return 0;
}