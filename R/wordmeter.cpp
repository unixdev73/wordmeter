/* USAGE:
 * Create a main database instance:
 * 	db <- createWordmeterDatabase()
 *
 * Parse texts:
 *  parseText(db, "MyText1", c("word", ...))
 */

#include <Rcpp.h>

struct WordInfo {
  std::vector<std::size_t> positions;
};

struct Database {
  std::map<std::string, WordInfo> words;
  std::size_t totalWordCount{};
};

struct WordmeterDatabase {
	std::unordered_map<std::string, Database> texts;
	std::vector<std::pair<std::string, std::string>> wordPairs;
};

using DbHandle = Rcpp::XPtr<WordmeterDatabase>;

// [[Rcpp::export]]
DbHandle createWordmeterDatabase() {
	/* Description: Creates a wordmeter database structure */
	return DbHandle(new WordmeterDatabase{});
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
