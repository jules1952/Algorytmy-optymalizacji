#include "genetic_algorithm.h"

int main()
{
    std::vector<Item> items = generateItems();
    runAlgorithm(items);
    return 0;
}