#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <list>
#include <set>
#include <thread>
#include <mutex>

using namespace std;

// Mutex to synchronize access to shared resources
mutex mtx;

static list<string> bookNames = {
    "Cats by Moncrif.txt",
    "Foxes Book of Martyrs Part 1.txt",
    "Foxes Book of Martyrs Part 2.txt",
    "Foxes Book of Martyrs Part 3.txt",
    "Foxes Book of Martyrs Part 4.txt",
    "Foxes Book of Martyrs Part 5.txt",
    "Foxes Book of Martyrs Part 6.txt",
    "Gerards Herbal Vol. 1.txt",
    "Gerards Herbal Vol. 2.txt",
    "Gerard's Herbal Vol. 3.txt",
    "Gerards Herbal Vol.4.txt",
    "Gil Blas.txt",
    "Gossip in a Library.txt",
    "Hudibras.txt",
    "King of the Beggars.txt",
    "Knocknagow.txt",
    "Les Chats par Moncrif.txt",
    "Love and Madness - Herbert Croft.txt",
    "Lives and Anecdotes of Misers.txt",
    "Memoirs of Laetitia Pilkington V 1.txt",
    "Memoirs of Laetitia Pilkington V 2.txt",
    "Memoirs of Laetitia Pilkington V 3.txt",
    "Memoirs of Mrs Margaret Leeson - Peg Plunkett.txt",
    "Monro his Expedition.txt",
    "Mrs Beetons Book of Household Management.txt",
    "Out of the Hurly-Burly.txt",
    "Percys Reliques.txt",
    "Pompey the Little.txt",
    "Radical Pamphlets from the English Civil War.txt",
    "Scepsis Scientifica.txt",
    "The Anatomy of Melancholy Part 1.txt",
    "The Anatomy of Melancholy Part 2.txt",
    "The Anatomy of Melancholy Part 3.txt",
    "The Complete Cony-catching.txt",
    "The Consolation of Philosophy.txt",
    "The Covent Garden Calendar.txt",
    "The Devil on Two Sticks.txt",
    "The Diary of a Lover of Literature.txt",
    "The History Of Ireland - Geoffrey Keating.txt",
    "The History of the Human Heart.txt",
    "The Ingoldsby Legends.txt",
    "The Life of Beau Nash.txt",
    "The Life of john Buncle by Thomas Amory.txt",
    "The Life of King Richard III.txt",
    "The Life of Pico della Mirandola.txt",
    "The Martyrdom of Man.txt",
    "The Masterpiece of Aristotle.txt",
    "The Memoirs of Count Boruwlaski.txt",
    "The Metamorphosis of Ajax.txt",
    "The Newgate Calendar - Supplement 3.txt",
    "The Newgate Calendar Supplement 2.txt",
    "The Newgate Calendar Supplement.txt",
    "The Newgate Calendar V 1.txt",
    "The Newgate Calendar V 2.txt",
    "The Newgate Calendar V 3.txt",
    "The Newgate Calendar V 4.txt",
    "The Newgate Calendar V 5.txt",
    "The Newgate Calendar V 6.txt",
    "The Poems of Ossian.txt",
    "The Poetical Works of John Skelton.txt",
    "The Protestant Reformation.txt",
    "The Real Story of John Carteret Pilkington.txt",
    "The Rowley Poems.txt",
    "The Silver Fox.txt"};

