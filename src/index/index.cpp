#include <picosha2.h>
#include <fstream>
#include <index/index.hpp>
#include <parser/parser.hpp>

using namespace std;

void Index::setDocs(string key, string text) {
  docs = make_pair(key, text);
}

pair<string, string> Index::getDocs() {
  return docs;
}

void Index::setEntries(string key, vector<term> terms) {
  entries = make_pair(key, terms);
}

pair<string, vector<term>> Index::getEntries() {
  return entries;
}

void indexBuilder::setIndexes(Index index) {
  indexes.push_back(index);
}

vector<Index> indexBuilder::getIndexes() {
  return indexes;
}

static bool hashUniquenessCheck(string hash, vector<Index> indexes) {
  for (size_t i = 0; i < indexes.size(); ++i) {
    if (hash == indexes[i].getEntries().first) {
      return false;
    }
  }
  return true;
}

static term createTerm(pair<string, vector<int>> ngram, string doc_id) {
  term term_obj;
  term_obj.text = ngram.first;
  term_obj.doc_count = 1;
  term_obj.doc_id_and_pos.push_back(make_pair(doc_id, ngram.second));
  return term_obj;
}

void Index::correct_index(
    string doc_id,
    string hash,
    string term,
    vector<int> pos) {
  if (hash == entries.first) {
    for (size_t i = 0; i < entries.second.size(); i++) {
      if (entries.second[i].text == term) {
        entries.second[i].doc_id_and_pos.push_back(make_pair(doc_id, pos));
        entries.second[i].doc_count++;
      }
    }
  }
}

void indexBuilder::add_document(string document_id, string text) {
  parser parsing_string;
  parsing_string.parseStr(text);
  string hash_hex_str, prev_ngram;
  vector<term> entries_terms;
  vector<pair<string, vector<int>>> ngrams = parsing_string.getNgrams();
  for (size_t i = 0; i < ngrams.size(); i++) {
    picosha2::hash256_hex_string(ngrams[i].first.substr(0, 3), hash_hex_str);
    hash_hex_str = hash_hex_str.substr(0, 6);
    if (hashUniquenessCheck(hash_hex_str, indexes)) {
      prev_ngram = ngrams[i].first.substr(0, 3);
      while (i < ngrams.size() && ngrams[i].first.substr(0, 3) == prev_ngram) {
        entries_terms.push_back(createTerm(ngrams[i], document_id));
        i++;
      }
      i--;
      indexes.push_back(index(document_id, text, hash_hex_str, entries_terms));
      entries_terms.clear();
    } else {
      for (size_t j = 0; j < indexes.size(); j++) {
        indexes[j].correct_index(
            document_id, hash_hex_str, ngrams[i].first, ngrams[i].second);
      }
    }
  }
}

Index indexBuilder::index(
    string doc_key,
    string doc_text,
    string entries_key,
    vector<term> entries_terms) {
  Index index;
  index.setDocs(doc_key, doc_text);
  index.setEntries(entries_key, entries_terms);
  return index;
}

bool demo_exists(const fs::path& p, fs::file_status s = fs::file_status{}) {
  if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
    return true;
  else
    return false;
}

void textIndexWriter::write(string path, Index index) {
  const fs::path path_index{path};
  if (!demo_exists(path_index))
    fs::create_directory(path_index);
  const fs::path path_docs{path + "/docs"};
  if (!demo_exists(path_docs))
    fs::create_directory(path_docs);
  const fs::path path_entries{path + "/entries"};
  if (!demo_exists(path_entries))
    fs::create_directory(path_entries);
  ofstream doc(path + "/docs/" + index.getDocs().first);
  ofstream entries(path + "/entries/" + index.getEntries().first);
  doc << index.getDocs().second;
  vector<term> terms = index.getEntries().second;
  for (size_t i = 0; i < terms.size(); ++i) {
    entries << terms[i].text << " " << terms[i].doc_count << " ";
    for (size_t j = 0; j < terms[i].doc_id_and_pos.size(); ++j) {
      entries << terms[i].doc_id_and_pos[j].first << " "
              << terms[i].doc_id_and_pos[j].second.size() << " ";
      for (size_t k = 0; k < terms[i].doc_id_and_pos[j].second.size(); ++k) {
        entries << terms[i].doc_id_and_pos[j].second[k] << " ";
      }
    }
    entries << endl;
  }
  doc.close();
  entries.close();
}

string textIndexWriter::testIndex(Index index) {
  string result_index;
  vector<term> terms = index.getEntries().second;
  for (size_t i = 0; i < terms.size(); ++i) {
    result_index += terms[i].text + " " + to_string(terms[i].doc_count) + " ";
    for (size_t j = 0; j < terms[i].doc_id_and_pos.size(); ++j) {
      result_index += terms[i].doc_id_and_pos[j].first + " " +
          to_string(terms[i].doc_id_and_pos[j].second.size()) + " ";
      for (size_t k = 0; k < terms[i].doc_id_and_pos[j].second.size(); ++k) {
        result_index += to_string(terms[i].doc_id_and_pos[j].second[k]) + " ";
      }
    }
    result_index += "\n";
  }
  return result_index;
}