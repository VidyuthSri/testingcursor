#include "query_engine.h"
#include <iostream>
#include <stdexcept>

namespace sqlengine {

QueryEngine::QueryEngine() {
    codegen_ = std::make_unique<LLVMCodeGenerator>();
}

QueryEngine::~QueryEngine() = default;

std::vector<Row> QueryEngine::execute(const std::string& sql) {
    clearError();
    
    try {
        // Step 1: Tokenize the SQL
        Lexer lexer(sql);
        auto tokens = lexer.tokenize();
        
        if (tokens.empty()) {
            setError("No tokens found in SQL");
            return {};
        }
        
        // Step 2: Parse tokens into AST
        Parser parser(tokens);
        auto statement = parser.parseStatement();
        
        if (!statement) {
            setError("Failed to parse SQL statement");
            return {};
        }
        
        // Step 3: Generate and execute code
        codegen_->generateCode(*statement, database_);
        codegen_->execute();
        
        // Step 4: Return results
        return codegen_->getResults();
        
    } catch (const std::exception& e) {
        setError(e.what());
        return {};
    }
}

} // namespace sqlengine