#include <unordered_map>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

import wordmeter;

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <file.db> [<fileN.db>]... N\n";
    return 1;
  }

  std::vector<wm::DatabasePtr> databases{};
  databases.reserve(argc - 2);
  std::vector<std::string> files{};
  files.reserve(argc - 2);

  std::size_t n{}; // n is the number of the most popular words to consider

  for (int i = 1; i < argc - 1; ++i) {
    if (!std::filesystem::exists(argv[i])) {
      std::cerr << "The file: " << argv[i] << " does not exist" << std::endl;
      return 1;
    }

    files.push_back(argv[i]);
  }

  try {
    n = std::stoul(argv[argc - 1]);
  } catch (...) {
    std::cerr << "Failed to convert: " << argv[argc - 1] << " to number\n";
    return 1;
  }

  for (std::size_t i = 0; i < files.size(); ++i) {
    databases.push_back(wm::DatabasePtr{0, 0});
    if (!wm::read(files[i], databases[i])) {
      std::cerr << "Failed to read database from: " << files[i] << std::endl;
      return 1;
    }
  }

  std::vector<std::pair<std::string, std::size_t>> mostCommonWords{};
  std::unordered_map<std::string, std::size_t> uniqueWords{};

  for (std::size_t i = 0; i < databases.size(); ++i) {
    auto mcw = wm::getMostCommonWords(databases[i], n);
    for (std::size_t j = 0; j < mcw.size(); ++j) {
      if (!uniqueWords.contains(mcw[j].first))
        uniqueWords.emplace(mcw[j].first, 0);
      uniqueWords.at(mcw[j].first) += mcw[j].second;
    }
  }

  mostCommonWords.reserve(uniqueWords.size());
  for (auto const &[word, count] : uniqueWords)
    mostCommonWords.push_back({word, count});

  auto predicate = [](auto const &a, auto const &b) {
    return a.second > b.second;
  };
  std::sort(mostCommonWords.begin(), mostCommonWords.end(), predicate);

  for (; mostCommonWords.size() > n;)
    mostCommonWords.pop_back();

  auto mostCommonPairs = wm::makePairs(mostCommonWords);
  for (auto const &[wordA, wordB] : mostCommonPairs)
    std::cout << wordA << " " << wordB << "\n";
}
