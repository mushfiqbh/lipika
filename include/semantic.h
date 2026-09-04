#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>

class SemanticAnalyzer {
public:
    bool analyze(const Program& program);
    const std::vector<std::string>& errors() const { return errors_; }

private:
    std::unordered_map<std::string, ValueType> symbols;
    std::vector<std::string> errors_;

    ValueType expressionType(const Expr* expr);
    void statement(const Stmt* stmt);
};
