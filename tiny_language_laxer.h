#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum TokenType {
    IF_KW, THEN_KW, ELSE_KW, END_KW,
    REPEAT_KW, UNTIL_KW, READ_KW, WRITE_KW,
    ID, NUM, STRING_LIT,
    ADDOP, SUBOP, MULOP, DIVOP,
    COMPARISONOP,
    ASSIGNMENTOP,
    SEMICOLON,
    LPAREN, RPAREN,
    COMMA,
    COMMENT,
    UNKNOWN,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;              // ✅ إضافة العمود
    std::string error;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
    static std::string typeName(TokenType t);

private:
    std::string src;
    size_t pos;
    int line;
    int col;              // ✅ إضافة العمود

    char peek() const;
    char advance();
    void skipWhitespace();

    Token nextToken();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readAssignOrColon();
    Token readComment();
    Token readString();

    static TokenType lookupKeyword(const std::string& word);
};

#endif