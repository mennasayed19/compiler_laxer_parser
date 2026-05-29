#include "tiny_language_laxer.h"
#include <cctype>
#include <unordered_map>

// ===================== KEYWORDS =====================
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"if", IF_KW}, {"then", THEN_KW}, {"else", ELSE_KW},
    {"end", END_KW}, {"repeat", REPEAT_KW}, {"until", UNTIL_KW},
    {"read", READ_KW}, {"write", WRITE_KW},
    };

// ===================== CONSTRUCTOR =====================
Lexer::Lexer(const std::string& source)
    : src(source), pos(0), line(1), col(1) {}

// ===================== MAIN LOOP =====================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token t = nextToken();

        tokens.push_back(t);

        if (t.type == END_OF_FILE)
            break;
    }

    return tokens;
}

// ===================== CORE HELPERS =====================
char Lexer::peek() const {
    return (pos >= src.size()) ? '\0' : src[pos];
}

char Lexer::advance() {
    char c = src[pos++];

    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }

    return c;
}

void Lexer::skipWhitespace() {
    while (pos < src.size() &&
           std::isspace((unsigned char)peek())) {
        advance();
    }
}

// ===================== MAIN TOKENIZER =====================
Token Lexer::nextToken() {
    skipWhitespace();

    if (pos >= src.size())
        return {END_OF_FILE, "", line, col};

    char c = peek();
    int startLine = line;
    int startCol  = col;

    // IDENTIFIER / KEYWORD
    if (std::isalpha((unsigned char)c) || c == '_')
        return readIdentifierOrKeyword();

    // NUMBER
    if (std::isdigit((unsigned char)c))
        return readNumber();

    // SPECIAL CASES
    if (c == ':') return readAssignOrColon();
    if (c == '{') return readComment();
    if (c == '"') return readString();

    advance();
    std::string lex(1, c);

    switch (c) {
    case '+': return {ADDOP, lex, startLine, startCol};
    case '-': return {SUBOP, lex, startLine, startCol};
    case '*': return {MULOP, lex, startLine, startCol};
    case '/': return {DIVOP, lex, startLine, startCol};
    case '=':
    case '<': return {COMPARISONOP, lex, startLine, startCol};
    case ';': return {SEMICOLON, lex, startLine, startCol};
    case '(': return {LPAREN, lex, startLine, startCol};
    case ')': return {RPAREN, lex, startLine, startCol};
    case ',': return {COMMA, lex, startLine, startCol};
    default:
        return {
            UNKNOWN,
            lex,
            startLine,
            startCol,
            "Lexical Error: unknown character '" + lex + "'"
        };
    }
}

// ===================== IDENTIFIER / KEYWORD =====================
Token Lexer::readIdentifierOrKeyword() {
    int tok_line = line;
    int tok_col  = col;

    std::string lexeme;

    while (pos < src.size() &&
           (std::isalnum((unsigned char)peek()) || peek() == '_')) {
        lexeme += advance();
    }

    auto it = KEYWORDS.find(lexeme);
    TokenType type = (it != KEYWORDS.end()) ? it->second : ID;

    return {type, lexeme, tok_line, tok_col};
}

// ===================== NUMBER =====================
Token Lexer::readNumber() {
    int tok_line = line;
    int tok_col  = col;

    std::string lexeme;

    while (pos < src.size() &&
           std::isdigit((unsigned char)peek())) {
        lexeme += advance();
    }

    return {NUM, lexeme, tok_line, tok_col};
}

// ===================== ASSIGNMENT := =====================
Token Lexer::readAssignOrColon() {
    int tok_line = line;
    int tok_col  = col;

    advance(); // consume ':'

    if (peek() == '=') {
        advance();
        return {ASSIGNMENTOP, ":=", tok_line, tok_col, ""};
    }

    return {
        UNKNOWN, ":", tok_line, tok_col,
        "Lexical Error: expected ':=' but got ':' at line "
            + std::to_string(tok_line)
    };
}
// ===================== COMMENT { } =====================
Token Lexer::readComment() {
    int tok_line = line;
    int tok_col  = col;

    std::string lexeme;
    lexeme += advance(); // {

    while (pos < src.size() && peek() != '}') {
        lexeme += advance();
    }

    if (pos < src.size()) {
        lexeme += advance(); // }
        return {COMMENT, lexeme, tok_line, tok_col};
    }

    return {
        UNKNOWN,
        lexeme,
        tok_line,
        tok_col,
        "Lexical Error: unterminated comment"
    };
}

// ===================== STRING "..." =====================
Token Lexer::readString() {
    int tok_line = line;
    int tok_col  = col;

    std::string lexeme;
    lexeme += advance(); // "

    while (pos < src.size() &&
           peek() != '"' &&
           peek() != '\n') {
        lexeme += advance();
    }

    if (pos < src.size() && peek() == '"') {
        lexeme += advance();
        return {STRING_LIT, lexeme, tok_line, tok_col};
    }

    return {
        UNKNOWN,
        lexeme,
        tok_line,
        tok_col,
        "Lexical Error: unterminated string"
    };
}

// ===================== KEYWORD LOOKUP =====================
TokenType Lexer::lookupKeyword(const std::string& word) {
    auto it = KEYWORDS.find(word);
    return (it != KEYWORDS.end()) ? it->second : ID;
}

// ===================== TYPE NAME =====================
std::string Lexer::typeName(TokenType t) {
    switch (t) {
    case IF_KW: case THEN_KW: case ELSE_KW:
    case END_KW: case REPEAT_KW: case UNTIL_KW:
    case READ_KW: case WRITE_KW:
        return "KEYWORD";

    case ID:           return "IDENTIFIER";
    case NUM:          return "NUMBER";
    case STRING_LIT:   return "STRING";

    case ADDOP:        return "ADD";
    case SUBOP:        return "SUB";
    case MULOP:        return "MUL";
    case DIVOP:        return "DIV";

    case COMPARISONOP: return "RELOP";
    case ASSIGNMENTOP: return "ASSIGN";

    case SEMICOLON:    return "SEMICOLON";
    case LPAREN:       return "LPAREN";
    case RPAREN:       return "RPAREN";
    case COMMA:        return "COMMA";

    case COMMENT:      return "COMMENT";
    case UNKNOWN:      return "UNKNOWN";
    case END_OF_FILE:  return "EOF";
    }

    return "UNKNOWN";
}