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

module;

#include <filesystem>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>

module wordmeter;

namespace wm {
struct WordInfo {
  std::vector<std::size_t> positions;
};

struct Database {
  std::map<std::string, WordInfo> words;
  std::size_t totalWordCount{};
};

std::size_t getTotalWordCount(DatabasePtr const &database) {
  return database->totalWordCount;
}

std::vector<std::size_t> const &getWordPositions(std::string const &w,
                                                 DatabasePtr const &db) {
  if (!db->words.contains(w))
    throw std::runtime_error{"The requested word is not in the database: " + w};
  return db->words.at(w).positions;
}

bool parseText(std::string const &inputFile, DatabasePtr &database) {
  database = {new Database{}, [](Database *const p) { delete p; }};

  if (!std::filesystem::exists(inputFile)) {
    std::cerr << "The file: " << inputFile << " does not exist" << std::endl;
    return 1;
  }

  std::ifstream readStream{inputFile};
  if (!readStream.is_open()) {
    std::cerr << "Failed to open file: " << inputFile << std::endl;
    return 1;
  }

  std::size_t wordIndex{};
  std::string word{};

  while (readStream >> word) {
    if (!database->words.contains(word))
      database->words.emplace(word, WordInfo{});

    auto &info = database->words.at(word);
    info.positions.push_back(wordIndex++);
  }

  database->totalWordCount = wordIndex;
  return true;
}

bool read(std::string const &inputFile, DatabasePtr &database) {
  database = {new Database{}, [](Database *const p) { delete p; }};

  if (!std::filesystem::exists(inputFile)) {
    std::cerr << "The file: " << inputFile << " does not exist" << std::endl;
    return 1;
  }

  std::ifstream readStream{inputFile, std::ios::binary};
  if (!readStream.is_open()) {
    std::cerr << "Failed to open file: " << inputFile << std::endl;
    return 1;
  }

  std::size_t &totalWordCount =
      database->totalWordCount; // number of words in original text
  std::size_t wordCount{};      // number of unique words stored in binary file
  readStream.read((char *)&totalWordCount, sizeof(totalWordCount));
  readStream.read((char *)&wordCount, sizeof(wordCount));

  std::vector<std::size_t> positionCounts{};
  std::vector<char> wordLengths{};
  std::vector<WordInfo> info{};
  positionCounts.reserve(wordCount);
  wordLengths.reserve(wordCount);
  info.reserve(wordCount);

  // Read position counts and word lengths
  for (std::size_t i = 0; i < wordCount; ++i) {
    std::size_t posCnt{};
    char len{};
    readStream.read((char *)&posCnt, sizeof(posCnt));
    readStream.read(&len, sizeof(len));
    positionCounts.push_back(posCnt);
    wordLengths.push_back(len);
  }

  // Read positions
  for (std::size_t i = 0; i < wordCount; ++i) {
    info.push_back({});
    auto &pos = info.back().positions;
    for (std::size_t posIdx = 0; posIdx < positionCounts[i]; ++posIdx) {
      std::size_t position{};
      readStream.read((char *)&position, sizeof(position));
      pos.push_back(position);
    }
  }

  // Read words
  for (std::size_t i = 0; i < wordCount; ++i) {
    std::string word{};
    char c{};
    for (char len = 0; len < wordLengths[i]; ++len) {
      readStream.read(&c, sizeof(char));
      word.push_back(c);
    }
    database->words.emplace(std::move(word), std::move(info[i]));
  }

  return true;
}

bool write(DatabasePtr const &database, std::string const &outputFile) {
  std::ofstream writeStream{outputFile, std::ios::binary};

  std::size_t &totalWordCount = database->totalWordCount;
  std::size_t wordCount = database->words.size();
  writeStream.write((char *)&totalWordCount, sizeof(totalWordCount));
  writeStream.write((char *)&wordCount, sizeof(wordCount));

  for (auto const &word : database->words) {
    auto const positionCount = word.second.positions.size();
    char const length = word.first.size();
    writeStream.write((char *)&positionCount, sizeof(positionCount));
    writeStream.write(&length, sizeof(length));
  }

  for (auto const &word : database->words)
    for (auto const position : word.second.positions)
      writeStream.write((char *)&position, sizeof(position));

  for (auto const &word : database->words)
    writeStream.write(word.first.c_str(), word.first.size());

  return true;
}

void print(DatabasePtr const &database,
           std::ostream &outStream,
           std::size_t const maxPosCnt,
           bool sortByFreq) {
  auto const &words = database->words;
  if (words.empty())
    return;

  std::size_t maxOcc{}, maxWordSz{};

  for (auto const &[word, info] : words) {
    maxOcc = std::max(maxOcc, info.positions.size());
    maxWordSz = std::max(maxWordSz, word.size());
  }

  struct WordT {
    std::string id{};
    WordInfo info{};
  };

  std::vector<WordT> wordTable{};
  wordTable.reserve(words.size());
  for (auto const &[word, info] : words)
    wordTable.push_back(WordT{word, info});

  if (sortByFreq) {
    auto predicate = [](auto const &a, auto const &b) {
      return a.info.positions.size() > b.info.positions.size();
    };
    std::sort(wordTable.begin(), wordTable.end(), predicate);
  }

  for (auto const &wordEntry : wordTable) {
    auto const &word = wordEntry.id;
    auto const &info = wordEntry.info;
    outStream << std::left << std::setw(maxWordSz) << word << " ";
    outStream << std::setw(maxOcc) << info.positions.size() << " ";
    for (std::size_t i = 0; i < maxPosCnt && i < info.positions.size(); ++i) {
      outStream << info.positions[i];
      if (i < std::min(maxPosCnt - 1, info.positions.size() - 1))
        outStream << " ";
    }
    outStream << "\n";
  }
}
} // namespace wm
