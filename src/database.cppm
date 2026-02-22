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

#include <functional>
#include <ostream>
#include <memory>
#include <vector>
#include <string>

export module wordmeter:database;

export namespace wm {
struct Database;

using DatabaseDel = std::function<void(Database *const)>;
using DatabasePtr = std::unique_ptr<Database, DatabaseDel>;

// Generate a database structure from a text file
bool parseText(std::string const &inputTextFile, DatabasePtr &database);

// Save a database structure as a binary file
bool write(DatabasePtr const &database, std::string const &outputFile);

// Create a database structure from a binary file
bool read(std::string const &inputBinaryFile, DatabasePtr &database);

// Display the database, and only list up to maxPosCnt positions for each entry
void print(DatabasePtr const &database,
           std::ostream &outStream,
           std::size_t const maxPosCnt,
           bool sortByFreq);

std::size_t getTotalWordCount(DatabasePtr const &database);

std::vector<std::size_t> const &getWordPositions(std::string const &word,
                                                 DatabasePtr const &db);
} // namespace wm
