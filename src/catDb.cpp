#include <filesystem>
#include <functional>
#include <iostream>
#include <string>

import wordmeter;

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Invocation not valid; Usage: <inputFile.db> [<maxPosColSz>]";
    std::cerr << std::endl;
    return 1;
  }

  std::string const inputFile{argv[1]};
  std::string maxPosStr{};
  std::size_t maxPositions{};

  if (argc > 2) {
    maxPosStr = argv[2];
    try {
      maxPositions = std::stoul(maxPosStr);
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

  if (argc > 2)
    print(db, std::cout, maxPositions);
  else
    print(db, std::cout);
}
