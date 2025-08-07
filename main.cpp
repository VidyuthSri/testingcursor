#include "query_engine.h"
#include <iostream>
#include <iomanip>

using namespace sqlengine;

void printResults(const std::vector<Row>& results, const Table* table = nullptr) {
    if (results.empty()) {
        std::cout << "No results.\n";
        return;
    }
    
    // Print header if we have table schema
    if (table) {
        const auto& schema = table->getSchema();
        for (size_t i = 0; i < schema.getColumnCount(); ++i) {
            if (i > 0) std::cout << " | ";
            std::cout << std::setw(12) << schema.getColumn(i).name;
        }
        std::cout << "\n";
        
        // Print separator
        for (size_t i = 0; i < schema.getColumnCount(); ++i) {
            if (i > 0) std::cout << "-+-";
            std::cout << std::string(12, '-');
        }
        std::cout << "\n";
    }
    
    // Print rows
    for (const auto& row : results) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) std::cout << " | ";
            std::cout << std::setw(12) << row[i].toString();
        }
        std::cout << "\n";
    }
    std::cout << "\n" << results.size() << " row(s) returned.\n\n";
}

void executeSQL(QueryEngine& engine, const std::string& sql) {
    std::cout << "Executing: " << sql << "\n";
    std::cout << std::string(50, '-') << "\n";
    
    auto results = engine.execute(sql);
    
    if (!engine.getLastError().empty()) {
        std::cout << "Error: " << engine.getLastError() << "\n\n";
        return;
    }
    
    printResults(results);
}

int main() {
    std::cout << "SQL Engine with LLVM Demonstration\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    QueryEngine engine;
    
    try {
        // Create a table
        executeSQL(engine, "CREATE TABLE users (id INTEGER, name TEXT, age INTEGER, active BOOLEAN)");
        
        // Insert some data
        executeSQL(engine, "INSERT INTO users VALUES (1, 'Alice', 30, true)");
        executeSQL(engine, "INSERT INTO users VALUES (2, 'Bob', 25, false)");
        executeSQL(engine, "INSERT INTO users VALUES (3, 'Charlie', 35, true)");
        
        // Query all data
        executeSQL(engine, "SELECT * FROM users");
        
        // Query with WHERE clause (simplified - boolean literal)
        executeSQL(engine, "SELECT * FROM users WHERE true");
        
        // Create another table
        executeSQL(engine, "CREATE TABLE products (id INTEGER, name TEXT, price REAL)");
        
        // Insert product data
        executeSQL(engine, "INSERT INTO products VALUES (1, 'Widget', 19.99)");
        executeSQL(engine, "INSERT INTO products VALUES (2, 'Gadget', 29.99)");
        
        // Query products
        executeSQL(engine, "SELECT * FROM products");
        
        // Drop a table
        executeSQL(engine, "DROP TABLE products");
        
        // Try to query dropped table (should fail)
        executeSQL(engine, "SELECT * FROM products");
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "SQL Engine demonstration completed successfully!\n";
    return 0;
}