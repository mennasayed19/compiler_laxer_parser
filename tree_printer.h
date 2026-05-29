#ifndef TREEPRINTER_H
#define TREEPRINTER_H

#include "parse_node.h"
#include <string>
#include <iostream>

// ─────────────────────────────────────────────
//  TreePrinter
//  Pretty-prints the parse tree to any ostream
// ─────────────────────────────────────────────
class TreePrinter {
public:
    static void print(const NodePtr& node,
                      std::ostream& out = std::cout,
                      const std::string& prefix = "",
                      bool isLast = true);
};

#endif