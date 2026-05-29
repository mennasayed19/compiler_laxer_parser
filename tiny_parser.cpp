#include "tiny_parser.h"

// ═══════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════
Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), pos_(0) {}

// ═══════════════════════════════════════════════
//  Public entry point
// ═══════════════════════════════════════════════
NodePtr Parser::parse() {
    NodePtr root = parseProgram();
    // Check for leftover tokens after the program
    Token t = current();
    if (t.type != END_OF_FILE) {
        addError("Unexpected token '" + t.lexeme + "' after program end",
                 t.line, 0);
    }
    return root;
}

// ═══════════════════════════════════════════════
//  Static helpers
// ═══════════════════════════════════════════════
bool Parser::isSkippable(const Token& t) {
    return t.type == COMMENT || !t.error.empty();
    // whitespace is already stripped by the lexer
}

bool Parser::isBlockTerminator(const Token& t) {
    return t.type == END_KW   ||
           t.type == ELSE_KW  ||
           t.type == UNTIL_KW ||
           t.type == END_OF_FILE;
}

bool Parser::isStmtStart(const Token& t) {
    return t.type == IF_KW     ||
           t.type == REPEAT_KW ||
           t.type == READ_KW   ||
           t.type == WRITE_KW  ||
           t.type == ID;
}

// ═══════════════════════════════════════════════
//  Step 1 — Helper methods
// ═══════════════════════════════════════════════

// Find index of next meaningful (non-skippable) token
size_t Parser::currentIndex() const {
    size_t i = pos_;
    while (i < tokens_.size() && isSkippable(tokens_[i])) ++i;
    return i;
}

// Peek at current meaningful token (no consume)
Token Parser::current() const {
    size_t i = currentIndex();
    if (i >= tokens_.size()) return {END_OF_FILE, "", 0, };
    return tokens_[i];
}

// Look 'offset' meaningful tokens ahead (0 = current)
Token Parser::peek(int offset) const {
    size_t i = pos_;
    int count = 0;
    while (i < tokens_.size()) {
        if (!isSkippable(tokens_[i])) {
            if (count == offset) return tokens_[i];
            ++count;
        }
        ++i;
    }
    return {END_OF_FILE, "", 0, };
}

// Consume current meaningful token, advance pos_
Token Parser::advance() {
    size_t i = currentIndex();
    pos_ = i + 1;   // skip past the consumed token
    if (i >= tokens_.size()) return {END_OF_FILE, "", 0, };
    return tokens_[i];
}

bool Parser::check(TokenType t) const {
    return current().type == t;
}

bool Parser::checkKeyword(const std::string& kw) const {
    Token t = current();
    // keyword tokens carry the keyword name as their type name
    return t.lexeme == kw;
}

bool Parser::checkCompOp() const {
    return current().type == COMPARISONOP;
}

bool Parser::checkAddOp() const {
    TokenType t = current().type;
    return t == ADDOP || t == SUBOP;
}

bool Parser::checkMulOp() const {
    TokenType t = current().type;
    return t == MULOP || t == DIVOP;
}

// Consume expected type; record error if wrong
bool Parser::consume(TokenType t, const std::string& errMsg) {
    if (check(t)) { advance(); return true; }
    Token cur = current();
    addError(errMsg + " (got '" + cur.lexeme + "')", cur.line, 0);
    return false;
}

// Consume expected keyword; record error if wrong
bool Parser::consumeKeyword(const std::string& kw, const std::string& errMsg) {
    Token cur = current();
    if (cur.lexeme == kw) {
        advance();
        return true;
    }
    // تعديل الرسالة لتشمل نوع الخطأ "Syntax Error"
    addError("Syntax Error: " + errMsg + " (Found '" + cur.lexeme + "')", cur.line, 0);
    return false;
}

// Record error; deduplicate by line+col
void Parser::addError(const std::string& msg, int line, int col) {
    ParseError e(msg, line, col);
    for (const auto& ex : errors_)
        if (ex == e) return;
    errors_.push_back(e);
}

// Error recovery: skip until semicolon or statement-start keyword
void Parser::synchronize() {
    while (true) {
        Token t = current();
        if (t.type == END_OF_FILE)  break;
        if (t.type == SEMICOLON)    { advance(); break; }
        if (isStmtStart(t))         break;
        if (isBlockTerminator(t))   break;
        advance();
    }
}

// ═══════════════════════════════════════════════
//  Step 2 — Grammar rules
// ═══════════════════════════════════════════════

