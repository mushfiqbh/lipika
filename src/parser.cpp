#include "../include/parser.h"
#include <sstream>

Parser::Parser(const std::vector<Token>& t) : tokens(t) {}

const Token& Parser::peek() const { return tokens[current]; }
const Token& Parser::previous() const { return tokens[current - 1]; }
bool Parser::check(TokenType t) const { return !isAtEnd() && peek().type == t; }
bool Parser::match(TokenType t) { if (check(t)) { advance(); return true; } return false; }
const Token& Parser::advance() { if (!isAtEnd()) ++current; return previous(); }
bool Parser::isAtEnd() const { return peek().type == TokenType::EndOfFile; }

void Parser::error(const std::string& msg) {
    std::ostringstream os;
    os << "Syntax Error at line " << peek().line << ": " << msg;
    errors_.push_back(os.str());
}

void Parser::synchronize() {
    size_t start = current;
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        if (peek().type == TokenType::If || peek().type == TokenType::While ||
            peek().type == TokenType::IntegerType || peek().type == TokenType::FloatType ||
            peek().type == TokenType::Print) {
            if (current == start) advance();
            return;
        }
        advance();
    }
    if (current == start && !isAtEnd()) advance();
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) { advance(); return; }
    error(message);
    throw std::runtime_error(message);
}

Program Parser::parse() {
    Program p;
    while (!isAtEnd()) {
        if (match(TokenType::EndOfLine)) continue;
        try {
            p.statements.push_back(statement());
        } catch (...) {
            synchronize();
        }
    }
    return p;
}

StmtPtr Parser::statement() {
    if (check(TokenType::IntegerType) || check(TokenType::FloatType)) return declaration();
    if (match(TokenType::Print)) return printStatement();
    if (match(TokenType::If)) return ifStatement();
    if (match(TokenType::While)) return whileStatement();
    return assignmentOrExpression();
}

StmtPtr Parser::declaration() {
    ValueType type;
    if (match(TokenType::IntegerType)) type = ValueType::Integer;
    else {
        consume(TokenType::FloatType, "Expected type keyword.");
        type = ValueType::Float;
    }
    Token name = peek();
    consume(TokenType::Identifier, "Expected variable name after type.");
    consume(TokenType::Assign, "Expected '=' after variable name.");
    auto init = expression();
    consume(TokenType::Semicolon, "Expected ';' after declaration.");
    return std::make_unique<DeclarationStmt>(type, name.lexeme, std::move(init));
}

StmtPtr Parser::assignmentOrExpression() {
    Token name = peek();
    consume(TokenType::Identifier, "Expected statement.");
    consume(TokenType::Assign, "Expected '=' after identifier.");
    auto value = expression();
    consume(TokenType::Semicolon, "Expected ';' after assignment.");
    return std::make_unique<AssignmentStmt>(name.lexeme, std::move(value));
}

StmtPtr Parser::printStatement() {
    consume(TokenType::LeftParen, "Expected '(' after দেখাও.");
    auto e = expression();
    consume(TokenType::RightParen, "Expected ')' after expression.");
    consume(TokenType::Semicolon, "Expected ';' after print statement.");
    return std::make_unique<PrintStmt>(std::move(e));
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LeftParen, "Expected '(' after যদি.");
    auto condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition.");
    auto thenBlock = block();
    std::unique_ptr<BlockStmt> elseBlock;
    if (match(TokenType::Else)) elseBlock = block();

    auto stmt = std::make_unique<IfStmt>();
    stmt->condition = std::move(condition);
    stmt->thenBlock = std::move(thenBlock);
    stmt->elseBlock = std::move(elseBlock);
    return stmt;
}

StmtPtr Parser::whileStatement() {
    consume(TokenType::LeftParen, "Expected '(' after যতক্ষণ.");
    auto condition = expression();
    consume(TokenType::RightParen, "Expected ')' after condition.");
    auto body = block();

    auto stmt = std::make_unique<WhileStmt>();
    stmt->condition = std::move(condition);
    stmt->body = std::move(body);
    return stmt;
}

std::unique_ptr<BlockStmt> Parser::block() {
    consume(TokenType::LeftBrace, "Expected '{'.");
    auto b = std::make_unique<BlockStmt>();
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        if (match(TokenType::EndOfLine)) continue;
        try { b->statements.push_back(statement()); }
        catch (...) { synchronize(); }
    }
    consume(TokenType::RightBrace, "Expected '}'.");
    return b;
}

ExprPtr Parser::expression() { return equality(); }

ExprPtr Parser::equality() {
    auto e = comparison();
    while (match(TokenType::Equal) || match(TokenType::NotEqual)) {
        std::string op = previous().lexeme;
        auto r = comparison();
        e = std::make_unique<BinaryExpr>(std::move(e), op, std::move(r));
    }
    return e;
}

ExprPtr Parser::comparison() {
    auto e = term();
    while (check(TokenType::Less) || check(TokenType::Greater) ||
           check(TokenType::LessEqual) || check(TokenType::GreaterEqual)) {
        advance();
        std::string op = previous().lexeme;
        auto r = term();
        e = std::make_unique<BinaryExpr>(std::move(e), op, std::move(r));
    }
    return e;
}

ExprPtr Parser::term() {
    auto e = factor();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        advance();
        std::string op = previous().lexeme;
        auto r = factor();
        e = std::make_unique<BinaryExpr>(std::move(e), op, std::move(r));
    }
    return e;
}

ExprPtr Parser::factor() {
    auto e = unary();
    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Percent)) {
        advance();
        std::string op = previous().lexeme;
        auto r = unary();
        e = std::make_unique<BinaryExpr>(std::move(e), op, std::move(r));
    }
    return e;
}

ExprPtr Parser::unary() {
    return primary();
}

ExprPtr Parser::primary() {
    if (match(TokenType::Number)) return std::make_unique<NumberExpr>(previous().lexeme);
    if (match(TokenType::Identifier)) return std::make_unique<VariableExpr>(previous().lexeme);
    if (match(TokenType::LeftParen)) {
        auto e = expression();
        consume(TokenType::RightParen, "Expected ')'.");
        return e;
    }
    error("Expected expression.");
    throw std::runtime_error("expression");
}
