#include "llvm_codegen.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <iostream>

namespace sqlengine {

LLVMCodeGenerator::LLVMCodeGenerator() {
    // Initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("sql_query", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
    
    // Initialize JIT
    auto jit_or_err = llvm::orc::LLJITBuilder().create();
    if (jit_or_err) {
        jit_ = std::move(*jit_or_err);
    }
    
    initializeTypes();
    createRuntimeFunctions();
}

LLVMCodeGenerator::~LLVMCodeGenerator() = default;

void LLVMCodeGenerator::initializeTypes() {
    int64_type_ = llvm::Type::getInt64Ty(*context_);
    double_type_ = llvm::Type::getDoubleTy(*context_);
    bool_type_ = llvm::Type::getInt1Ty(*context_);
    ptr_type_ = llvm::PointerType::get(*context_, 0);
    
    // Create Value struct type (simplified)
    std::vector<llvm::Type*> value_fields = {
        llvm::Type::getInt32Ty(*context_), // type tag
        llvm::Type::getInt64Ty(*context_)  // data (union simplified as int64)
    };
    value_type_ = llvm::StructType::create(*context_, value_fields, "Value");
    
    // Create Row type (simplified as array of values)
    row_type_ = llvm::StructType::create(*context_, {
        llvm::ArrayType::get(value_type_, 16) // max 16 columns
    }, "Row");
}

void LLVMCodeGenerator::createRuntimeFunctions() {
    // Create function prototypes for runtime support
    auto int_arg = std::vector<llvm::Type*>{int64_type_};
    auto double_arg = std::vector<llvm::Type*>{double_type_};
    auto string_arg = std::vector<llvm::Type*>{ptr_type_};
    auto compare_args = std::vector<llvm::Type*>{llvm::PointerType::get(value_type_, 0), llvm::PointerType::get(value_type_, 0)};
    
    auto void_type = llvm::Type::getVoidTy(*context_);
    
    print_int_func_ = llvm::Function::Create(
        llvm::FunctionType::get(void_type, int_arg, false),
        llvm::Function::ExternalLinkage, "print_int", module_.get());
    
    print_double_func_ = llvm::Function::Create(
        llvm::FunctionType::get(void_type, double_arg, false),
        llvm::Function::ExternalLinkage, "print_double", module_.get());
    
    print_string_func_ = llvm::Function::Create(
        llvm::FunctionType::get(void_type, string_arg, false),
        llvm::Function::ExternalLinkage, "print_string", module_.get());
    
    compare_values_func_ = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), compare_args, false),
        llvm::Function::ExternalLinkage, "compare_values", module_.get());
}

void LLVMCodeGenerator::generateCode(Statement& statement, Database& database) {
    current_database_ = &database;
    results_.clear();
    
    // Generate LLVM IR for the statement
    statement.accept(*this);
    
    // Verify the module
    std::string error_str;
    if (llvm::verifyModule(*module_, &llvm::errs())) {
        throw std::runtime_error("LLVM module verification failed");
    }
}

void LLVMCodeGenerator::execute() {
    compileAndExecute();
}

void LLVMCodeGenerator::visit(LiteralExpression& node) {
    current_value_ = createValue(node.value);
}

void LLVMCodeGenerator::visit(ColumnExpression& node) {
    // This will be implemented when we have a row context
    // For now, create a placeholder
    current_value_ = llvm::ConstantInt::get(int64_type_, 0);
}

void LLVMCodeGenerator::visit(BinaryExpression& node) {
    // Visit left operand
    node.left->accept(*this);
    llvm::Value* left = current_value_;
    
    // Visit right operand
    node.right->accept(*this);
    llvm::Value* right = current_value_;
    
    // Generate operation based on operator
    switch (node.op) {
        case BinaryExpression::Operator::ADD:
            current_value_ = builder_->CreateAdd(left, right, "add_tmp");
            break;
        case BinaryExpression::Operator::SUBTRACT:
            current_value_ = builder_->CreateSub(left, right, "sub_tmp");
            break;
        case BinaryExpression::Operator::MULTIPLY:
            current_value_ = builder_->CreateMul(left, right, "mul_tmp");
            break;
        case BinaryExpression::Operator::DIVIDE:
            current_value_ = builder_->CreateSDiv(left, right, "div_tmp");
            break;
        case BinaryExpression::Operator::EQUAL:
            current_value_ = builder_->CreateICmpEQ(left, right, "eq_tmp");
            break;
        case BinaryExpression::Operator::NOT_EQUAL:
            current_value_ = builder_->CreateICmpNE(left, right, "ne_tmp");
            break;
        case BinaryExpression::Operator::LESS_THAN:
            current_value_ = builder_->CreateICmpSLT(left, right, "lt_tmp");
            break;
        case BinaryExpression::Operator::LESS_EQUAL:
            current_value_ = builder_->CreateICmpSLE(left, right, "le_tmp");
            break;
        case BinaryExpression::Operator::GREATER_THAN:
            current_value_ = builder_->CreateICmpSGT(left, right, "gt_tmp");
            break;
        case BinaryExpression::Operator::GREATER_EQUAL:
            current_value_ = builder_->CreateICmpSGE(left, right, "ge_tmp");
            break;
        case BinaryExpression::Operator::AND:
            current_value_ = builder_->CreateAnd(left, right, "and_tmp");
            break;
        case BinaryExpression::Operator::OR:
            current_value_ = builder_->CreateOr(left, right, "or_tmp");
            break;
    }
}

