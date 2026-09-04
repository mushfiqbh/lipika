#pragma once
#include <string>

enum class TokenType {
    IntegerType, FloatType,
    If, Else, While, Print,
    Identifier, Number,
    Plus, Minus, Star, Slash, Percent,
    Assign,
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    LeftParen, RightParen, LeftBrace, RightBrace,
    Semicolon, EndOfLine, EndOfFile,
    Invalid
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

std::string tokenTypeName(TokenType type);
