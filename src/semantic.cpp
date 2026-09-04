#include "../include/semantic.h"

static bool numeric(ValueType t) {
    return t == ValueType::Integer || t == ValueType::Float;
}

bool SemanticAnalyzer::analyze(const Program& program) {
    symbols.clear();
    errors_.clear();
    for (const auto& s : program.statements) statement(s.get());
    return errors_.empty();
}

ValueType SemanticAnalyzer::expressionType(const Expr* expr) {
    if (auto n = dynamic_cast<const NumberExpr*>(expr))
        return n->value.find('.') != std::string::npos ? ValueType::Float : ValueType::Integer;

    if (auto v = dynamic_cast<const VariableExpr*>(expr)) {
        auto it = symbols.find(v->name);
        if (it == symbols.end()) {
            errors_.push_back("Semantic Error: variable '" + v->name + "' is not declared.");
            return ValueType::Unknown;
        }
        return it->second;
    }

    if (auto b = dynamic_cast<const BinaryExpr*>(expr)) {
        auto l = expressionType(b->left.get());
        auto r = expressionType(b->right.get());
        if (!numeric(l) || !numeric(r)) return ValueType::Unknown;

        if (b->op == "==" || b->op == "!=" || b->op == "<" || b->op == ">" ||
            b->op == "<=" || b->op == ">=")
            return ValueType::Integer; // target Python boolean is acceptable as a condition

        return (l == ValueType::Float || r == ValueType::Float) ? ValueType::Float : ValueType::Integer;
    }
    return ValueType::Unknown;
}

void SemanticAnalyzer::statement(const Stmt* stmt) {
    if (auto d = dynamic_cast<const DeclarationStmt*>(stmt)) {
        if (symbols.count(d->name))
            errors_.push_back("Semantic Error: variable '" + d->name + "' already declared.");
        auto actual = expressionType(d->initializer.get());
        if (actual != ValueType::Unknown && actual != d->type)
            errors_.push_back("Type Error: initializer type does not match variable '" + d->name + "'.");
        symbols[d->name] = d->type;
    } else if (auto a = dynamic_cast<const AssignmentStmt*>(stmt)) {
        auto it = symbols.find(a->name);
        if (it == symbols.end()) {
            errors_.push_back("Semantic Error: variable '" + a->name + "' is not declared.");
        } else {
            auto actual = expressionType(a->value.get());
            if (actual != ValueType::Unknown && actual != it->second)
                errors_.push_back("Type Error: assignment type does not match variable '" + a->name + "'.");
        }
    } else if (auto p = dynamic_cast<const PrintStmt*>(stmt)) {
        expressionType(p->value.get());
    } else if (auto i = dynamic_cast<const IfStmt*>(stmt)) {
        expressionType(i->condition.get());
        for (const auto& s : i->thenBlock->statements) statement(s.get());
        if (i->elseBlock)
            for (const auto& s : i->elseBlock->statements) statement(s.get());
    } else if (auto w = dynamic_cast<const WhileStmt*>(stmt)) {
        expressionType(w->condition.get());
        for (const auto& s : w->body->statements) statement(s.get());
    }
}
