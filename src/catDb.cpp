#include <filesystem>
#include <functional>
#include <iostream>
#include <string>

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
  bool sortByFreq{};

  for (int i = 1; i < argc; ++i) {
    if (std::string{argv[i]} == "--sortByFreq")
      sortByFreq = true;
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

  print(db, std::cout, maxPositions, sortByFreq);
}
