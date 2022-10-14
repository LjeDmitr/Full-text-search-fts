#include <parser/parser.hpp>

using namespace std;

int main() {
	string text = "Dr. Jekyll and Mr. Hyde";
	vector<string> stop_words = {"and", "dr", "mr"}; 
	int ngram_min_length = 3;
	int ngram_max_length = 6;
	cout << parseStr(text, stop_words, ngram_min_length, ngram_max_length) << endl;

	return 0;
}