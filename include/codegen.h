#pragma once
#include "ast.h"
#include <string>
#include <sstream>

class PythonCodeGenerator {
public:
    std::string generate(const Program& program);

private:
    std::ostringstream out;
    int indent = 0;

    void line(const std::string& text);
    std::string expr(const Expr* e);
    void stmt(const Stmt* s);
    void block(const BlockStmt* b);
};
