#include "../include/token.h"

std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::IntegerType: return "INTEGER_TYPE";
        case TokenType::FloatType: return "FLOAT_TYPE";
        case TokenType::If: return "IF";
        case TokenType::Else: return "ELSE";
        case TokenType::While: return "WHILE";
        case TokenType::Print: return "PRINT";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Number: return "NUMBER";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Assign: return "ASSIGN";
        case TokenType::Equal: return "EQUAL";
        case TokenType::NotEqual: return "NOT_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::Greater: return "GREATER";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::EndOfLine: return "END_OF_LINE";
        case TokenType::EndOfFile: return "EOF";
        default: return "INVALID";
    }
}
