/**
* Lipika Compiler - Bangla Programming Language to Python
* 
* Run the commands using c++
* g++ -std=c++17 src/*.cpp -o lipika
* lipika examples/demo.lipi
* python output.py
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <cctype>
#include <stdexcept>

// =============================================================================
// AST & TOKEN DEFINITIONS
// =============================================================================

enum class TokenType {
    IntegerType, FloatType, If, Else, While, Print,
    Identifier, Number, Plus, Minus, Star, Slash, Percent, Assign,
    Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
    LeftParen, RightParen, LeftBrace, RightBrace, Semicolon, EndOfLine, EndOfFile, Invalid
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line, column;
};

enum class ValueType { Integer, Float, Unknown };

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr { std::string value; };
struct VariableExpr { std::string name; };
struct BinaryExpr { ExprPtr left; std::string op; ExprPtr right; };

struct Expr {
    std::variant<NumberExpr, VariableExpr, BinaryExpr> node;
};

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct BlockStmt { std::vector<StmtPtr> statements; };
struct DeclarationStmt { ValueType type; std::string name; ExprPtr initializer; };
struct AssignmentStmt { std::string name; ExprPtr value; };
struct PrintStmt { ExprPtr value; };
struct IfStmt { ExprPtr condition; BlockStmt thenBlock; std::unique_ptr<BlockStmt> elseBlock; };
struct WhileStmt { ExprPtr condition; BlockStmt body; };

struct Stmt {
    std::variant<DeclarationStmt, AssignmentStmt, PrintStmt, IfStmt, WhileStmt> node;
};

struct Program { std::vector<StmtPtr> statements; };

// Helper template for variant pattern matching
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// =============================================================================
// LEXER (Tokenizer)
// =============================================================================

class Lexer {
    std::string source;
    size_t pos = 0;
    int line = 1, column = 1;

    char peek() const { return pos < source.size() ? source[pos] : '\0'; }
    char advance() {
        char c = peek();
        if (c == '\0') return c;
        pos++;
        if (c == '\n') { line++; column = 1; } else column++;
        return c;
    }

    int utf8Len(size_t offset) const {
        unsigned char c = source[offset];
        if (c >= 0xC2 && c <= 0xDF) return offset + 1 < source.size() ? 2 : 0;
        if (c >= 0xE0 && c <= 0xEF) return offset + 2 < source.size() ? 3 : 0;
        if (c >= 0xF0 && c <= 0xF4) return offset + 3 < source.size() ? 3 : 0;
        return 0;
    }

public:
    explicit Lexer(std::string s) : source(std::move(s)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> out;
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"সংখ্যা", TokenType::IntegerType},
            {"দশমিক", TokenType::FloatType},
            {"যদি", TokenType::If},
            {"নাহলে", TokenType::Else},
            {"যতক্ষণ", TokenType::While},
            {"দেখাও", TokenType::Print}
        };

        while (true) {
            while (peek() == ' ' || peek() == '\t' || peek() == '\r' || (peek() == '/' && pos + 1 < source.size() && source[pos + 1] == '/')) {
                if (peek() == '/') while (peek() != '\0' && peek() != '\n') advance();
                else advance();
            }

            int sl = line, sc = column;
            char c = peek();
            if (c == '\0') { out.push_back({TokenType::EndOfFile, "", sl, sc}); break; }
            if (c == '\n') { advance(); out.push_back({TokenType::EndOfLine, "\\n", sl, sc}); continue; }

            int ulen = utf8Len(pos);
            if (std::isalpha((unsigned char)c) || c == '_' || ulen > 0) {
                std::string text;
                while (true) {
                    if (std::isalnum((unsigned char)peek()) || peek() == '_') text += advance();
                    else if (int l = utf8Len(pos)) { for (int i = 0; i < l; ++i) text += advance(); }
                    else break;
                }
                auto it = keywords.find(text);
                out.push_back({it != keywords.end() ? it->second : TokenType::Identifier, text, sl, sc});
                continue;
            }

            if (std::isdigit((unsigned char)c)) {
                std::string text;
                bool dot = false;
                while (std::isdigit((unsigned char)peek()) || (!dot && peek() == '.' && (dot = true)))
                    text += advance();
                out.push_back({TokenType::Number, text, sl, sc});
                continue;
            }

            advance();
            auto matchOp = [&](char next, TokenType single, TokenType doubleType, std::string sStr, std::string dStr) {
                if (peek() == next) { advance(); out.push_back({doubleType, dStr, sl, sc}); }
                else out.push_back({single, sStr, sl, sc});
            };

            switch (c) {
                case '+': out.push_back({TokenType::Plus, "+", sl, sc}); break;
                case '-': out.push_back({TokenType::Minus, "-", sl, sc}); break;
                case '*': out.push_back({TokenType::Star, "*", sl, sc}); break;
                case '/': out.push_back({TokenType::Slash, "/", sl, sc}); break;
                case '%': out.push_back({TokenType::Percent, "%", sl, sc}); break;
                case '(': out.push_back({TokenType::LeftParen, "(", sl, sc}); break;
                case ')': out.push_back({TokenType::RightParen, ")", sl, sc}); break;
                case '{': out.push_back({TokenType::LeftBrace, "{", sl, sc}); break;
                case '}': out.push_back({TokenType::RightBrace, "}", sl, sc}); break;
                case ';': out.push_back({TokenType::Semicolon, ";", sl, sc}); break;
                case '=': matchOp('=', TokenType::Assign, TokenType::Equal, "=", "=="); break;
                case '!': matchOp('=', TokenType::Invalid, TokenType::NotEqual, "!", "!="); break;
                case '<': matchOp('=', TokenType::Less, TokenType::LessEqual, "<", "<="); break;
                case '>': matchOp('=', TokenType::Greater, TokenType::GreaterEqual, ">", ">="); break;
                default:  out.push_back({TokenType::Invalid, std::string(1, c), sl, sc}); break;
            }
        }
        return out;
    }
};

// =============================================================================
// PARSER
// =============================================================================

class Parser {
    const std::vector<Token>& tokens;
    size_t current = 0;
    std::vector<std::string> errors_;

    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current == 0 ? 0 : current - 1]; }
    bool check(TokenType t) const { return !isAtEnd() && peek().type == t; }
    bool isAtEnd() const { return peek().type == TokenType::EndOfFile; }
    const Token& advance() { if (!isAtEnd()) current++; return previous(); }
    bool match(TokenType t) { if (check(t)) { advance(); return true; } return false; }

    void error(const std::string& msg) {
        errors_.push_back("Syntax Error at line " + std::to_string(peek().line) + ": " + msg);
    }

    void consume(TokenType type, const std::string& msg) {
        if (check(type)) advance();
        else { error(msg); throw std::runtime_error(msg); }
    }

    void synchronize() {
        while (!isAtEnd()) {
            if (previous().type == TokenType::Semicolon) return;
            if (peek().type == TokenType::EndOfLine || peek().type == TokenType::RightBrace) {
                if (peek().type == TokenType::EndOfLine) advance();
                return;
            }
            advance();
        }
    }

    ExprPtr primary() {
        if (match(TokenType::Number)) return std::make_unique<Expr>(NumberExpr{previous().lexeme});
        if (match(TokenType::Identifier)) return std::make_unique<Expr>(VariableExpr{previous().lexeme});
        if (match(TokenType::LeftParen)) {
            auto e = expression();
            consume(TokenType::RightParen, "Expected ')'.");
            return e;
        }
        error("Expected expression.");
        throw std::runtime_error("expression");
    }

    ExprPtr binary(auto nextLevel, std::initializer_list<TokenType> types) {
        auto e = (this->*nextLevel)();
        while (true) {
            bool found = false;
            for (auto t : types) {
                if (check(t)) {
                    advance();
                    std::string op = previous().lexeme;
                    auto r = (this->*nextLevel)();
                    e = std::make_unique<Expr>(BinaryExpr{std::move(e), op, std::move(r)});
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        return e;
    }

    ExprPtr factor() { return binary(&Parser::primary, {TokenType::Star, TokenType::Slash, TokenType::Percent}); }
    ExprPtr term() { return binary(&Parser::factor, {TokenType::Plus, TokenType::Minus}); }
    ExprPtr comparison() { return binary(&Parser::term, {TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual}); }
    ExprPtr equality() { return binary(&Parser::comparison, {TokenType::Equal, TokenType::NotEqual}); }
    ExprPtr expression() { return equality(); }

    BlockStmt block() {
        consume(TokenType::LeftBrace, "Expected '{'.");
        BlockStmt b;
        while (!check(TokenType::RightBrace) && !isAtEnd()) {
            if (match(TokenType::EndOfLine)) continue;
            try { b.statements.push_back(statement()); } catch (...) { synchronize(); }
        }
        consume(TokenType::RightBrace, "Expected '}'.");
        return b;
    }

    StmtPtr statement() {
        if (check(TokenType::IntegerType) || check(TokenType::FloatType)) {
            ValueType type = match(TokenType::IntegerType) ? ValueType::Integer : ValueType::Float;
            Token name = peek();
            consume(TokenType::Identifier, "Expected variable name.");
            consume(TokenType::Assign, "Expected '='.");
            auto init = expression();
            consume(TokenType::Semicolon, "Expected ';'.");
            return std::make_unique<Stmt>(DeclarationStmt{type, name.lexeme, std::move(init)});
        }
        if (match(TokenType::Print)) {
            consume(TokenType::LeftParen, "Expected '('.");
            auto e = expression();
            consume(TokenType::RightParen, "Expected ')'.");
            consume(TokenType::Semicolon, "Expected ';'.");
            return std::make_unique<Stmt>(PrintStmt{std::move(e)});
        }
        if (match(TokenType::If)) {
            consume(TokenType::LeftParen, "Expected '('.");
            auto cond = expression();
            consume(TokenType::RightParen, "Expected ')'.");
            auto thenB = block();
            std::unique_ptr<BlockStmt> elseB;
            if (match(TokenType::Else)) elseB = std::make_unique<BlockStmt>(block());
            return std::make_unique<Stmt>(IfStmt{std::move(cond), std::move(thenB), std::move(elseB)});
        }
        if (match(TokenType::While)) {
            consume(TokenType::LeftParen, "Expected '('.");
            auto cond = expression();
            consume(TokenType::RightParen, "Expected ')'.");
            return std::make_unique<Stmt>(WhileStmt{std::move(cond), block()});
        }

        Token name = peek();
        consume(TokenType::Identifier, "Expected statement.");
        consume(TokenType::Assign, "Expected '='.");
        auto val = expression();
        consume(TokenType::Semicolon, "Expected ';'.");
        return std::make_unique<Stmt>(AssignmentStmt{name.lexeme, std::move(val)});
    }

public:
    explicit Parser(const std::vector<Token>& t) : tokens(t) {}

    Program parse() {
        Program p;
        while (!isAtEnd()) {
            if (match(TokenType::EndOfLine)) continue;
            try { p.statements.push_back(statement()); } catch (...) { synchronize(); }
        }
        return p;
    }

    const std::vector<std::string>& errors() const { return errors_; }
};

// =============================================================================
// SEMANTIC ANALYZER
// =============================================================================

class SemanticAnalyzer {
    std::unordered_map<std::string, ValueType> symbols;
    std::vector<std::string> errors_;

    ValueType expressionType(const Expr* expr) {
        if (!expr) return ValueType::Unknown;
        return std::visit(overloaded{
            [](const NumberExpr& n) { return n.value.find('.') != std::string::npos ? ValueType::Float : ValueType::Integer; },
            [this](const VariableExpr& v) {
                if (!symbols.count(v.name)) {
                    errors_.push_back("Semantic Error: variable '" + v.name + "' is not declared.");
                    return ValueType::Unknown;
                }
                return symbols[v.name];
            },
            [this](const BinaryExpr& b) {
                auto l = expressionType(b.left.get());
                auto r = expressionType(b.right.get());
                if (l == ValueType::Unknown || r == ValueType::Unknown) return ValueType::Unknown;
                if (b.op == "==" || b.op == "!=" || b.op == "<" || b.op == ">" || b.op == "<=" || b.op == ">=")
                    return ValueType::Integer;
                return (l == ValueType::Float || r == ValueType::Float) ? ValueType::Float : ValueType::Integer;
            }
        }, expr->node);
    }

    void statement(const Stmt* stmt) {
        if (!stmt) return;
        std::visit(overloaded{
            [this](const DeclarationStmt& d) {
                if (symbols.count(d.name)) errors_.push_back("Semantic Error: variable '" + d.name + "' already declared.");
                auto actual = expressionType(d.initializer.get());
                if (actual != ValueType::Unknown && actual != d.type)
                    errors_.push_back("Type Error: initializer type mismatch for '" + d.name + "'.");
                symbols[d.name] = d.type;
            },
            [this](const AssignmentStmt& a) {
                if (!symbols.count(a.name)) errors_.push_back("Semantic Error: variable '" + a.name + "' is not declared.");
                else {
                    auto actual = expressionType(a.value.get());
                    if (actual != ValueType::Unknown && actual != symbols[a.name])
                        errors_.push_back("Type Error: assignment type mismatch for '" + a.name + "'.");
                }
            },
            [this](const PrintStmt& p) { expressionType(p.value.get()); },
            [this](const IfStmt& i) {
                expressionType(i.condition.get());
                for (const auto& s : i.thenBlock.statements) statement(s.get());
                if (i.elseBlock) for (const auto& s : i.elseBlock->statements) statement(s.get());
            },
            [this](const WhileStmt& w) {
                expressionType(w.condition.get());
                for (const auto& s : w.body.statements) statement(s.get());
            }
        }, stmt->node);
    }

public:
    bool analyze(const Program& program) {
        symbols.clear(); errors_.clear();
        for (const auto& s : program.statements) statement(s.get());
        return errors_.empty();
    }
    const std::vector<std::string>& errors() const { return errors_; }
};

// =============================================================================
// CODE GENERATOR (Python Target)
// =============================================================================

class PythonCodeGenerator {
    std::ostringstream out;
    int indent = 0;

    void line(const std::string& text) { out << std::string(indent * 4, ' ') << text << "\n"; }

    std::string expr(const Expr* e) {
        if (!e) return "0";
        return std::visit(overloaded{
            [](const NumberExpr& n) { return n.value; },
            [](const VariableExpr& v) { return v.name; },
            [this](const BinaryExpr& b) { return "(" + expr(b.left.get()) + " " + b.op + " " + expr(b.right.get()) + ")"; }
        }, e->node);
    }

    void block(const BlockStmt& b) {
        if (b.statements.empty()) { line("pass"); return; }
        for (const auto& s : b.statements) stmt(s.get());
    }

    void stmt(const Stmt* s) {
        if (!s) return;
        std::visit(overloaded{
            [this](const DeclarationStmt& d) { line(d.name + " = " + expr(d.initializer.get())); },
            [this](const AssignmentStmt& a) { line(a.name + " = " + expr(a.value.get())); },
            [this](const PrintStmt& p) { line("print(" + expr(p.value.get()) + ")"); },
            [this](const IfStmt& i) {
                line("if " + expr(i.condition.get()) + ":");
                indent++; block(i.thenBlock); indent--;
                if (i.elseBlock) {
                    line("else:");
                    indent++; block(*i.elseBlock); indent--;
                }
            },
            [this](const WhileStmt& w) {
                line("while " + expr(w.condition.get()) + ":");
                indent++; block(w.body); indent--;
            }
        }, s->node);
    }

public:
    std::string generate(const Program& program) {
        out.str(""); out.clear(); indent = 0;
        for (const auto& s : program.statements) stmt(s.get());
        return out.str();
    }
};

// =============================================================================
// MAIN COMPILER DRIVER
// =============================================================================

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./lipika <source.lipi>\n";
        return 1;
    }

    try {
        std::ifstream in(argv[1]);
        if (!in) throw std::runtime_error("Cannot open source file.");
        std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& t : tokens) {
            if (t.type == TokenType::Invalid) {
                std::cerr << "Lexical Error at line " << t.line << ": invalid token '" << t.lexeme << "'\n";
                return 1;
            }
        }

        Parser parser(tokens);
        Program program = parser.parse();
        if (!parser.errors().empty()) {
            for (const auto& e : parser.errors()) std::cerr << e << "\n";
            return 1;
        }

        SemanticAnalyzer semantic;
        if (!semantic.analyze(program)) {
            for (const auto& e : semantic.errors()) std::cerr << e << "\n";
            return 1;
        }

        PythonCodeGenerator generator;
        std::ofstream("output.py") << generator.generate(program);

        std::cout << "Compilation successful.\nGenerated target: output.py\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Compiler Error: " << e.what() << "\n";
        return 1;
    }
}