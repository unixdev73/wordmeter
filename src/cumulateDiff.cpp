#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <cmath>
#include <map>

std::map<std::size_t, double> readCumulant(std::istream &src);

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: <cumulated1.txt> <cumulated2.txt>" << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(argv[1])) {
    std::cerr << "The file: " << argv[1] << " does not exist" << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(argv[2])) {
    std::cerr << "The file: " << argv[2] << " does not exist" << std::endl;
    return 1;
  }

  std::ifstream f1{argv[1]}, f2{argv[2]};

  if (!f1.is_open()) {
    std::cerr << "Failed to open file: " << argv[1] << std::endl;
    return 1;
  }

  if (!f2.is_open()) {
    std::cerr << "Failed to open file: " << argv[2] << std::endl;
    return 1;
  }

  auto c1 = readCumulant(f1);
  auto c2 = readCumulant(f2);

  auto calcSum = [](auto const &c1, auto const &c2) {
    auto const begin = std::min(c1.begin()->first, c2.begin()->first);
    auto const end = std::max(c1.rbegin()->first, c2.rbegin()->first);
    double y1{}, y2{};
    double sum{};

    for (std::size_t i = begin; i <= end; ++i) {

      if (c1.contains(i))
        y1 = c1.at(i);
      if (c2.contains(i))
        y2 = c2.at(i);

      sum += std::abs(y1 - y2);
    }

    return sum;
  };

  auto const sum = calcSum(c1, c2);
  std::cout << sum << std::endl;
}

std::map<std::size_t, double> readCumulant(std::istream &src) {
  std::map<std::size_t, double> data{};
  std::size_t dist{}; // distance between word A and B
  double occ{};       // occurrence of that distance in a text file

  unsigned char colId{};
  std::string tmp{};

  while (src >> tmp) {
    if (!colId) {
      try {
        dist = std::stoul(tmp);
      } catch (...) {
        std::cerr << "Failed to convert: " << tmp << " to a number"
                  << std::endl;
        throw;
      }
    } else {
      try {
        occ = std::stod(tmp);
      } catch (...) {
        std::cerr << "Failed to convert: " << tmp << " to a number"
                  << std::endl;
        throw;
      }

      data.emplace(dist, occ);
    }

    colId = (colId + 1) % 2;
  }

  return data;
}
