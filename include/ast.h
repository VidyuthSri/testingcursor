#pragma once

#include "types.h"
#include <memory>
#include <vector>
#include <string>

namespace sqlengine {

// Forward declarations
class ASTVisitor;

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Expression nodes
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Literal expression
class LiteralExpression : public Expression {
public:
    Value value;
    
    LiteralExpression(const Value& v) : value(v) {}
    void accept(ASTVisitor& visitor) override;
};

// Column reference expression
class ColumnExpression : public Expression {
public:
    std::string table_name; // optional
    std::string column_name;
    
    ColumnExpression(const std::string& col) : column_name(col) {}
    ColumnExpression(const std::string& table, const std::string& col) 
        : table_name(table), column_name(col) {}
    void accept(ASTVisitor& visitor) override;
};

// Binary operation expression
class BinaryExpression : public Expression {
public:
    enum class Operator {
        ADD, SUBTRACT, MULTIPLY, DIVIDE,
        EQUAL, NOT_EQUAL, LESS_THAN, LESS_EQUAL, GREATER_THAN, GREATER_EQUAL,
        AND, OR
    };
    
    std::unique_ptr<Expression> left;
    Operator op;
    std::unique_ptr<Expression> right;
    
    BinaryExpression(std::unique_ptr<Expression> l, Operator o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override;
};

// Unary operation expression
class UnaryExpression : public Expression {
public:
    enum class Operator {
        NOT, MINUS
    };
    
    Operator op;
    std::unique_ptr<Expression> operand;
    
    UnaryExpression(Operator o, std::unique_ptr<Expression> expr)
        : op(o), operand(std::move(expr)) {}
    void accept(ASTVisitor& visitor) override;
};

// Statement nodes
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// SELECT statement
class SelectStatement : public Statement {
public:
    std::vector<std::unique_ptr<Expression>> select_list;
    std::string from_table;
    std::unique_ptr<Expression> where_clause; // optional
    std::vector<std::string> order_by; // optional
    bool order_desc = false;
    int limit = -1; // optional
    
    void accept(ASTVisitor& visitor) override;
};

// INSERT statement
class InsertStatement : public Statement {
public:
    std::string table_name;
    std::vector<std::string> columns; // optional, if empty insert into all columns
    std::vector<std::vector<std::unique_ptr<Expression>>> values;
    
    void accept(ASTVisitor& visitor) override;
};

// CREATE TABLE statement
class CreateTableStatement : public Statement {
public:
    std::string table_name;
    std::vector<Column> columns;
    
    void accept(ASTVisitor& visitor) override;
};

// DROP TABLE statement
class DropTableStatement : public Statement {
public:
    std::string table_name;
    
    DropTableStatement(const std::string& name) : table_name(name) {}
    void accept(ASTVisitor& visitor) override;
};

// Visitor pattern interface
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(LiteralExpression& node) = 0;
    virtual void visit(ColumnExpression& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(SelectStatement& node) = 0;
    virtual void visit(InsertStatement& node) = 0;
    virtual void visit(CreateTableStatement& node) = 0;
    virtual void visit(DropTableStatement& node) = 0;
};

} // namespace sqlengine