#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace sqlengine {

// Table class for in-memory storage
class Table {
public:
    Table(const std::string& name, const Schema& schema);
    
    // Table operations
    void insertRow(const Row& row);
    void insertRow(Row&& row);
    
    const std::vector<Row>& getRows() const { return rows_; }
    const Schema& getSchema() const { return schema_; }
    const std::string& getName() const { return name_; }
    
    size_t getRowCount() const { return rows_.size(); }
    
    // Validate row against schema
    bool validateRow(const Row& row) const;

private:
    std::string name_;
    Schema schema_;
    std::vector<Row> rows_;
};

// Database class to manage multiple tables
class Database {
public:
    Database() = default;
    
    // Table management
    void createTable(const std::string& name, const Schema& schema);
    Table* getTable(const std::string& name);
    const Table* getTable(const std::string& name) const;
    
    bool hasTable(const std::string& name) const;
    void dropTable(const std::string& name);
    
    std::vector<std::string> getTableNames() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Table>> tables_;
};

} // namespace sqlengine