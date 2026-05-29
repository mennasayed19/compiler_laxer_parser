#include "parse_node.h"

std::string nodeTypeName(NodeType t) {
    switch (t) {
    case NodeType::PROGRAM:       return "PROGRAM";
    case NodeType::STMT_SEQUENCE: return "STMT_SEQUENCE";
    case NodeType::IF_STMT:       return "IF_STMT";
    case NodeType::REPEAT_STMT:   return "REPEAT_STMT";
    case NodeType::ASSIGN_STMT:   return "ASSIGN_STMT";
    case NodeType::READ_STMT:     return "READ_STMT";
    case NodeType::WRITE_STMT:    return "WRITE_STMT";
    case NodeType::EXP:           return "EXP";
    case NodeType::SIMPLE_EXP:    return "SIMPLE_EXP";
    case NodeType::TERM:          return "TERM";
    case NodeType::FACTOR:        return "FACTOR";
    case NodeType::IDENTIFIER:    return "IDENTIFIER";
    case NodeType::NUMBER:        return "NUMBER";
    case NodeType::STRING_LIT:    return "STRING_LIT";
    case NodeType::KEYWORD:       return "KEYWORD";
    case NodeType::OPERATOR:      return "OPERATOR";
    default:                      return "UNKNOWN";
    }
}