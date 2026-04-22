#ifndef STACK_H
#define STACK_H

#include <memory>
#include <string>
#include <vector>

struct ParseTreeNode;

class ParseStack {
public:
    ParseStack();
    void reset();
    void push(const std::string& symbol, int state, const std::shared_ptr<ParseTreeNode>& node);
    std::vector<std::shared_ptr<ParseTreeNode>> popForReduction(std::size_t count);
    int topState() const;
    std::size_t symbolCount() const;
    std::shared_ptr<ParseTreeNode> topNode() const;
    std::string toString() const;

private:
    std::vector<int> states;
    std::vector<std::string> symbols;
    std::vector<std::shared_ptr<ParseTreeNode>> nodes;
};

#endif
