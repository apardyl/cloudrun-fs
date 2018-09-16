#include <algorithm>
#include <random>
#include "randomutils.h"

std::string random_string(int length) {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, max_index - 1);
    std::string str(length, 0);
    for (char &c : str) {
        c = charset[distribution(generator)];
    }
    return str;
}
