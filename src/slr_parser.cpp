#include "slr_parser.h"

#include "stack.h"

#include <chrono>
#include <fstream>
#include <sstream>

namespace {
    std::string firstAndFollowDump(const Grammar& grammar) {
        std::ostringstream out;
        out << grammar.serializeGrammar() << "\nFIRST Sets:\n";
        for (const auto& entry : grammar.getFirstSets()) {
            if (!grammar.isNonTerminal(entry.first)) {
                continue;
            }
            out << "FIRST(" << entry.first << ") = { ";
            bool first = true;
            for (const std::string& s : entry.second) {
                if (!first) out << ", ";
                out << s;
                first = false;
            }
            out << " }\n";
        }
        out << "\nFOLLOW Sets:\n";
        for (const auto& entry : grammar.getFollowSets()) {
            out << "FOLLOW(" << entry.first << ") = { ";
            bool first = true;
            for (const std::string& s : entry.second) {
                if (!first) out << ", ";
                out << s;
                first = false;
            }
            out << " }\n";
        }
        return out.str();
    }

    ParseResult parseWithTable(const Grammar& grammar, const ParsingTable& table, const std::vector<std::string>& rawTokens) {
        ParseResult result;
        auto started = std::chrono::high_resolution_clock::now();
        std::vector<std::string> tokens = rawTokens;
        tokens.push_back(Grammar::END_MARKER);
        ParseStack stack;
        std::size_t inputIndex = 0;
        int step = 1;

        auto remainingInput = [&](std::size_t start) {
            std::ostringstream out;
            for (std::size_t i = start; i < tokens.size(); ++i) {
                out << tokens[i];
                if (i + 1 < tokens.size()) {
                    out << ' ';
                }
            }
            return out.str();
        };

        result.traceLines.push_back("Step\tStack\tInput\tAction");

        while (true) {
            int state = stack.topState();
            const std::string& lookahead = tokens[inputIndex];
            std::vector<std::string> actions = table.getAction(state, lookahead);
            std::ostringstream actionDesc;

            if (actions.empty()) {
                actionDesc << "error";
                std::ostringstream line;
                line << step << '\t' << stack.toString() << '\t' << remainingInput(inputIndex) << '\t' << actionDesc.str();
                result.traceLines.push_back(line.str());
                result.accepted = false;
                result.message = "Parsing error at token '" + lookahead + "' in state I" + std::to_string(state) + ".";
                break;
            }
            if (actions.size() > 1) {
                actionDesc << "conflict(";
                for (std::size_t i = 0; i < actions.size(); ++i) {
                    actionDesc << actions[i];
                    if (i + 1 < actions.size()) {
                        actionDesc << ',';
                    }
                }
                actionDesc << ')';
                std::ostringstream line;
                line << step << '\t' << stack.toString() << '\t' << remainingInput(inputIndex) << '\t' << actionDesc.str();
                result.traceLines.push_back(line.str());
                result.accepted = false;
                result.message = "Parsing halted because of an unresolved table conflict at state I" + std::to_string(state) + " on symbol '" + lookahead + "'.";
                break;
            }

            const std::string& action = actions.front();
            if (action == "acc") {
                actionDesc << "accept";
                std::ostringstream line;
                line << step << '\t' << stack.toString() << '\t' << remainingInput(inputIndex) << '\t' << actionDesc.str();
                result.traceLines.push_back(line.str());
                result.accepted = true;
                result.message = "String accepted successfully.";
                if (stack.symbolCount() == 1) {
                    result.root = stack.topNode();
                }
                break;
            }
            if (!action.empty() && action[0] == 's') {
                int nextState = std::stoi(action.substr(1));
                stack.push(lookahead, nextState, ParseTree::makeLeaf(lookahead));
                actionDesc << "shift " << nextState;
                std::ostringstream line;
                line << step << '\t' << stack.toString() << '\t' << remainingInput(inputIndex + 1) << '\t' << actionDesc.str();
                result.traceLines.push_back(line.str());
                ++inputIndex;
            } else if (!action.empty() && action[0] == 'r') {
                int pid = std::stoi(action.substr(1));
                const Production& p = grammar.getProductions().at(static_cast<std::size_t>(pid));
                std::vector<std::shared_ptr<ParseTreeNode>> children;
                if (!p.rhs.empty()) {
                    children = stack.popForReduction(p.rhs.size());
                } else {
                    children.push_back(ParseTree::makeLeaf(Grammar::EPSILON));
                }
                auto parent = ParseTree::makeNode(p.lhs, children);
                int gotoState = table.getGoto(stack.topState(), p.lhs);
                if (gotoState == -1) {
                    result.accepted = false;
                    result.message = "Missing GOTO entry after reducing by " + grammar.productionToString(p) + ".";
                    break;
                }
                stack.push(p.lhs, gotoState, parent);
                actionDesc << "reduce " << grammar.productionToString(p);
                result.reductions.push_back(grammar.productionToString(p));
                std::ostringstream line;
                line << step << '\t' << stack.toString() << '\t' << remainingInput(inputIndex) << '\t' << actionDesc.str();
                result.traceLines.push_back(line.str());
            } else {
                result.accepted = false;
                result.message = "Unknown parser action: " + action;
                break;
            }
            ++step;
        }

        auto ended = std::chrono::high_resolution_clock::now();
        result.parseTimeMs = std::chrono::duration<double, std::milli>(ended - started).count();
        return result;
    }
}

