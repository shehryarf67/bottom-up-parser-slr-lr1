#include "stack.h"

#include "tree.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

ParseStack::ParseStack() { reset(); }

void ParseStack::reset() {
    states.clear();
    symbols.clear();
    nodes.clear();
    states.push_back(0);
}

void ParseStack::push(const std::string& symbol, int state, const std::shared_ptr<ParseTreeNode>& node) {
    symbols.push_back(symbol);
    states.push_back(state);
    nodes.push_back(node);
}

std::vector<std::shared_ptr<ParseTreeNode>> ParseStack::popForReduction(std::size_t count) {
    if (count > symbols.size()) {
        throw std::runtime_error("Invalid reduction pop count.");
    }
    std::vector<std::shared_ptr<ParseTreeNode>> children;
    for (std::size_t i = 0; i < count; ++i) {
        children.push_back(nodes.back());
        nodes.pop_back();
        symbols.pop_back();
        states.pop_back();
    }
    std::reverse(children.begin(), children.end());
    return children;
}

int ParseStack::topState() const { return states.back(); }
std::size_t ParseStack::symbolCount() const { return symbols.size(); }
std::shared_ptr<ParseTreeNode> ParseStack::topNode() const { return nodes.empty() ? nullptr : nodes.back(); }

std::string ParseStack::toString() const {
    std::ostringstream out;
    out << states.front();
    for (std::size_t i = 0; i < symbols.size(); ++i) {
        out << ' ' << symbols[i] << ' ' << states[i + 1];
    }
    return out.str();
}
