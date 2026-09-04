#include "../include/codegen.h"

void PythonCodeGenerator::line(const std::string& text) {
    out << std::string(indent * 4, ' ') << text << "\n";
}

std::string PythonCodeGenerator::expr(const Expr* e) {
    if (auto n = dynamic_cast<const NumberExpr*>(e)) return n->value;
    if (auto v = dynamic_cast<const VariableExpr*>(e)) return v->name;
    if (auto b = dynamic_cast<const BinaryExpr*>(e))
        return "(" + expr(b->left.get()) + " " + b->op + " " + expr(b->right.get()) + ")";
    return "";
}

void PythonCodeGenerator::stmt(const Stmt* s) {
    if (auto d = dynamic_cast<const DeclarationStmt*>(s))
        line(d->name + " = " + expr(d->initializer.get()));
    else if (auto a = dynamic_cast<const AssignmentStmt*>(s))
        line(a->name + " = " + expr(a->value.get()));
    else if (auto p = dynamic_cast<const PrintStmt*>(s))
        line("print(" + expr(p->value.get()) + ")");
    else if (auto i = dynamic_cast<const IfStmt*>(s)) {
        line("if " + expr(i->condition.get()) + ":");
        indent++;
        block(i->thenBlock.get());
        indent--;
        if (i->elseBlock) {
            line("else:");
            indent++;
            block(i->elseBlock.get());
            indent--;
        }
    } else if (auto w = dynamic_cast<const WhileStmt*>(s)) {
        line("while " + expr(w->condition.get()) + ":");
        indent++;
        block(w->body.get());
        indent--;
    }
}

void PythonCodeGenerator::block(const BlockStmt* b) {
    if (b->statements.empty()) line("pass");
    for (const auto& s : b->statements) stmt(s.get());
}

std::string PythonCodeGenerator::generate(const Program& program) {
    out.str("");
    out.clear();
    indent = 0;
    for (const auto& s : program.statements) stmt(s.get());
    return out.str();
}
