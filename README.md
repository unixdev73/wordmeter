# Introduction

wordmeter is a project that aims to help linguists analyze texts.

wordmeter parses a text file and generates a binary database file.

Using the database, wordmeter can calculate the distances between
each occurrence of two words, as well as the average distance between them.

# Building

```console
cmake -B build -G "Ninja"
cmake --build ./build
```

# Usage

In the build dir in the subdir src there are the following binaries

- parseText
- calcStat
- catDb

### parseText

parseText generates a binary file from a text file that contains a database
of word occurrences and their positions. This speed up analysis later.

```console
Usage: parseText <inputFile.txt> <outputFile.db>
```

### calcStat

calcStat prints statistics regarding an input file. It can read a text file
directly (slower for multiple runs) or using a database binary file.

It takes as input a database file / text file, and a word pair file.
For each word pair in the word pair file it calculates the requested properties.

calcStat can also generate histograms that contain each distance and its
occurrence count for every word pair.

```console
Usage: calcStat [options] <inputFile.db> <wordPairFile.txt>
Usage: calcStat [options] <inputFile.txt> <wordPairFile.txt>

Options:
--dist prints the distances between each occurrence of wordA and wordB.
--minDist prints the smallest distance between wordA and wordB.
--maxDist prints the largest distance between wordA and wordB.
--avgDist prints the average distance between wordA and wordB.
--sqVariant prints the square variance of wordA and wordB.
--all applies all of the above options.
--hist[=outDir] generates distance histograms for each word pair.
--normHist divides the count of each distance by the number of all distances.
```

### catDb

catDb by default will display each word in the database, the number of occurrences
in the text, and each position at which it occurs.

The output may be sorted so that the most commonly occuring word is at the top.

Alternatively, the catDb utility can be used to print out a list of pairs
of the N most common words. For example, if there are two words in the text file,
then if N = 2, the output would be wordA wordA, wordA wordB, wordB wordB, wordB wordA.

```console
Usage: catDb [options] <inputFile.db> [<maxPosColSz>]

Options:
--sortByFreq sorts the output by popularity in descending order.
--wordCount prints only the number of unique words in the database.
--makePairs[=N] prints all (N^2) word pairs of the N most common words.
```
