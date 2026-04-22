#ifndef LR1_PARSER_H
#define LR1_PARSER_H

#include "grammar.h"
#include "items.h"
#include "parsing_table.h"
#include "slr_parser.h"
#include "tree.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class LR1Parser {
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
    std::vector<std::set<LR1Item>> states;
    std::map<std::pair<int, std::string>, int> transitions;
    ParsingTable table;
    double buildTimeMs = 0.0;
};

#endif
