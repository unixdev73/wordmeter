/* USAGE:
 * Create database instance:
 * 	db <- createWordmeterDatabase()
 *
 * Parse texts:
 *  parseText(db, "MyText1", c("word", ...))
 *
 * Generate list of most popular pairs of N most popular words
 *  populateMostCommonPairs(db, N, c("MyText1", ...))
 *
 * Compare two texts; This returns a square 2D matrix of side length equal
 * to the number of texts specified for comparison.
 *  mat2D <- compareTexts(db, c("MyText1", "MyText2", ...))
 */

#include <Rcpp.h>

struct WordInfo {
  std::vector<std::size_t> positions;
};

struct Database {
  std::map<std::string, WordInfo> words;
  std::size_t totalWordCount{};
};

struct PairHash {
    std::size_t operator()(const std::pair<std::string, std::string>& p) const {
        std::size_t h1 = std::hash<std::string>{}(p.first);
        std::size_t h2 = std::hash<std::string>{}(p.second);
        return h1 ^ (h2 << 1);  // combine hashes
    }
};

struct TextPairInfo {
	using WordPair = std::pair<std::string, std::string>;

	std::unordered_map<WordPair, std::vector<std::size_t>, PairHash> distances;
	std::unordered_map<WordPair, std::map<std::size_t, double>, PairHash> histograms;
	std::unordered_map<WordPair, std::map<std::size_t, double>, PairHash> cumulants;
};

struct WordmeterDatabase {
	std::unordered_map<std::string, Database> texts;
	std::vector<std::pair<std::string, std::string>> wordPairs;
	std::unordered_map<std::string, TextPairInfo> textPairInfo;
};

using DbHandle = Rcpp::XPtr<WordmeterDatabase>;

std::map<std::size_t, double>
calcCumulant(std::map<std::size_t, double> const& histogram) {
	std::map<std::size_t, double> cumulant;
	double val{};
	for (auto const &[distance, freq] : histogram) {
		cumulant.emplace(distance, freq + val);
		val += freq;
	}

	return cumulant;
}

std::map<std::size_t, double>
calcHist(std::vector<std::size_t> const& distances) {
	std::map<std::size_t, double> distOccMap{};

	for (auto const &distance : distances) {
		if (!distOccMap.count(distance))
			distOccMap.emplace(distance, 0);
		++distOccMap.at(distance);
	}

	for (auto const &[distance, occurrence] : distOccMap)
		distOccMap.at(distance) = double(occurrence) / double(distances.size());

	return distOccMap;
}

std::vector<std::size_t> calcDist(std::vector<std::size_t> const &posA,
                                  std::vector<std::size_t> const &posB,
                                  std::size_t const totalWordCount) {
  auto const p = [](auto const a, auto const b) { return a > b; };
  std::vector<std::size_t> out{};
  out.reserve(posA.size());

  for (auto nearestA = posA.begin(); nearestA != posA.end();) {
    auto nearestB = std::upper_bound(posB.begin(), posB.end(), *nearestA);

    if (nearestB == posB.end()) {
      auto rnearestA =
          std::upper_bound(posA.rbegin(), posA.rend(), posB.front(), p);
      if (rnearestA == posA.rend())
        out.insert(out.end(), totalWordCount - posA.back() + posB.front());
      return out;
    }

    auto rnearestA = std::upper_bound(posA.rbegin(), posA.rend(), *nearestB, p);

    if (rnearestA.base() > nearestA)
      nearestA = rnearestA.base();
    else
      ++nearestA;

    out.insert(out.end(), *nearestB - *rnearestA);
  }

  return out;
}

std::vector<std::pair<std::string, std::string>>
makeCombinations(std::vector<std::string> const& items) {
	std::vector<std::pair<std::string, std::string>> result;

	for (size_t i = 0; i < items.size(); ++i) {
		for (size_t j = i + 1; j < items.size(); ++j) {
			result.emplace_back(items[i], items[j]);
		}
	}

	return result;
}

