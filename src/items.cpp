#include "items.h"

#include <queue>
#include <sstream>

bool LR0Item::operator<(const LR0Item& other) const {
    if (productionId != other.productionId) {
        return productionId < other.productionId;
    }
    return dotPosition < other.dotPosition;
}

bool LR0Item::operator==(const LR0Item& other) const {
    return productionId == other.productionId && dotPosition == other.dotPosition;
}

bool LR1Item::operator<(const LR1Item& other) const {
    if (productionId != other.productionId) {
        return productionId < other.productionId;
    }
    if (dotPosition != other.dotPosition) {
        return dotPosition < other.dotPosition;
    }
    return lookahead < other.lookahead;
}

bool LR1Item::operator==(const LR1Item& other) const {
    return productionId == other.productionId && dotPosition == other.dotPosition && lookahead == other.lookahead;
}

namespace {
    template <typename T>
    int findState(const std::vector<std::set<T>>& states, const std::set<T>& state) {
        for (std::size_t i = 0; i < states.size(); ++i) {
            if (states[i] == state) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
}

namespace ItemUtils {
    std::set<LR0Item> closureLR0(const Grammar& grammar, const std::set<LR0Item>& items) {
        std::set<LR0Item> closure = items;
        bool changed = true;
        const auto& productions = grammar.getProductions();
        const auto& byLhs = grammar.getProductionsByLhs();
        while (changed) {
            changed = false;
            std::vector<LR0Item> snapshot(closure.begin(), closure.end());
            for (const LR0Item& item : snapshot) {
                const Production& p = productions.at(static_cast<std::size_t>(item.productionId));
                if (item.dotPosition >= p.rhs.size()) {
                    continue;
                }
                const std::string& next = p.rhs[item.dotPosition];
                if (!grammar.isNonTerminal(next)) {
                    continue;
                }
                auto it = byLhs.find(next);
                if (it == byLhs.end()) {
                    continue;
                }
                for (int pid : it->second) {
                    if (closure.insert({pid, 0}).second) {
                        changed = true;
                    }
                }
            }
        }
        return closure;
    }

    std::set<LR0Item> gotoLR0(const Grammar& grammar, const std::set<LR0Item>& items, const std::string& symbol) {
        std::set<LR0Item> moved;
        const auto& productions = grammar.getProductions();
        for (const LR0Item& item : items) {
            const Production& p = productions.at(static_cast<std::size_t>(item.productionId));
            if (item.dotPosition < p.rhs.size() && p.rhs[item.dotPosition] == symbol) {
                moved.insert({item.productionId, item.dotPosition + 1});
            }
        }
        if (moved.empty()) {
            return moved;
        }
        return closureLR0(grammar, moved);
    }

    std::vector<std::set<LR0Item>> buildCanonicalLR0(const Grammar& grammar, std::map<std::pair<int, std::string>, int>& transitions) {
        transitions.clear();
        std::vector<std::set<LR0Item>> states;
        std::queue<int> q;
        states.push_back(closureLR0(grammar, {{0, 0}}));
        q.push(0);
        std::vector<std::string> symbols = grammar.getAllSymbols();
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            for (const std::string& sym : symbols) {
                std::set<LR0Item> next = gotoLR0(grammar, states[current], sym);
                if (next.empty()) {
                    continue;
                }
                int idx = findState(states, next);
                if (idx == -1) {
                    idx = static_cast<int>(states.size());
                    states.push_back(next);
                    q.push(idx);
                }
                transitions[{current, sym}] = idx;
            }
        }
        return states;
    }

    std::set<LR1Item> closureLR1(const Grammar& grammar, const std::set<LR1Item>& items) {
        std::set<LR1Item> closure = items;
        bool changed = true;
        const auto& productions = grammar.getProductions();
        const auto& byLhs = grammar.getProductionsByLhs();
        while (changed) {
            changed = false;
            std::vector<LR1Item> snapshot(closure.begin(), closure.end());
            for (const LR1Item& item : snapshot) {
                const Production& p = productions.at(static_cast<std::size_t>(item.productionId));
                if (item.dotPosition >= p.rhs.size()) {
                    continue;
                }
                const std::string& B = p.rhs[item.dotPosition];
                if (!grammar.isNonTerminal(B)) {
                    continue;
                }
                std::vector<std::string> beta(p.rhs.begin() + static_cast<long>(item.dotPosition + 1), p.rhs.end());
                beta.push_back(item.lookahead);
                std::set<std::string> lookaheads = grammar.firstOfSequence(beta);
                auto it = byLhs.find(B);
                if (it == byLhs.end()) {
                    continue;
                }
                for (int pid : it->second) {
                    for (const std::string& la : lookaheads) {
                        if (la == Grammar::EPSILON) {
                            continue;
                        }
                        if (closure.insert({pid, 0, la}).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
        return closure;
    }

    std::set<LR1Item> gotoLR1(const Grammar& grammar, const std::set<LR1Item>& items, const std::string& symbol) {
        std::set<LR1Item> moved;
        const auto& productions = grammar.getProductions();
        for (const LR1Item& item : items) {
            const Production& p = productions.at(static_cast<std::size_t>(item.productionId));
            if (item.dotPosition < p.rhs.size() && p.rhs[item.dotPosition] == symbol) {
                moved.insert({item.productionId, item.dotPosition + 1, item.lookahead});
            }
        }
        if (moved.empty()) {
            return moved;
        }
        return closureLR1(grammar, moved);
    }

    std::vector<std::set<LR1Item>> buildCanonicalLR1(const Grammar& grammar, std::map<std::pair<int, std::string>, int>& transitions) {
        transitions.clear();
        std::vector<std::set<LR1Item>> states;
        std::queue<int> q;
        states.push_back(closureLR1(grammar, {{0, 0, Grammar::END_MARKER}}));
        q.push(0);
        std::vector<std::string> symbols = grammar.getAllSymbols();
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            for (const std::string& sym : symbols) {
                std::set<LR1Item> next = gotoLR1(grammar, states[current], sym);
                if (next.empty()) {
                    continue;
                }
                int idx = findState(states, next);
                if (idx == -1) {
                    idx = static_cast<int>(states.size());
                    states.push_back(next);
                    q.push(idx);
                }
                transitions[{current, sym}] = idx;
            }
        }
        return states;
    }

    std::string toString(const Grammar& grammar, const LR0Item& item) {
        return grammar.itemToString(item.productionId, item.dotPosition);
    }

    std::string toString(const Grammar& grammar, const LR1Item& item) {
        std::ostringstream out;
        out << '[' << grammar.itemToString(item.productionId, item.dotPosition) << ", " << item.lookahead << ']';
        return out.str();
    }

    std::string serializeLR0States(const Grammar& grammar, const std::vector<std::set<LR0Item>>& states) {
        std::ostringstream out;
        for (std::size_t i = 0; i < states.size(); ++i) {
            out << "I" << i << ":\n";
            for (const LR0Item& item : states[i]) {
                out << "  " << toString(grammar, item) << '\n';
            }
            out << '\n';
        }
        return out.str();
    }

    std::string serializeLR1States(const Grammar& grammar, const std::vector<std::set<LR1Item>>& states) {
        std::ostringstream out;
        for (std::size_t i = 0; i < states.size(); ++i) {
            out << "I" << i << ":\n";
            for (const LR1Item& item : states[i]) {
                out << "  " << toString(grammar, item) << '\n';
            }
            out << '\n';
        }
        return out.str();
    }
}
