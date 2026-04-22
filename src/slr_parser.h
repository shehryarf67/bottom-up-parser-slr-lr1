#ifndef SLR_PARSER_H
#define SLR_PARSER_H

#include "grammar.h"
#include "items.h"
#include "parsing_table.h"
#include "tree.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct ParserMetrics {
    int stateCount = 0;
    std::size_t filledActionCells = 0;
    std::size_t filledGotoCells = 0;
    std::size_t estimatedTableBytes = 0;
    double buildTimeMs = 0.0;
    int conflictCount = 0;
};

class SLRParser {
public:
    bool initialize(const std::string& grammarPath, std::string& error);
    void build();
    ParseResult parseTokens(const std::vector<std::string>& tokens) const;
    std::vector<std::pair<std::string, ParseResult>> parseFile(const std::string& inputPath) const;
    std::string serializeItems() const;
    std::string serializeTable() const;
    std::string serializeGrammarSummary() const;
    ParserMetrics getMetrics() const;
    const ParsingTable& getTable() const { return table; }

private:
    Grammar grammar;
    std::vector<std::set<LR0Item>> states;
    std::map<std::pair<int, std::string>, int> transitions;
    ParsingTable table;
    double buildTimeMs = 0.0;
};

#endif
