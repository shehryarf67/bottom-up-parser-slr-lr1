#ifndef PARSING_TABLE_H
#define PARSING_TABLE_H

#include <map>
#include <string>
#include <vector>

struct Conflict {
    int state = -1;
    std::string symbol;
    std::string type;
    std::vector<std::string> entries;
};

class ParsingTable {
public:
    void setNumStates(int count) { numStates = count; }
    void setActionSymbols(const std::vector<std::string>& symbols) { actionSymbols = symbols; }
    void setGotoSymbols(const std::vector<std::string>& symbols) { gotoSymbols = symbols; }

    void addAction(int state, const std::string& symbol, const std::string& action);
    void setGoto(int state, const std::string& symbol, int targetState);

    std::vector<std::string> getAction(int state, const std::string& symbol) const;
    int getGoto(int state, const std::string& symbol) const;

    bool hasConflicts() const { return !conflicts.empty(); }
    const std::vector<Conflict>& getConflicts() const { return conflicts; }
    std::size_t filledActionCells() const;
    std::size_t filledGotoCells() const;
    std::size_t estimatedSerializedSizeBytes() const;
    std::string serialize() const;

private:
    int numStates = 0;
    std::vector<std::string> actionSymbols;
    std::vector<std::string> gotoSymbols;
    std::map<int, std::map<std::string, std::vector<std::string>>> actions;
    std::map<int, std::map<std::string, int>> gotos;
    std::vector<Conflict> conflicts;

    static std::string join(const std::vector<std::string>& values, const std::string& delim);
    void registerConflictIfNeeded(int state, const std::string& symbol);
};

#endif
