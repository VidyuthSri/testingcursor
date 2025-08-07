#include "ast.h"

namespace sqlengine {

void LiteralExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ColumnExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BinaryExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void UnaryExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void SelectStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void InsertStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void CreateTableStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void DropTableStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace sqlengine