bool SLRParser::initialize(const std::string& grammarPath, std::string& error) {
    if (!grammar.loadFromFile(grammarPath, error)) {
        return false;
    }
    grammar.augmentGrammar();
    grammar.computeFirstSets();
    grammar.computeFollowSets();
    return true;
}

void SLRParser::build() {
    auto started = std::chrono::high_resolution_clock::now();
    states = ItemUtils::buildCanonicalLR0(grammar, transitions);
    table = ParsingTable();
    table.setNumStates(static_cast<int>(states.size()));
    table.setActionSymbols(grammar.getTerminalsForTable());
    table.setGotoSymbols(grammar.getNonTerminalsForTable());

    const auto& productions = grammar.getProductions();
    const auto& follow = grammar.getFollowSets();

    for (std::size_t i = 0; i < states.size(); ++i) {
        for (const LR0Item& item : states[i]) {
            const Production& p = productions.at(static_cast<std::size_t>(item.productionId));
            if (item.dotPosition < p.rhs.size()) {
                const std::string& sym = p.rhs[item.dotPosition];
                auto tr = transitions.find({static_cast<int>(i), sym});
                if (tr != transitions.end()) {
                    if (grammar.isTerminal(sym)) {
                        table.addAction(static_cast<int>(i), sym, "s" + std::to_string(tr->second));
                    } else if (sym != grammar.getAugmentedStartSymbol()) {
                        table.setGoto(static_cast<int>(i), sym, tr->second);
                    }
                }
            } else {
                if (p.lhs == grammar.getAugmentedStartSymbol()) {
                    table.addAction(static_cast<int>(i), Grammar::END_MARKER, "acc");
                } else {
                    std::string action = "r" + std::to_string(p.id);
                    auto it = follow.find(p.lhs);
                    if (it != follow.end()) {
                        for (const std::string& la : it->second) {
                            table.addAction(static_cast<int>(i), la, action);
                        }
                    }
                }
            }
        }
    }

    auto ended = std::chrono::high_resolution_clock::now();
    buildTimeMs = std::chrono::duration<double, std::milli>(ended - started).count();
}

ParseResult SLRParser::parseTokens(const std::vector<std::string>& tokens) const {
    return parseWithTable(grammar, table, tokens);
}

std::vector<std::pair<std::string, ParseResult>> SLRParser::parseFile(const std::string& inputPath) const {
    std::vector<std::pair<std::string, ParseResult>> results;
    std::ifstream in(inputPath);
    if (!in.is_open()) {
        results.push_back({"", ParseResult{false, {}, nullptr, "Unable to open input file: " + inputPath, 0.0, {}}});
        return results;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = Grammar::trim(line);
        if (trimmed.empty()) {
            results.push_back({"<empty input>", parseTokens({})});
            continue;
        }
        if (trimmed[0] == '#') {
            continue;
        }
        results.push_back({trimmed, parseTokens(Grammar::splitInputTokens(trimmed))});
    }
    if (results.empty()) {
        results.push_back({"<empty input>", parseTokens({})});
    }
    return results;
}

std::string SLRParser::serializeItems() const { return ItemUtils::serializeLR0States(grammar, states); }
std::string SLRParser::serializeTable() const { std::ostringstream out; out << table.serialize() << "Grammar classification: " << (table.hasConflicts() ? "Not SLR(1)" : "SLR(1)") << '\n'; return out.str(); }
std::string SLRParser::serializeGrammarSummary() const { return firstAndFollowDump(grammar); }

ParserMetrics SLRParser::getMetrics() const {
    ParserMetrics m;
    m.stateCount = static_cast<int>(states.size());
    m.filledActionCells = table.filledActionCells();
    m.filledGotoCells = table.filledGotoCells();
    m.estimatedTableBytes = table.estimatedSerializedSizeBytes();
    m.buildTimeMs = buildTimeMs;
    m.conflictCount = static_cast<int>(table.getConflicts().size());
    return m;
}
