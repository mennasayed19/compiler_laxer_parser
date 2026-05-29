#ifndef PARSENODE_H
#define PARSENODE_H

#include <string>
#include <vector>
#include <memory>

// ─────────────────────────────────────────────
//  Node types — internal (grammar rules) + leaf
// ─────────────────────────────────────────────
enum class NodeType {
    // Program structure
    PROGRAM,
    STMT_SEQUENCE,

    // Statements
    IF_STMT,
    REPEAT_STMT,
    ASSIGN_STMT,
    READ_STMT,
    WRITE_STMT,

    // Expressions
    EXP,
    SIMPLE_EXP,
    TERM,
    FACTOR,

    // Leaves
    IDENTIFIER,
    NUMBER,
    STRING_LIT,
    KEYWORD,
    OPERATOR
};

std::string nodeTypeName(NodeType t);

// ─────────────────────────────────────────────
//  ParseNode
// ─────────────────────────────────────────────
struct ParseNode {
    NodeType    type;
    std::string value;      // token lexeme for leaf nodes
    int         line = 0;
    int         col  = 0;

    std::vector<std::shared_ptr<ParseNode>> children;

    ParseNode(NodeType t, const std::string& v = "", int ln = 0, int cl = 0)
        : type(t), value(v), line(ln), col(cl) {}

    void addChild(std::shared_ptr<ParseNode> child) {
        if (child) children.push_back(child);
    }
};

using NodePtr = std::shared_ptr<ParseNode>;

#endif