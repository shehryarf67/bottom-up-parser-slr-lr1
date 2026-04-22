#include "grammar.h"

#include <cctype>
#include <fstream>
#include <sstream>

std::string Grammar::trim(const std::string& s) {
    std::size_t l = 0;
    while (l < s.size() && std::isspace(static_cast<unsigned char>(s[l])) != 0) {
        ++l;
    }
    std::size_t r = s.size();
    while (r > l && std::isspace(static_cast<unsigned char>(s[r - 1])) != 0) {
        --r;
    }
    return s.substr(l, r - l);
}

std::vector<std::string> Grammar::splitBy(const std::string& text, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(text);
    std::string part;
    while (std::getline(ss, part, delim)) {
        parts.push_back(trim(part));
    }
    return parts;
}

std::vector<std::string> Grammar::splitInputTokens(const std::string& s) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> Grammar::tokenizeAlternative(const std::string& alt) const {
    std::vector<std::string> tokens = splitInputTokens(alt);
    if (tokens.empty()) {
        return {};
    }
    if (tokens.size() == 1 && (tokens[0] == "epsilon" || tokens[0] == EPSILON)) {
        return {};
    }
    return tokens;
}

bool Grammar::loadFromFile(const std::string& path, std::string& error) {
    productions.clear();
    productionsByLhs.clear();
    terminals.clear();
    nonTerminals.clear();
    nonTerminalOrder.clear();
    firstSets.clear();
    followSets.clear();
    originalStartSymbol.clear();
    augmentedStartSymbol.clear();
    augmented = false;

    std::ifstream in(path);
    if (!in.is_open()) {
        error = "Unable to open grammar file: " + path;
        return false;
    }

    std::vector<std::pair<std::string, std::string>> raw;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::size_t pos = line.find("->");
        if (pos == std::string::npos) {
            error = "Invalid production line: " + line;
            return false;
        }
        std::string lhs = trim(line.substr(0, pos));
        std::string rhs = trim(line.substr(pos + 2));
        if (lhs.empty()) {
            error = "Invalid production line with empty LHS.";
            return false;
        }
        if (rhs.empty()) {
            rhs = EPSILON;
        }
        if (nonTerminals.insert(lhs).second) {
            nonTerminalOrder.push_back(lhs);
        }
        raw.push_back({lhs, rhs});
    }

    if (raw.empty()) {
        error = "Grammar file is empty.";
        return false;
    }

    originalStartSymbol = raw.front().first;
    int id = 0;
    for (const auto& entry : raw) {
        std::vector<std::string> alts = splitBy(entry.second, '|');
        for (const std::string& alt : alts) {
            Production p;
            p.id = id++;
            p.lhs = entry.first;
            p.rhs = tokenizeAlternative(alt);
            productions.push_back(p);
        }
    }

    for (const Production& p : productions) {
        for (const std::string& sym : p.rhs) {
            if (nonTerminals.find(sym) == nonTerminals.end()) {
                terminals.insert(sym);
            }
        }
    }

    rebuildIndexes();
    return true;
}

void Grammar::rebuildIndexes() {
    productionsByLhs.clear();
    for (std::size_t i = 0; i < productions.size(); ++i) {
        productions[i].id = static_cast<int>(i);
        productionsByLhs[productions[i].lhs].push_back(static_cast<int>(i));
    }
}

void Grammar::augmentGrammar() {
    if (augmented) {
        return;
    }
    augmentedStartSymbol = originalStartSymbol + "Prime";
    while (nonTerminals.count(augmentedStartSymbol) != 0 || terminals.count(augmentedStartSymbol) != 0) {
        augmentedStartSymbol += "Prime";
    }
    Production p;
    p.lhs = augmentedStartSymbol;
    p.rhs = {originalStartSymbol};
    productions.insert(productions.begin(), p);
    nonTerminals.insert(augmentedStartSymbol);
    nonTerminalOrder.insert(nonTerminalOrder.begin(), augmentedStartSymbol);
    rebuildIndexes();
    augmented = true;
}

bool Grammar::isNonTerminal(const std::string& symbol) const {
    return nonTerminals.count(symbol) != 0;
}

bool Grammar::isTerminal(const std::string& symbol) const {
    return symbol == END_MARKER || terminals.count(symbol) != 0;
}

