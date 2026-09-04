#pragma once
#include "token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    char peek() const;
    char peekNext() const;
    char advance();
    void skipWhitespace();
    bool hasUtf8IdentifierStart() const;
    Token makeToken(TokenType type, const std::string& lexeme, int startLine, int startColumn);
    Token identifierOrKeyword();
    Token number();
};