void LLVMCodeGenerator::visit(UnaryExpression& node) {
    node.operand->accept(*this);
    llvm::Value* operand = current_value_;
    
    switch (node.op) {
        case UnaryExpression::Operator::NOT:
            current_value_ = builder_->CreateNot(operand, "not_tmp");
            break;
        case UnaryExpression::Operator::MINUS:
            current_value_ = builder_->CreateNeg(operand, "neg_tmp");
            break;
    }
}

void LLVMCodeGenerator::visit(SelectStatement& node) {
    // Get the table
    current_table_ = current_database_->getTable(node.from_table);
    if (!current_table_) {
        throw std::runtime_error("Table not found: " + node.from_table);
    }
    
    // Create a function for the SELECT query
    auto void_type = llvm::Type::getVoidTy(*context_);
    auto select_func_type = llvm::FunctionType::get(void_type, {}, false);
    current_function_ = createFunction("select_query", select_func_type);
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context_, "entry", current_function_);
    builder_->SetInsertPoint(entry);
    
    // For simplicity, just iterate through rows and apply WHERE clause
    // In a real implementation, this would be more sophisticated
    
    // Simple interpretation for demonstration
    const auto& rows = current_table_->getRows();
    for (const auto& row : rows) {
        bool include_row = true;
        
        // Apply WHERE clause if present
        if (node.where_clause) {
            // For simplicity, we'll evaluate this in C++ rather than LLVM
            // In a full implementation, this would be JIT compiled
            include_row = evaluateWhereClause(*node.where_clause, row);
        }
        
        if (include_row) {
            results_.push_back(row);
        }
    }
    
    builder_->CreateRetVoid();
}

void LLVMCodeGenerator::visit(InsertStatement& node) {
    // Get the table
    current_table_ = current_database_->getTable(node.table_name);
    if (!current_table_) {
        throw std::runtime_error("Table not found: " + node.table_name);
    }
    
    // For simplicity, execute the insert directly
    for (const auto& value_list : node.values) {
        Row row;
        for (const auto& expr : value_list) {
            // Evaluate expression to get value
            if (auto literal = dynamic_cast<LiteralExpression*>(expr.get())) {
                row.push_back(literal->value);
            } else {
                throw std::runtime_error("Complex expressions in INSERT not yet supported");
            }
        }
        current_table_->insertRow(std::move(row));
    }
}

void LLVMCodeGenerator::visit(CreateTableStatement& node) {
    Schema schema;
    for (const auto& col : node.columns) {
        schema.addColumn(col);
    }
    current_database_->createTable(node.table_name, schema);
}

void LLVMCodeGenerator::visit(DropTableStatement& node) {
    current_database_->dropTable(node.table_name);
}

llvm::Function* LLVMCodeGenerator::createFunction(const std::string& name, llvm::FunctionType* type) {
    return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, module_.get());
}

llvm::Value* LLVMCodeGenerator::createValue(const Value& value) {
    switch (value.getType()) {
        case DataType::INTEGER:
            return llvm::ConstantInt::get(int64_type_, value.get<int64_t>());
        case DataType::REAL:
            return llvm::ConstantFP::get(double_type_, value.get<double>());
        case DataType::BOOLEAN:
            return llvm::ConstantInt::get(bool_type_, value.get<bool>());
        case DataType::TEXT:
            // For strings, we'd need to create a global string constant
            // Simplified for now
            return llvm::ConstantInt::get(int64_type_, 0);
        default:
            return llvm::ConstantInt::get(int64_type_, 0);
    }
}

void LLVMCodeGenerator::compileAndExecute() {
    if (!jit_) {
        // Fallback to direct execution for demonstration
        return;
    }
    
    // Add the module to JIT
    auto tsm = llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_));
    auto err = jit_->addIRModule(std::move(tsm));
    if (err) {
        throw std::runtime_error("Failed to add module to JIT");
    }
    
    // Look up the function
    auto symbol = jit_->lookup("select_query");
    if (!symbol) {
        // Function might not exist for non-SELECT statements
        return;
    }
    
    // Execute the function
    auto func = symbol->getValue();
    auto func_ptr = reinterpret_cast<void(*)()>(func);
    func_ptr();
}

// Helper method for WHERE clause evaluation (simplified C++ version)
bool LLVMCodeGenerator::evaluateWhereClause(Expression& expr, const Row& row) {
    if (auto binary = dynamic_cast<BinaryExpression*>(&expr)) {
        bool left_val = evaluateWhereClause(*binary->left, row);
        bool right_val = evaluateWhereClause(*binary->right, row);
        
        switch (binary->op) {
            case BinaryExpression::Operator::AND:
                return left_val && right_val;
            case BinaryExpression::Operator::OR:
                return left_val || right_val;
            default:
                return true; // Simplified
        }
    } else if (auto literal = dynamic_cast<LiteralExpression*>(&expr)) {
        if (literal->value.getType() == DataType::BOOLEAN) {
            return literal->value.get<bool>();
        }
    }
    
    return true; // Default to include row
}

} // namespace sqlengine