void Grammar::computeFirstSets() {
    firstSets.clear();
    firstSets[END_MARKER].insert(END_MARKER);
    for (const std::string& t : terminals) {
        firstSets[t].insert(t);
    }
    for (const std::string& nt : nonTerminals) {
        firstSets[nt];
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production& p : productions) {
            if (p.rhs.empty()) {
                if (firstSets[p.lhs].insert(EPSILON).second) {
                    changed = true;
                }
                continue;
            }
            bool allNullable = true;
            for (const std::string& sym : p.rhs) {
                std::set<std::string> s;
                if (isNonTerminal(sym)) {
                    s = firstSets[sym];
                } else {
                    s.insert(sym);
                }
                for (const std::string& v : s) {
                    if (v != EPSILON && firstSets[p.lhs].insert(v).second) {
                        changed = true;
                    }
                }
                if (s.count(EPSILON) == 0) {
                    allNullable = false;
                    break;
                }
            }
            if (allNullable && firstSets[p.lhs].insert(EPSILON).second) {
                changed = true;
            }
        }
    }
}

std::set<std::string> Grammar::firstOfSequence(const std::vector<std::string>& sequence) const {
    std::set<std::string> result;
    if (sequence.empty()) {
        result.insert(EPSILON);
        return result;
    }
    bool allNullable = true;
    for (const std::string& sym : sequence) {
        std::set<std::string> s;
        if (isNonTerminal(sym)) {
            auto it = firstSets.find(sym);
            if (it != firstSets.end()) {
                s = it->second;
            }
        } else {
            s.insert(sym);
        }
        for (const std::string& v : s) {
            if (v != EPSILON) {
                result.insert(v);
            }
        }
        if (s.count(EPSILON) == 0) {
            allNullable = false;
            break;
        }
    }
    if (allNullable) {
        result.insert(EPSILON);
    }
    return result;
}

void Grammar::computeFollowSets() {
    followSets.clear();
    for (const std::string& nt : nonTerminals) {
        followSets[nt];
    }
    followSets[originalStartSymbol].insert(END_MARKER);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production& p : productions) {
            for (std::size_t i = 0; i < p.rhs.size(); ++i) {
                const std::string& B = p.rhs[i];
                if (!isNonTerminal(B)) {
                    continue;
                }
                std::vector<std::string> beta(p.rhs.begin() + static_cast<long>(i + 1), p.rhs.end());
                std::set<std::string> firstBeta = firstOfSequence(beta);
                for (const std::string& x : firstBeta) {
                    if (x != EPSILON && followSets[B].insert(x).second) {
                        changed = true;
                    }
                }
                if (beta.empty() || firstBeta.count(EPSILON) != 0) {
                    for (const std::string& x : followSets[p.lhs]) {
                        if (followSets[B].insert(x).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

std::string Grammar::productionToString(const Production& p, bool withId) const {
    std::ostringstream out;
    if (withId) {
        out << '(' << p.id << ") ";
    }
    out << p.lhs << " -> ";
    if (p.rhs.empty()) {
        out << EPSILON;
    } else {
        for (std::size_t i = 0; i < p.rhs.size(); ++i) {
            out << p.rhs[i];
            if (i + 1 < p.rhs.size()) {
                out << ' ';
            }
        }
    }
    return out.str();
}

std::string Grammar::itemToString(int productionId, std::size_t dotPosition) const {
    const Production& p = productions.at(static_cast<std::size_t>(productionId));
    std::ostringstream out;
    out << p.lhs << " -> ";
    if (p.rhs.empty()) {
        out << (dotPosition == 0 ? "• @" : "@ •");
        return out.str();
    }
    for (std::size_t i = 0; i <= p.rhs.size(); ++i) {
        if (i == dotPosition) {
            out << "•";
            if (i < p.rhs.size()) {
                out << ' ';
            }
        }
        if (i < p.rhs.size()) {
            out << p.rhs[i];
            if (i + 1 < p.rhs.size() || i + 1 == dotPosition) {
                out << ' ';
            }
        }
    }
    return trim(out.str());
}

std::string Grammar::serializeGrammar() const {
    std::ostringstream out;
    out << "Original Start Symbol: " << originalStartSymbol << '\n';
    out << "Augmented Start Symbol: " << augmentedStartSymbol << "\n\n";
    for (const Production& p : productions) {
        out << productionToString(p, true) << '\n';
    }
    return out.str();
}

std::vector<std::string> Grammar::getAllSymbols() const {
    std::vector<std::string> out(terminals.begin(), terminals.end());
    for (const std::string& nt : nonTerminalOrder) {
        out.push_back(nt);
    }
    return out;
}

std::vector<std::string> Grammar::getTerminalsForTable() const {
    std::vector<std::string> out(terminals.begin(), terminals.end());
    out.push_back(END_MARKER);
    return out;
}

std::vector<std::string> Grammar::getNonTerminalsForTable() const {
    std::vector<std::string> out;
    for (const std::string& nt : nonTerminalOrder) {
        if (nt != augmentedStartSymbol) {
            out.push_back(nt);
        }
    }
    return out;
}
