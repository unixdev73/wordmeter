/* Copyright (c) 2026 unixdev73@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <unordered_map>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <regex>

import wordmeter;

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Invocation not valid; Usage: [options] <inputFile> "
                 "<wordPairFile.txt>\n";
    std::cerr << std::endl;
    return 1;
  }

  bool dist{}, minDist{}, maxDist{}, avgDist{}, sqVariant{}, hist{}, normHist{},
      all{};
  std::string inputFile{}, wordPairFile{}, outputDir{"./"}, wordA{}, wordB{};
  short posArgIdx{};
  std::smatch match{};

  for (int i = 1; i < argc; ++i) {
    std::string const argument = argv[i];
    if (argument == "--dist")
      dist = true;
    else if (argument == "--minDist")
      minDist = true;
    else if (argument == "--maxDist")
      maxDist = true;
    else if (argument == "--avgDist")
      avgDist = true;
    else if (argument == "--sqVariant")
      sqVariant = true;
    else if (std::regex_match(argument, match, std::regex{"^--hist=(.+)$"})) {
      outputDir = match[1];
      if (!std::filesystem::exists(outputDir)) {
        std::cerr << "The directory: " << outputDir << " does not exist\n";
        return 1;
      }
      hist = true;
    } else if (argument == "--hist")
      hist = true;
    else if (argument == "--normHist")
      normHist = true;
    else if (argument == "--all")
      all = true;
    else {
      if (!posArgIdx)
        inputFile = argv[i];
      else
        wordPairFile = argv[i];
      ++posArgIdx;
    }
  }

  std::string const ext = std::filesystem::path{inputFile}.extension().string();
  wm::DatabasePtr db{0, 0};

  if (ext == ".txt") {
    if (!wm::parseText(inputFile, db)) {
      std::cerr << "Failed to parse file: " << inputFile << std::endl;
      return 1;
    }
  } else {
    if (!wm::read(inputFile, db)) {
      std::cerr << "Failed to read file: " << inputFile << std::endl;
      return 1;
    }
  }

  std::ifstream stream{wordPairFile};
  if (!stream.is_open()) {
    std::cerr << "Failed to open word pair file: " << wordPairFile << std::endl;
    return 1;
  }

  std::size_t const totalWordCnt = wm::getTotalWordCount(db);
  std::size_t const colWidth{15};
  std::cout << std::left << std::setprecision(colWidth);

  while (stream >> wordA) {
    if (!(stream >> wordB))
      break;

    std::vector<std::size_t> distances{};
    try {
      auto const &posA = wm::getWordPositions(wordA, db);
      auto const &posB = wm::getWordPositions(wordB, db);
      distances = wm::calcDist(posA, posB, totalWordCnt);
    } catch (std::exception const &error) {
      std::cerr << error.what() << "\n";
      return 1;
    }

    if (dist || minDist || maxDist || avgDist || sqVariant || all)
      std::cout << wordA << " " << wordB << std::endl;

    if (dist || all) {
      std::cout << std::setw(colWidth) << "count " << distances.size() << " ";
      std::cout << std::setw(colWidth) << "distances ";
      for (std::size_t i = 0; i < distances.size(); ++i) {
        std::cout << distances[i];
        if (i < distances.size() - 1)
          std::cout << " ";
      }
      std::cout << std::endl;
    }

    if (minDist || all) {
      std::cout << std::setw(colWidth) << "minDistance ";
      std::size_t val = distances.front();
      for (std::size_t i = 0; i < distances.size(); ++i)
        val = std::min(val, distances[i]);
      std::cout << val << std::endl;
    }

    if (maxDist || all) {
      std::cout << std::setw(colWidth) << "maxDistance ";
      std::size_t val{};
      for (std::size_t i = 0; i < distances.size(); ++i)
        val = std::max(val, distances[i]);
      std::cout << val << std::endl;
    }

    if (avgDist || sqVariant || all) {
      double avgDistance{}, squareVariant{};
      for (auto const d : distances) {
        squareVariant += d * d;
        avgDistance += d;
      }
      avgDistance = avgDistance / distances.size();
      squareVariant = squareVariant / distances.size();
      squareVariant -= avgDistance * avgDistance;

      if (avgDist || all) {
        std::cout << std::setw(colWidth) << "avgDistance ";
        std::cout << avgDistance << std::endl;
      }

      if (sqVariant || all) {
        std::cout << std::setw(colWidth) << "sqVariant ";
        std::cout << squareVariant << std::endl;
      }
    }

    if (hist) {
      std::string const output = outputDir + "/" + wordA + "_" + wordB + ".txt";
      std::ofstream histData{output};
      if (!histData.is_open()) {
        std::cerr << "Failed to open histogram for writing: " << output << "\n";
        return 1;
      }

      std::unordered_map<std::size_t, std::size_t> distOccMap{};
      for (auto const &distance : distances) {
        if (!distOccMap.contains(distance))
          distOccMap.emplace(distance, 0);
        ++distOccMap.at(distance);
      }

      std::vector<std::pair<std::size_t, double>> distOcc{};
      distOcc.reserve(distOccMap.size());
      if (normHist)
        for (auto const &[distance, occurrence] : distOccMap)
          distOcc.push_back(
              {distance, double(occurrence) / double(distances.size())});
      else
        for (auto const &[distance, occurrence] : distOccMap)
          distOcc.push_back({distance, double(occurrence)});

      auto predicate = [](auto const &a, auto const &b) {
        return a.first < b.first; // sort by distance
      };
      std::sort(distOcc.begin(), distOcc.end(), predicate);

      for (auto const &[distance, occurrence] : distOcc)
        histData << distance << " " << std::setprecision(15) << occurrence
                 << "\n";
    }
  }

  return 0;
}
