#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <map>
#include <set>
#include <string>
#include <vector>

struct Production {
    int id = -1;
    std::string lhs;
    std::vector<std::string> rhs;
};

class Grammar {
public:
    static constexpr const char* EPSILON = "@";
    static constexpr const char* END_MARKER = "$";

    bool loadFromFile(const std::string& path, std::string& error);
    void augmentGrammar();
    void computeFirstSets();
    void computeFollowSets();

    bool isNonTerminal(const std::string& symbol) const;
    bool isTerminal(const std::string& symbol) const;
    std::set<std::string> firstOfSequence(const std::vector<std::string>& sequence) const;

    std::string productionToString(const Production& p, bool withId = false) const;
    std::string itemToString(int productionId, std::size_t dotPosition) const;
    std::string serializeGrammar() const;

    std::vector<std::string> getAllSymbols() const;
    std::vector<std::string> getTerminalsForTable() const;
    std::vector<std::string> getNonTerminalsForTable() const;

    static std::string trim(const std::string& s);
    static std::vector<std::string> splitInputTokens(const std::string& s);

    const std::vector<Production>& getProductions() const { return productions; }
    const std::map<std::string, std::vector<int>>& getProductionsByLhs() const { return productionsByLhs; }
    const std::set<std::string>& getTerminals() const { return terminals; }
    const std::set<std::string>& getNonTerminals() const { return nonTerminals; }
    const std::vector<std::string>& getNonTerminalOrder() const { return nonTerminalOrder; }
    const std::map<std::string, std::set<std::string>>& getFirstSets() const { return firstSets; }
    const std::map<std::string, std::set<std::string>>& getFollowSets() const { return followSets; }
    const std::string& getOriginalStartSymbol() const { return originalStartSymbol; }
    const std::string& getAugmentedStartSymbol() const { return augmentedStartSymbol; }

private:
    std::vector<Production> productions;
    std::map<std::string, std::vector<int>> productionsByLhs;
    std::set<std::string> terminals;
    std::set<std::string> nonTerminals;
    std::vector<std::string> nonTerminalOrder;
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;
    std::string originalStartSymbol;
    std::string augmentedStartSymbol;
    bool augmented = false;

    static std::vector<std::string> splitBy(const std::string& text, char delim);
    std::vector<std::string> tokenizeAlternative(const std::string& alt) const;
    void rebuildIndexes();
};

#endif
