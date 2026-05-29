#include "tree_printer.h"

void TreePrinter::print(const NodePtr& node,
                        std::ostream& out,
                        const std::string& prefix,
                        bool isLast) {
    if (!node) return;

    out << prefix;
    out << (isLast ? "└── " : "├── ");

    // Node label
    out << nodeTypeName(node->type);
    if (!node->value.empty()) out << " [" << node->value << "]";
    if (node->line > 0)       out << "  (line " << node->line << ")";
    out << "\n";

    const std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < node->children.size(); ++i) {
        bool last = (i == node->children.size() - 1);
        print(node->children[i], out, childPrefix, last);
    }
}