#include "parsing_table.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

std::string ParsingTable::join(const std::vector<std::string>& values, const std::string& delim) {
    std::ostringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        out << values[i];
        if (i + 1 < values.size()) {
            out << delim;
        }
    }
    return out.str();
}

void ParsingTable::registerConflictIfNeeded(int state, const std::string& symbol) {
    auto itRow = actions.find(state);
    if (itRow == actions.end()) {
        return;
    }
    auto itCell = itRow->second.find(symbol);
    if (itCell == itRow->second.end()) {
        return;
    }
    std::vector<std::string>& entries = itCell->second;
    std::sort(entries.begin(), entries.end());
    entries.erase(std::unique(entries.begin(), entries.end()), entries.end());
    if (entries.size() <= 1) {
        return;
    }
    for (const Conflict& c : conflicts) {
        if (c.state == state && c.symbol == symbol) {
            return;
        }
    }
    bool hasShift = false;
    bool hasReduce = false;
    bool hasAccept = false;
    for (const std::string& entry : entries) {
        if (!entry.empty() && entry[0] == 's') {
            hasShift = true;
        } else if (!entry.empty() && entry[0] == 'r') {
            hasReduce = true;
        } else if (entry == "acc") {
            hasAccept = true;
        }
    }
    std::string type = "General Conflict";
    if (hasShift && hasReduce) {
        type = "Shift/Reduce Conflict";
    } else if (hasReduce) {
        type = "Reduce/Reduce Conflict";
    } else if (hasAccept) {
        type = "Accept Conflict";
    }
    conflicts.push_back({state, symbol, type, entries});
}

void ParsingTable::addAction(int state, const std::string& symbol, const std::string& action) {
    actions[state][symbol].push_back(action);
    registerConflictIfNeeded(state, symbol);
}

void ParsingTable::setGoto(int state, const std::string& symbol, int targetState) {
    gotos[state][symbol] = targetState;
}

std::vector<std::string> ParsingTable::getAction(int state, const std::string& symbol) const {
    auto itRow = actions.find(state);
    if (itRow == actions.end()) {
        return {};
    }
    auto itCell = itRow->second.find(symbol);
    if (itCell == itRow->second.end()) {
        return {};
    }
    return itCell->second;
}

int ParsingTable::getGoto(int state, const std::string& symbol) const {
    auto itRow = gotos.find(state);
    if (itRow == gotos.end()) {
        return -1;
    }
    auto itCell = itRow->second.find(symbol);
    if (itCell == itRow->second.end()) {
        return -1;
    }
    return itCell->second;
}

std::size_t ParsingTable::filledActionCells() const {
    std::size_t n = 0;
    for (const auto& row : actions) {
        n += row.second.size();
    }
    return n;
}

std::size_t ParsingTable::filledGotoCells() const {
    std::size_t n = 0;
    for (const auto& row : gotos) {
        n += row.second.size();
    }
    return n;
}

std::size_t ParsingTable::estimatedSerializedSizeBytes() const {
    return serialize().size();
}

std::string ParsingTable::serialize() const {
    std::vector<std::size_t> widths;
    widths.push_back(8);
    for (const std::string& s : actionSymbols) {
        widths.push_back(std::max<std::size_t>(14, s.size() + 2));
    }
    for (const std::string& s : gotoSymbols) {
        widths.push_back(std::max<std::size_t>(14, s.size() + 2));
    }

    std::ostringstream out;
    out << std::left << std::setw(static_cast<int>(widths[0])) << "State";
    std::size_t wi = 1;
    for (const std::string& s : actionSymbols) {
        out << std::left << std::setw(static_cast<int>(widths[wi++])) << s;
    }
    for (const std::string& s : gotoSymbols) {
        out << std::left << std::setw(static_cast<int>(widths[wi++])) << s;
    }
    out << '\n';

    for (int st = 0; st < numStates; ++st) {
        std::ostringstream label;
        label << 'I' << st;
        out << std::left << std::setw(static_cast<int>(widths[0])) << label.str();
        wi = 1;
        for (const std::string& s : actionSymbols) {
            std::string cell;
            auto itRow = actions.find(st);
            if (itRow != actions.end()) {
                auto itCell = itRow->second.find(s);
                if (itCell != itRow->second.end()) {
                    cell = join(itCell->second, " / ");
                }
            }
            out << std::left << std::setw(static_cast<int>(widths[wi++])) << cell;
        }
        for (const std::string& s : gotoSymbols) {
            std::string cell;
            auto itRow = gotos.find(st);
            if (itRow != gotos.end()) {
                auto itCell = itRow->second.find(s);
                if (itCell != itRow->second.end()) {
                    cell = std::to_string(itCell->second);
                }
            }
            out << std::left << std::setw(static_cast<int>(widths[wi++])) << cell;
        }
        out << '\n';
    }

    if (conflicts.empty()) {
        out << "\nNo conflicts detected.\n";
    } else {
        out << "\nConflicts:\n";
        for (const Conflict& c : conflicts) {
            out << "  State I" << c.state << ", Symbol '" << c.symbol << "': " << c.type
                << " => " << join(c.entries, " / ") << '\n';
        }
    }
    return out.str();
}