// [[Rcpp::export]]
Rcpp::NumericMatrix compareTexts(DbHandle const& db, std::vector<std::string> const& texts) {
	/* Parameters:
	 * db - pointer to main database structure
	 * texts - identifiers of texts to compare
	 *
	 * Description:
	 * Creates a 2D matrix of differences between two files.
	 * Each difference is the result of the square root of the sum of the squares
	 * of discrete integrals of absolute values of differences of cumulants.
	 */

	auto const wmDb = db.get();
	if (!wmDb) {
		std::cerr << "The database pointer = nullptr" << std::endl;
		return Rcpp::NumericMatrix{};
	}

	if (!wmDb->wordPairs.size()) {
		std::cerr << "There are no words pairs to use for the matrix" << std::endl;
		return Rcpp::NumericMatrix{};
	}

	for (std::size_t i = 0; i < texts.size(); ++i) {
		if (!wmDb->texts.count(texts[i])) {
			std::cerr << "The text id: " << texts[i] << " is not the id of a parsed text" << std::endl;
			return Rcpp::NumericMatrix{};
		}
		for (std::size_t j = 0; j < texts.size(); ++j) {
			if (i != j && texts[i] == texts[j]) {
				std::cerr << "The text id: " << texts[i] << " is not unique" << std::endl;
				return Rcpp::NumericMatrix{};
			}
		}
	}

	Rcpp::NumericMatrix compTable(texts.size(), texts.size());
	auto& tpInfoDb = wmDb->textPairInfo;

	// compute word pair histograms for each word pair for each file
	// compute cumulants from each histogram for each file
	for (auto const& [wordA, wordB] : wmDb->wordPairs) {
		for (auto const textId : texts) {
			auto const& textDb = wmDb->texts.at(textId);

			if (!tpInfoDb.count(textId))
				tpInfoDb.emplace(textId, TextPairInfo{});		
			auto& tpInfo = tpInfoDb.at(textId);

			std::vector<std::size_t> posA{}, posB{};
			if (textDb.words.count(wordA))
				posA = textDb.words.at(wordA).positions;
			if (textDb.words.count(wordB))
				posB = textDb.words.at(wordB).positions;

			auto const wp = std::pair<std::string, std::string>(wordA, wordB);
			if (!tpInfo.distances.count(wp))
				tpInfo.distances.emplace(wp, std::vector<std::size_t>{});
			tpInfo.distances.at(wp) = calcDist(posA, posB, textDb.totalWordCount);

			if (!tpInfo.histograms.count(wp))
				tpInfo.histograms.emplace(wp, std::map<std::size_t, double>{});
			tpInfo.histograms.at(wp) = calcHist(tpInfo.distances.at(wp));

			if (!tpInfo.cumulants.count(wp))
				tpInfo.cumulants.emplace(wp, std::map<std::size_t, double>{});
			tpInfo.cumulants.at(wp) = calcCumulant(tpInfo.histograms.at(wp));
		}
	}

	auto textCombos = makeCombinations(texts);
	// compute discrete integrals from cumulants from each file for each word pair
	// sum the difference squares and take the square root
	// insert the result in the appropriate cell in the matrix

	return Rcpp::NumericMatrix{};
}

std::vector<std::pair<std::string, std::size_t>>
getMostCommonWords(Database const* const &database, std::size_t const wordCount) {
  struct WordT {
    std::string id;
    std::size_t occ{};
  };

  std::vector<std::pair<std::string, std::size_t>> words{};
  words.reserve(database->words.size());

  for (auto const &[word, info] : database->words)
    words.push_back({word, info.positions.size()});

  auto predicate = [](auto const &a, auto const &b) {
    return a.second > b.second;
  };
  std::sort(words.begin(), words.end(), predicate);

  for (; words.size() > wordCount;)
    words.pop_back();

  return words;
}

std::vector<std::pair<std::string, std::string>>
makePairs(std::vector<std::pair<std::string, std::size_t>> const &v) {
  std::vector<std::pair<std::string, std::string>> out{};
  out.reserve(v.size() * v.size());

  for (std::size_t i = 0; i < v.size(); ++i)
    for (std::size_t j = 0; j < v.size(); ++j)
      out.push_back({v[i].first, v[j].first});

  return out;
}

// [[Rcpp::export]]
void populateMostCommonPairs(DbHandle const& db, std::size_t const count, std::vector<std::string> const& identifiers) {
	/* Parameters:
	 * db - pointer to main database structure
	 * count - count^2 pairs will be generated from count most popular words
	 * identifiers - text ids from which the most popular words will be chosen
	 *
	 * Description:
	 * Generates all possible pairs from a list of count most commonly
	 * occurring words in a list of texts specified in an array.
	 * The resulting list of pairs is stored for later operations.
	 */

  std::vector<std::pair<std::string, std::size_t>> mostCommonWords{};
  std::unordered_map<std::string, std::size_t> uniqueWords{};
	std::vector<Database const*> textDatabases;
	auto const wmDb = db.get();

	for (auto const id : identifiers) {
		if (!wmDb->texts.count(id)) {
			std::cerr << "The requested text is not parsed: " << id << std::endl;
			return;
		}

		textDatabases.push_back(&wmDb->texts.at(id));
	}

  for (std::size_t i = 0; i < textDatabases.size(); ++i) {
    auto mcw = getMostCommonWords(textDatabases[i], count);
    for (std::size_t j = 0; j < mcw.size(); ++j) {
      if (!uniqueWords.count(mcw[j].first))
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

  for (; mostCommonWords.size() > count;)
    mostCommonWords.pop_back();

	wmDb->wordPairs = makePairs(mostCommonWords);
}

// [[Rcpp::export]]
void parseText(DbHandle const& db, std::string const& id, std::vector<std::string> const& text) {
	/* Parameters:
	 * db - pointer to main database structure
	 * id - name of text
	 * text - array of words, for example: c("word", ...)
	 *
	 * Description:
	 * Calculates the number of occurrences of each unique word in the text
	 * and stores the position of each occurrence
	 */

	if (id.empty() || text.empty()) {
		std::cerr << "The id or text parameter is empty" << std::endl;
		return;
	}

	auto const wmDb = db.get();
	if (!wmDb) {
		std::cerr << "The main database pointer = nullptr" << std::endl;
		return;
	}

	auto& texts = wmDb->texts;
	if (!texts.count(id))
		texts.emplace(id, Database{});

	auto& textDb = texts.at(id);
	auto& words = textDb.words;

	textDb.totalWordCount = text.size();
	std::size_t wordIndex{};

  for (auto const& word : text) {
    if (!words.count(word))
      words.emplace(word, WordInfo{});

    auto &info = words.at(word);
    info.positions.push_back(wordIndex++);
  }
}

// [[Rcpp::export]]
DbHandle createWordmeterDatabase() {
	/* Description: Creates a wordmeter database structure */
	return DbHandle(new WordmeterDatabase{});
}
