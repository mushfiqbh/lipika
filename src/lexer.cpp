#include "../include/lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace {
int utf8SequenceLength(const std::string& source, size_t offset) {
    unsigned char lead = static_cast<unsigned char>(source[offset]);
    int length = 0;
    if (lead >= 0xC2 && lead <= 0xDF) length = 2;
    else if (lead >= 0xE0 && lead <= 0xEF) length = 3;
    else if (lead >= 0xF0 && lead <= 0xF4) length = 4;
    else return 0;

    if (offset + length > source.size()) return 0;
    for (int index = 1; index < length; ++index) {
        unsigned char byte = static_cast<unsigned char>(source[offset + index]);
        if (byte < 0x80 || byte > 0xBF) return 0;
    }
    return length;
}
}

Lexer::Lexer(const std::string& s) : source(s) {}

char Lexer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::peekNext() const {
    return pos + 1 < source.size() ? source[pos + 1] : '\0';
}

char Lexer::advance() {
    char c = peek();
    if (c == '\0') return c;
    ++pos;
    if (c == '\n') { ++line; column = 1; }
    else ++column;
    return c;
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') advance();
        else if (c == '/' && peekNext() == '/') {
            while (peek() != '\0' && peek() != '\n') advance();
        } else break;
    }
}

bool Lexer::hasUtf8IdentifierStart() const {
    return utf8SequenceLength(source, pos) != 0;
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme,
                       int startLine, int startColumn) {
    return {type, lexeme, startLine, startColumn};
}

Token Lexer::identifierOrKeyword() {
    int sl = line, sc = column;
    std::string text;
    while (true) {
        unsigned char c = static_cast<unsigned char>(peek());
        if (std::isalnum(c) || c == '_') {
            text += advance();
            continue;
        }
        int utf8Length = utf8SequenceLength(source, pos);
        if (utf8Length == 0) break;
        for (int index = 0; index < utf8Length; ++index) text += advance();
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"সংখ্যা", TokenType::IntegerType},
        {"দশমিক", TokenType::FloatType},
        {"যদি", TokenType::If},
        {"নাহলে", TokenType::Else},
        {"যতক্ষণ", TokenType::While},
        {"দেখাও", TokenType::Print}
    };

    auto it = keywords.find(text);
    if (it != keywords.end()) return makeToken(it->second, text, sl, sc);
    return makeToken(TokenType::Identifier, text, sl, sc);
}

Token Lexer::number() {
    int sl = line, sc = column;
    std::string text;
    bool dot = false;
    while (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.') {
        if (peek() == '.') {
            if (dot) break;
            dot = true;
        }
        text += advance();
    }
    return makeToken(TokenType::Number, text, sl, sc);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> out;
    while (true) {
        skipWhitespace();
        int sl = line, sc = column;
        char c = peek();
        if (c == '\0') {
            out.push_back(makeToken(TokenType::EndOfFile, "", sl, sc));
            break;
        }
        if (c == '\n') {
            advance();
            out.push_back(makeToken(TokenType::EndOfLine, "\\n", sl, sc));
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || hasUtf8IdentifierStart()) {
            out.push_back(identifierOrKeyword());
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c))) {
            out.push_back(number());
            continue;
        }

        switch (c) {
            case '+': advance(); out.push_back(makeToken(TokenType::Plus, "+", sl, sc)); break;
            case '-': advance(); out.push_back(makeToken(TokenType::Minus, "-", sl, sc)); break;
            case '*': advance(); out.push_back(makeToken(TokenType::Star, "*", sl, sc)); break;
            case '/': advance(); out.push_back(makeToken(TokenType::Slash, "/", sl, sc)); break;
            case '%': advance(); out.push_back(makeToken(TokenType::Percent, "%", sl, sc)); break;
            case '(': advance(); out.push_back(makeToken(TokenType::LeftParen, "(", sl, sc)); break;
            case ')': advance(); out.push_back(makeToken(TokenType::RightParen, ")", sl, sc)); break;
            case '{': advance(); out.push_back(makeToken(TokenType::LeftBrace, "{", sl, sc)); break;
            case '}': advance(); out.push_back(makeToken(TokenType::RightBrace, "}", sl, sc)); break;
            case ';': advance(); out.push_back(makeToken(TokenType::Semicolon, ";", sl, sc)); break;
            case '=':
                advance();
                if (peek() == '=') { advance(); out.push_back(makeToken(TokenType::Equal, "==", sl, sc)); }
                else out.push_back(makeToken(TokenType::Assign, "=", sl, sc));
                break;
            case '!':
                advance();
                if (peek() == '=') { advance(); out.push_back(makeToken(TokenType::NotEqual, "!=", sl, sc)); }
                else out.push_back(makeToken(TokenType::Invalid, "!", sl, sc));
                break;
            case '<':
                advance();
                if (peek() == '=') { advance(); out.push_back(makeToken(TokenType::LessEqual, "<=", sl, sc)); }
                else out.push_back(makeToken(TokenType::Less, "<", sl, sc));
                break;
            case '>':
                advance();
                if (peek() == '=') { advance(); out.push_back(makeToken(TokenType::GreaterEqual, ">=", sl, sc)); }
                else out.push_back(makeToken(TokenType::Greater, ">", sl, sc));
                break;
            default:
                advance();
                out.push_back(makeToken(TokenType::Invalid, std::string(1, c), sl, sc));
        }
    }
    return out;
}
