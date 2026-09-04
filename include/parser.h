#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <string>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    Program parse();

    const std::vector<std::string>& errors() const { return errors_; }

private:
    const std::vector<Token>& tokens;
    size_t current = 0;
    std::vector<std::string> errors_;

    const Token& peek() const;
    const Token& previous() const;
    bool check(TokenType t) const;
    bool match(TokenType t);
    const Token& advance();
    bool isAtEnd() const;

    void error(const std::string& msg);
    void synchronize();

    StmtPtr statement();
    StmtPtr declaration();
    StmtPtr assignmentOrExpression();
    StmtPtr printStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    std::unique_ptr<BlockStmt> block();

    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();

    void consume(TokenType type, const std::string& message);
};
