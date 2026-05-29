#ifndef PARSER_H
#define PARSER_H
#include "tiny_language_laxer.h"

#include "parse_node.h"
#include "parse_error.h"
#include <vector>
#include <string>

// ─────────────────────────────────────────────
//  Parser
//  Receives a flat token list from the Lexer
//  and builds a parse tree using recursive
//  descent, one grammar rule per method.
// ─────────────────────────────────────────────
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // Entry point — returns root node (PROGRAM)
    NodePtr parse();

    // Access errors after parse()
    const ErrorList& errors() const { return errors_; }
    bool hasErrors()          const { return !errors_.empty(); }

private:
    // ── token stream ──
    std::vector<Token> tokens_;
    size_t             pos_;

    // ── error list ──
    ErrorList errors_;

    // ─────────────────────────────────────────
    //  Step 1 — Helper methods
    // ─────────────────────────────────────────
    Token   current() const;          // peek, skip WS/comments/unknown
    Token   peek(int offset) const;   // look further ahead
    Token   advance();                // consume + skip

    bool    check(TokenType t)  const;
    bool    checkKeyword(const std::string& kw) const;
    bool    checkCompOp()       const;   // = or <
    bool    checkAddOp()        const;   // + or -
    bool    checkMulOp()        const;   // * or /

    bool    consume(TokenType t, const std::string& errMsg);
    bool    consumeKeyword(const std::string& kw, const std::string& errMsg);

    void    addError(const std::string& msg, int line, int col);
    void    synchronize();   // error recovery

    // helper: raw index of current meaningful token
    size_t  currentIndex() const;

    // ─────────────────────────────────────────
    //  Step 2 — Statement grammar rules
    // ─────────────────────────────────────────
    NodePtr parseProgram();
    NodePtr parseStmtSequence();
    NodePtr parseStatement();

    NodePtr parseIfStmt();
    NodePtr parseRepeatStmt();
    NodePtr parseAssignStmt();
    NodePtr parseReadStmt();
    NodePtr parseWriteStmt();

    // ─────────────────────────────────────────
    //  Step 3 — Expression grammar (precedence
    //  hierarchy: exp > simpleExp > term > factor)
    // ─────────────────────────────────────────
    NodePtr parseExp();
    NodePtr parseSimpleExp();
    NodePtr parseTerm();
    NodePtr parseFactor();

    // ─────────────────────────────────────────
    //  Helpers that skip whitespace/comments
    // ─────────────────────────────────────────
    static bool isSkippable(const Token& t);
    static bool isBlockTerminator(const Token& t);
    static bool isStmtStart(const Token& t);
};

#endif