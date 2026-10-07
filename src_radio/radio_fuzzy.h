#pragma once
// Radio mode -- typo tolerant search, the same scoring as the music player's search (fuzzy_score() in src/app.cpp, which
// lives in that file's anonymous namespace, so it is repeated here; keep the two in sync).
//
//   tier 1: the query is a literal substring (after lower-casing and folding - _ . / to spaces): 1000 minus its position,
//           so an exact hit always outranks a fuzzy one and earlier hits rank higher
//   tier 2: every query word must have a word in the target within edit distance (quality >= 0.55); the score is the
//           mean quality * 100
//   negative = not a match
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace muisc::radio {

inline std::string fuzzy_normalize(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        else if (c == '-' || c == '_' || c == '.' || c == '/') c = ' ';
    }
    return s;
}

inline std::vector<std::string> fuzzy_words(const std::string& s) {
    std::vector<std::string> w;
    std::string cur;
    for (char c : s) {
        if (c == ' ' || c == '\t') { if (!cur.empty()) { w.push_back(cur); cur.clear(); } }
        else cur += c;
    }
    if (!cur.empty()) w.push_back(cur);
    return w;
}

inline int fuzzy_levenshtein(const std::string& a, const std::string& b) {
    const size_t n = a.size(), m = b.size();
    std::vector<int> prev(m + 1), cur(m + 1);
    for (size_t j = 0; j <= m; ++j) prev[j] = static_cast<int>(j);
    for (size_t i = 1; i <= n; ++i) {
        cur[0] = static_cast<int>(i);
        for (size_t j = 1; j <= m; ++j) {
            const int cost = a[i - 1] == b[j - 1] ? 0 : 1;
            cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost});
        }
        std::swap(prev, cur);
    }
    return prev[m];
}

inline double fuzzy_score(const std::string& query, const std::string& target) {
    const std::string q = fuzzy_normalize(query), t = fuzzy_normalize(target);
    if (q.empty()) return 0.0;
    const size_t pos = t.find(q);
    if (pos != std::string::npos) return 1000.0 - std::min<double>(static_cast<double>(pos), 900.0);
    const auto qw = fuzzy_words(q), tw = fuzzy_words(t);
    if (qw.empty() || tw.empty()) return -1.0;
    double total = 0.0;
    for (const auto& a : qw) {
        double best = -1.0;
        for (const auto& b : tw) {
            const size_t mx = std::max(a.size(), b.size());
            if (mx == 0) continue;
            best = std::max(best, 1.0 - static_cast<double>(fuzzy_levenshtein(a, b)) / static_cast<double>(mx));
        }
        if (best < 0.55) return -1.0;
        total += best;
    }
    return total / static_cast<double>(qw.size()) * 100.0;
}

} // namespace muisc::radio
