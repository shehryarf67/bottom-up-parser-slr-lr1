#ifndef TREE_H
#define TREE_H

#include <memory>
#include <string>
#include <vector>

struct ParseTreeNode {
    std::string symbol;
    std::vector<std::shared_ptr<ParseTreeNode>> children;
};

struct ParseResult {
    bool accepted = false;
    std::vector<std::string> traceLines;
    std::shared_ptr<ParseTreeNode> root;
    std::string message;
    double parseTimeMs = 0.0;
    std::vector<std::string> reductions;
};

class ParseTree {
public:
    static std::shared_ptr<ParseTreeNode> makeLeaf(const std::string& symbol);
    static std::shared_ptr<ParseTreeNode> makeNode(const std::string& symbol, const std::vector<std::shared_ptr<ParseTreeNode>>& children);
    static std::string render(const std::shared_ptr<ParseTreeNode>& root);

private:
    static void renderChild(const std::shared_ptr<ParseTreeNode>& node, const std::string& prefix, bool isLast, std::vector<std::string>& lines);
};

#endif
