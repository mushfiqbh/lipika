#pragma once
#include <memory>
#include <string>
#include <vector>

enum class ValueType { Integer, Float, Unknown };

struct Expr {
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr : Expr {
    std::string value;
    explicit NumberExpr(std::string v) : value(std::move(v)) {}
};

struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

struct BinaryExpr : Expr {
    ExprPtr left;
    std::string op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, std::string o, ExprPtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
};

struct DeclarationStmt : Stmt {
    ValueType type;
    std::string name;
    ExprPtr initializer;
    DeclarationStmt(ValueType t, std::string n, ExprPtr e)
        : type(t), name(std::move(n)), initializer(std::move(e)) {}
};

struct AssignmentStmt : Stmt {
    std::string name;
    ExprPtr value;
    AssignmentStmt(std::string n, ExprPtr e)
        : name(std::move(n)), value(std::move(e)) {}
};

struct PrintStmt : Stmt {
    ExprPtr value;
    explicit PrintStmt(ExprPtr e) : value(std::move(e)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    std::unique_ptr<BlockStmt> body;
};

struct Program {
    std::vector<StmtPtr> statements;
};
