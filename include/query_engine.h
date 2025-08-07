#pragma once

#include "storage.h"
#include "lexer.h"
#include "parser.h"
#include "llvm_codegen.h"
#include <string>
#include <memory>

namespace sqlengine {

class QueryEngine {
public:
    QueryEngine();
    ~QueryEngine();
    
    // Execute a SQL query and return results
    std::vector<Row> execute(const std::string& sql);
    
    // Get the underlying database for direct access
    Database& getDatabase() { return database_; }
    const Database& getDatabase() const { return database_; }
    
    // Get last error message
    const std::string& getLastError() const { return last_error_; }
    
private:
    Database database_;
    std::unique_ptr<LLVMCodeGenerator> codegen_;
    std::string last_error_;
    
    void clearError() { last_error_.clear(); }
    void setError(const std::string& error) { last_error_ = error; }
};

} // namespace sqlengine