// program → stmt_sequence EOF
NodePtr Parser::parseProgram() {
    auto node = std::make_shared<ParseNode>(NodeType::PROGRAM, "", 0, 0);
    node->addChild(parseStmtSequence());
    return node;
}

// stmt_sequence → stmt { ';' stmt }
NodePtr Parser::parseStmtSequence() {
    auto node = std::make_shared<ParseNode>(NodeType::STMT_SEQUENCE);
    NodePtr stmt = parseStatement();
    if (stmt) node->addChild(stmt);

    while (check(SEMICOLON)) {
        advance();  // consume ';'
        // If we hit a block terminator, stop
        if (isBlockTerminator(current())) break;
        stmt = parseStatement();
        if (stmt) node->addChild(stmt);
    }
    return node;
}

// statement → if | repeat | read | write | assign
NodePtr Parser::parseStatement() {
    Token t = current();

    if (t.type == IF_KW)     return parseIfStmt();
    if (t.type == REPEAT_KW) return parseRepeatStmt();
    if (t.type == READ_KW)   return parseReadStmt();
    if (t.type == WRITE_KW)  return parseWriteStmt();
    if (t.type == ID)        return parseAssignStmt();

    // Block terminator — tell parent to stop
    if (isBlockTerminator(t)) return nullptr;

    // Unexpected token
    addError("Unexpected token '" + t.lexeme + "' in statement", t.line, 0);
    synchronize();
    return nullptr;
}

// if-stmt → 'if' exp 'then' stmt_seq ['else' stmt_seq] 'end'
NodePtr Parser::parseIfStmt() {
    Token kw = current();
    auto node = std::make_shared<ParseNode>(NodeType::IF_STMT, "if", kw.line);
    advance();  // consume 'if'

    // condition
    node->addChild(parseExp());

    // 'then'
    if (!consumeKeyword("then", "Expected 'then' after if-condition at line " +
                                    std::to_string(current().line))) {
        synchronize();
    } else {
        auto kwNode = std::make_shared<ParseNode>(NodeType::KEYWORD, "then");
        node->addChild(kwNode);
    }

    // then-branch
    node->addChild(parseStmtSequence());

    // optional 'else'
    if (check(ELSE_KW)) {
        auto elseNode = std::make_shared<ParseNode>(NodeType::KEYWORD, "else");
        node->addChild(elseNode);
        advance();  // consume 'else'
        node->addChild(parseStmtSequence());
    }

    // 'end'
    if (!consumeKeyword("end", "Expected 'end' to close if-statement at line " +
                                   std::to_string(current().line))) {
        synchronize();
    } else {
        auto endNode = std::make_shared<ParseNode>(NodeType::KEYWORD, "end");
        node->addChild(endNode);
    }
    return node;
}

// repeat-stmt → 'repeat' stmt_seq 'until' exp
NodePtr Parser::parseRepeatStmt() {
    Token kw = current();
    auto node = std::make_shared<ParseNode>(NodeType::REPEAT_STMT, "repeat", kw.line);
    advance();  // consume 'repeat'

    // body
    node->addChild(parseStmtSequence());

    // 'until'
    if (!consumeKeyword("until", "Expected 'until' to close repeat at line " +
                                     std::to_string(current().line))) {
        synchronize();
        return node;
    }
    auto untilNode = std::make_shared<ParseNode>(NodeType::KEYWORD, "until");
    node->addChild(untilNode);

    // Special case: semicolon right after 'until' means missing condition
    if (check(SEMICOLON) || check(END_OF_FILE)) {
        addError("Expected condition after 'until' at line " +
                     std::to_string(current().line), current().line, 0);
        return node;
    }

    // condition
    node->addChild(parseExp());
    return node;
}

// assign-stmt → identifier ':=' exp
NodePtr Parser::parseAssignStmt() {
    Token id = current();
    auto node = std::make_shared<ParseNode>(NodeType::ASSIGN_STMT, id.lexeme, id.line);
    advance();  // consume identifier

    auto idNode = std::make_shared<ParseNode>(NodeType::IDENTIFIER, id.lexeme, id.line);
    node->addChild(idNode);

    // ':='
    if (!consume(ASSIGNMENTOP, "Expected ':=' after '" + id.lexeme + "' at line " +
                                   std::to_string(id.line))) {
        synchronize();
        return node;
    }
    auto opNode = std::make_shared<ParseNode>(NodeType::OPERATOR, ":=");
    node->addChild(opNode);

    // Extra check: obviously missing expression
    Token next = current();
    if (next.type == SEMICOLON || next.type == END_OF_FILE ||
        next.type == UNTIL_KW  || next.type == END_KW) {
        addError("Expected expression after ':=' at line " +
                     std::to_string(id.line) + ", got '" + next.lexeme + "'",
                 id.line, 0);
        return node;
    }

    node->addChild(parseExp());
    return node;
}

