#ifndef ITEMS_H
#define ITEMS_H

#include "grammar.h"

#include <map>
#include <set>
#include <string>
#include <vector>

struct LR0Item {
    int productionId = -1;
    std::size_t dotPosition = 0;
    bool operator<(const LR0Item& other) const;
    bool operator==(const LR0Item& other) const;
};

struct LR1Item {
    int productionId = -1;
    std::size_t dotPosition = 0;
    std::string lookahead;
    bool operator<(const LR1Item& other) const;
    bool operator==(const LR1Item& other) const;
};

namespace ItemUtils {
    std::set<LR0Item> closureLR0(const Grammar& grammar, const std::set<LR0Item>& items);
    std::set<LR0Item> gotoLR0(const Grammar& grammar, const std::set<LR0Item>& items, const std::string& symbol);
    std::vector<std::set<LR0Item>> buildCanonicalLR0(const Grammar& grammar, std::map<std::pair<int, std::string>, int>& transitions);

    std::set<LR1Item> closureLR1(const Grammar& grammar, const std::set<LR1Item>& items);
    std::set<LR1Item> gotoLR1(const Grammar& grammar, const std::set<LR1Item>& items, const std::string& symbol);
    std::vector<std::set<LR1Item>> buildCanonicalLR1(const Grammar& grammar, std::map<std::pair<int, std::string>, int>& transitions);

    std::string toString(const Grammar& grammar, const LR0Item& item);
    std::string toString(const Grammar& grammar, const LR1Item& item);
    std::string serializeLR0States(const Grammar& grammar, const std::vector<std::set<LR0Item>>& states);
    std::string serializeLR1States(const Grammar& grammar, const std::vector<std::set<LR1Item>>& states);
}

#endif
