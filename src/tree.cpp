#include "tree.h"

#include <sstream>

std::shared_ptr<ParseTreeNode> ParseTree::makeLeaf(const std::string& symbol) {
    auto node = std::make_shared<ParseTreeNode>();
    node->symbol = symbol;
    return node;
}

std::shared_ptr<ParseTreeNode> ParseTree::makeNode(const std::string& symbol, const std::vector<std::shared_ptr<ParseTreeNode>>& children) {
    auto node = std::make_shared<ParseTreeNode>();
    node->symbol = symbol;
    node->children = children;
    return node;
}

void ParseTree::renderChild(const std::shared_ptr<ParseTreeNode>& node, const std::string& prefix, bool isLast, std::vector<std::string>& lines) {
    if (!node) {
        return;
    }
    std::ostringstream line;
    line << prefix << (isLast ? "└── " : "├── ") << node->symbol;
    lines.push_back(line.str());
    std::string nextPrefix = prefix + (isLast ? "    " : "│   ");
    for (std::size_t i = 0; i < node->children.size(); ++i) {
        renderChild(node->children[i], nextPrefix, i + 1 == node->children.size(), lines);
    }
}

std::string ParseTree::render(const std::shared_ptr<ParseTreeNode>& root) {
    if (!root) {
        return "<empty tree>\n";
    }
    std::vector<std::string> lines;
    lines.push_back(root->symbol);
    for (std::size_t i = 0; i < root->children.size(); ++i) {
        renderChild(root->children[i], "", i + 1 == root->children.size(), lines);
    }
    std::ostringstream out;
    for (const std::string& line : lines) {
        out << line << '\n';
    }
    return out.str();
}