class BookAnalysis {
    map<string, list<string>> bookContent;
    map<string, map<string, int>> wordFrequency;
    map<string, map<string, double>> normalizedWordScores;
    map<string, list<string>> frequentWords;
    map<string, double> similarityScores;

public:
    void loadBooks(list<string>& bookTitles) {
        mtx.lock();
        cout << "Starting the process of reading book content..." << endl;
        mtx.unlock();

        set<string> ignoredWords = {
            "the", "be", "to", "of", "and", "a", "in", "that", "have", "I",
            "it", "for", "not", "on", "with", "he", "as", "you", "do", "at",
            "this", "but", "his", "by", "from", "they", "we", "say", "her",
            "she", "or", "an", "will", "my", "one", "all", "would", "there",
            "their", "what", "so", "up", "out", "if", "about", "who", "get",
            "which", "go", "me"};

        vector<thread> threads;
        for (const string& title : bookTitles) {
            threads.emplace_back([this, &title, &ignoredWords]() {
                ifstream file("books/" + title);
                if (file.is_open()) {
                    list<string> words;
                    string word;
                    while (file >> word) {
                        transform(word.begin(), word.end(), word.begin(), ::tolower);
                        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
                        if (ignoredWords.find(word) == ignoredWords.end()) {
                            words.push_back(word);
                        }
                    }
                    {
                        lock_guard<mutex> lock(mtx);
                        bookContent[title] = words;
                    }
                    file.close();
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        mtx.lock();
        cout << "Finished loading all book data!" << endl << endl;
        mtx.unlock();
    }

    void countWordOccurrences() {
        mtx.lock();
        cout << "Counting words across all books..." << endl;
        mtx.unlock();

        vector<thread> threads;
        for (const auto& book : bookContent) {
            threads.emplace_back([this, &book]() {
                map<string, int> wordCount;
                int totalWordCount = 0;
                for (const auto& word : book.second) {
                    wordCount[word]++;
                    totalWordCount++;
                }
                wordCount["total"] = totalWordCount;
                {
                    lock_guard<mutex> lock(mtx);
                    wordFrequency[book.first] = wordCount;
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        mtx.lock();
        cout << "Word counting complete!" << endl << endl;
        mtx.unlock();
    }

    void computeNormalizedWordScores() {
        mtx.lock();
        cout << "Calculating normalized scores for word occurrences..." << endl;
        mtx.unlock();

        vector<thread> threads;
        for (const auto& book : wordFrequency) {
            threads.emplace_back([this, &book]() {
                map<string, double> normalizedScores;
                int totalWords = book.second.at("total");
                for (const auto& entry : book.second) {
                    if (entry.first != "total") {
                        normalizedScores[entry.first] = static_cast<double>(entry.second) / totalWords;
                    }
                }
                {
                    lock_guard<mutex> lock(mtx);
                    normalizedWordScores[book.first] = normalizedScores;
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        mtx.lock();
        cout << "Normalization process completed successfully!" << endl << endl;
        mtx.unlock();
    }

    void highlightFrequentWords() {
        mtx.lock();
        cout << "Highlighting the most common words in each book..." << endl;
        mtx.unlock();

        ofstream outFile("outputData/word_frequencies.txt");
        outFile << "📝 Analysis of Most Common Words per Book 📝\n";
        outFile << "==========================================\n\n";

        for (const auto& book : normalizedWordScores) {
            list<pair<string, double>> sortedWords(book.second.begin(), book.second.end());
            sortedWords.sort([](const pair<string, double>& a, const pair<string, double>& b) {
                return a.second > b.second;
            });

            list<string> topWords;
            int rank = 0;
            int maxWords = min(100, static_cast<int>(sortedWords.size()));

            outFile << "📗 Book: " << book.first << "\n";
            outFile << string(60, '-') << "\n";

            for (const auto& wordScore : sortedWords) {
                if (rank >= maxWords) break;
                topWords.push_back(wordScore.first);
                outFile << rank + 1 << ". " << left << wordScore.first << " (Frequency: " << wordScore.second << ")\n";
                rank++;
            }

            frequentWords[book.first] = topWords;
            outFile << "\n";
        }

        outFile.close();
        mtx.lock();
        cout << "Word frequency analysis saved to outputData/word_frequencies.txt" << endl << endl;
        mtx.unlock();
    }

    void analyzeBookSimilarities() {
        mtx.lock();
        cout << "Analyzing similarities between books..." << endl;
        mtx.unlock();

        ofstream outFile("outputData/book_comparison_matrix.txt");
        outFile << "🔍 Comparative Analysis Between Books\n";
        outFile << string(50, '=') << "\n\n";

        vector<thread> threads;
        vector<string> bookTitlesVec(frequentWords.size());
        transform(frequentWords.begin(), frequentWords.end(), bookTitlesVec.begin(),
                  [](const auto& pair) { return pair.first; });

        for (size_t i = 0; i < bookTitlesVec.size(); i++) {
            threads.emplace_back([this, &bookTitlesVec, i, &outFile]() {
                for (size_t j = i + 1; j < bookTitlesVec.size(); j++) {
                    string book1 = bookTitlesVec[i];
                    string book2 = bookTitlesVec[j];
                    int commonWordCount = count_if(frequentWords[book1].begin(), frequentWords[book1].end(),
                                                   [&](const string& word) {
                                                       return find(frequentWords[book2].begin(),
                                                                   frequentWords[book2].end(),
                                                                   word) != frequentWords[book2].end();
                                                   });

                    double similarityIndex = static_cast<double>(commonWordCount) / 100.0;
                    string key = book1 + " & " + book2;
                    {
                        lock_guard<mutex> lock(mtx);
                        similarityScores[key] = similarityIndex;
                        outFile << "Pair: " << book1 << " & " << book2 << "\nSimilarity Index: "
                                << similarityIndex << "\n" << string(40, '-') << "\n";
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        outFile.close();
        mtx.lock();
        cout << "Book similarity analysis saved to outputData/book_comparison_matrix.txt" << endl << endl;
        mtx.unlock();
    }

    void displayTopSimilarBooks() {
        mtx.lock();
        cout << "Finding top 10 book pairs with highest similarity scores..." << endl;
        mtx.unlock();

        vector<pair<string, double>> sortedPairs(similarityScores.begin(), similarityScores.end());
        sort(sortedPairs.begin(), sortedPairs.end(),
             [](const pair<string, double>& a, const pair<string, double>& b) {
                 return a.second > b.second;
             });

        ofstream outFile("outputData/top_similar_books.txt");
        outFile << "📚 Top 10 Most Similar Books 📚\n";
        outFile << string(30, '-') << "\n";

        for (int i = 0; i < min(10, static_cast<int>(sortedPairs.size())); i++) {
            outFile << i + 1 << ". " << sortedPairs[i].first
                    << " (Score: " << sortedPairs[i].second << ")\n";
        }

        outFile.close();
        mtx.lock();
        cout << "Top similar books list saved to outputData/top_similar_books.txt" << endl << endl;
        mtx.unlock();
    }
};

int main() {
    BookAnalysis analysis;

    analysis.loadBooks(bookNames);
    analysis.countWordOccurrences();
    analysis.computeNormalizedWordScores();
    analysis.highlightFrequentWords();
    analysis.analyzeBookSimilarities();
    analysis.displayTopSimilarBooks();

    return 0;
}
