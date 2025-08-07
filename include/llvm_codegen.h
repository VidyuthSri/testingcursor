#pragma once

#include "ast.h"
#include "storage.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <memory>

namespace sqlengine {

class LLVMCodeGenerator : public ASTVisitor {
public:
    LLVMCodeGenerator();
    ~LLVMCodeGenerator();
    
    // Main interface
    void generateCode(Statement& statement, Database& database);
    void execute();
    
    // Result access
    const std::vector<Row>& getResults() const { return results_; }
    
    // ASTVisitor implementation
    void visit(LiteralExpression& node) override;
    void visit(ColumnExpression& node) override;
    void visit(BinaryExpression& node) override;
    void visit(UnaryExpression& node) override;
    void visit(SelectStatement& node) override;
    void visit(InsertStatement& node) override;
    void visit(CreateTableStatement& node) override;
    void visit(DropTableStatement& node) override;

private:
    // LLVM components
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::orc::LLJIT> jit_;
    
    // Current state during code generation
    Database* current_database_;
    Table* current_table_;
    llvm::Function* current_function_;
    llvm::Value* current_value_;
    std::vector<Row> results_;
    
    // LLVM types
    llvm::Type* int64_type_;
    llvm::Type* double_type_;
    llvm::Type* bool_type_;
    llvm::Type* ptr_type_;
    llvm::StructType* value_type_;
    llvm::StructType* row_type_;
    
    // Helper methods
    void initializeTypes();
    void createRuntimeFunctions();
    llvm::Function* createFunction(const std::string& name, llvm::FunctionType* type);
    llvm::Value* createValue(const Value& value);
    llvm::Value* loadColumn(const std::string& column_name, llvm::Value* row_ptr);
    llvm::Value* evaluateExpression(Expression& expr, llvm::Value* row_ptr);
    
    // Runtime function declarations
    llvm::Function* print_int_func_;
    llvm::Function* print_double_func_;
    llvm::Function* print_string_func_;
    llvm::Function* compare_values_func_;
    
    // JIT compilation
    void compileAndExecute();
    
    // Helper method for WHERE clause evaluation
    bool evaluateWhereClause(Expression& expr, const Row& row);
};

} // namespace sqlengine