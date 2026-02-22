#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <regex>

import wordmeter;

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Invocation not valid; Usage: [options] <inputFile.db> "
                 "[<maxPosColSz>]";
    std::cerr << std::endl;
    return 1;
  }

  std::string inputFile{}, maxPosStr{};
  long maxPositions{-1};
  short posArgIdx{};
  bool sortByFreq{}, wordCount{}, makePairs{};
  std::size_t pairWordCount{};
  std::smatch match{};

  for (int i = 1; i < argc; ++i) {
    std::string argument = argv[i];

    if (argument == "--sortByFreq")
      sortByFreq = true;
    else if (argument == "--wordCount")
      wordCount = true;
    else if (std::regex_match(
                 argument, match, std::regex{"^--makePairs=(.+)$"})) {
      makePairs = true;
      try {
        pairWordCount = std::stoul(match[1]);
      } catch (...) {
        std::cerr << "Failed to convert: " << match[1] << " to number\n";
        return 1;
      }
    } else if (std::regex_match(argument, std::regex{"^--makePairs$"}))
      makePairs = true;
    else {
      if (!posArgIdx)
        inputFile = argv[i];
      else
        maxPosStr = argv[i];
      ++posArgIdx;
    }
  }

  if (maxPosStr.size()) {
    try {
      maxPositions = std::stol(maxPosStr);
    } catch (...) {
      std::cerr << "Failed to convert: " << maxPosStr << " to number\n";
      return 1;
    }
  }

  wm::DatabasePtr db{0, 0};
  if (!wm::read(inputFile, db)) {
    std::cerr << "Failed to read binary file: " << inputFile << std::endl;
    return 1;
  }

  if (wordCount)
    std::cout << wm::getWordCount(db) << std::endl;
  else if (makePairs) {
    auto pairs = wm::makeMostCommonPairs(db, pairWordCount);
    for (auto const &[a, b] : pairs)
      std::cout << a << " " << b << "\n";
  } else
    print(db, std::cout, maxPositions, sortByFreq);
}
