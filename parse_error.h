#ifndef PARSEERROR_H
#define PARSEERROR_H

#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  ParseError — one syntax error entry
// ─────────────────────────────────────────────
struct ParseError {
    std::string message;
    int         line;
    int         col;

    ParseError(const std::string& msg, int ln, int cl)
        : message(msg), line(ln), col(cl) {}

    // For deduplication
    bool operator==(const ParseError& o) const {
        return line == o.line && col == o.col;
    }
};

using ErrorList = std::vector<ParseError>;

#endif