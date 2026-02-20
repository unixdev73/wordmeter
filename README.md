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