// read-stmt → 'read' identifier { ',' identifier }
NodePtr Parser::parseReadStmt() {
    Token kw = current();
    auto node = std::make_shared<ParseNode>(NodeType::READ_STMT, "read", kw.line);
    advance();  // consume 'read'

    if (!check(ID)) {
        addError("Expected identifier after 'read' at line " +
                     std::to_string(kw.line), kw.line, 0);
        synchronize();
        return node;
    }

    // first identifier
    Token id = advance();
    node->addChild(std::make_shared<ParseNode>(NodeType::IDENTIFIER, id.lexeme, id.line));

    // more identifiers separated by commas
    while (check(COMMA)) {
        advance();  // consume ','
        if (!check(ID)) {
            addError("Expected identifier after ',' in read at line " +
                         std::to_string(current().line), current().line, 0);
            break;
        }
        Token nxt = advance();
        node->addChild(std::make_shared<ParseNode>(NodeType::IDENTIFIER, nxt.lexeme, nxt.line));
    }
    return node;
}

// write-stmt → 'write' exp { ',' exp }
NodePtr Parser::parseWriteStmt() {
    Token kw = current();
    auto node = std::make_shared<ParseNode>(NodeType::WRITE_STMT, "write", kw.line);
    advance();  // consume 'write'

    if (check(SEMICOLON) || check(END_OF_FILE) || isBlockTerminator(current())) {
        addError("Expected expression after 'write' at line " +
                     std::to_string(kw.line), kw.line, 0);
        return node;
    }

    node->addChild(parseExp());

    while (check(COMMA)) {
        advance();  // consume ','
        node->addChild(parseExp());
    }
    return node;
}

// ═══════════════════════════════════════════════
//  Step 3 — Expression hierarchy
// ═══════════════════════════════════════════════

// exp → simple_exp [ comp_op simple_exp ]
NodePtr Parser::parseExp() {
    auto node = std::make_shared<ParseNode>(NodeType::EXP);
    node->addChild(parseSimpleExp());

    if (checkCompOp()) {
        Token op = advance();
        node->addChild(std::make_shared<ParseNode>(NodeType::OPERATOR, op.lexeme, op.line));
        node->addChild(parseSimpleExp());
    }
    return node;
}

// simple_exp → term { addop term }
NodePtr Parser::parseSimpleExp() {
    auto node = std::make_shared<ParseNode>(NodeType::SIMPLE_EXP);
    node->addChild(parseTerm());

    while (checkAddOp()) {
        Token op = advance();
        node->addChild(std::make_shared<ParseNode>(NodeType::OPERATOR, op.lexeme, op.line));
        node->addChild(parseTerm());
    }
    return node;
}

// term → factor { mulop factor }
NodePtr Parser::parseTerm() {
    auto node = std::make_shared<ParseNode>(NodeType::TERM);
    node->addChild(parseFactor());

    while (checkMulOp()) {
        Token op = advance();
        node->addChild(std::make_shared<ParseNode>(NodeType::OPERATOR, op.lexeme, op.line));
        node->addChild(parseFactor());
    }
    return node;
}

// factor → number | identifier | '(' exp ')'
NodePtr Parser::parseFactor() {
    Token t = current();

    if (t.type == NUM) {
        advance();
        return std::make_shared<ParseNode>(NodeType::NUMBER, t.lexeme, t.line);
    }
    if (t.type == ID) {
        advance();
        return std::make_shared<ParseNode>(NodeType::IDENTIFIER, t.lexeme, t.line);
    }
    if (t.type == STRING_LIT) {
        advance();
        return std::make_shared<ParseNode>(NodeType::STRING_LIT, t.lexeme, t.line);
    }
    if (t.type == LPAREN) {
        advance();  // consume '('
        NodePtr inner = parseExp();
        consume(RPAREN, "Expected ')' after expression at line " +
                            std::to_string(t.line));
        auto node = std::make_shared<ParseNode>(NodeType::FACTOR, "()");
        node->addChild(inner);
        return node;
    }

    // Nothing matched — report only if not a natural stopper
    if (!isBlockTerminator(t) && t.type != SEMICOLON && t.type != END_OF_FILE) {
        addError("Unexpected token '" + t.lexeme + "' in expression at line " +
                     std::to_string(t.line), t.line, 0);
        advance();
    }
    return nullptr;
}