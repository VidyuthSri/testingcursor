#include "storage.h"
#include <stdexcept>
#include <algorithm>

namespace sqlengine {

// Value implementation
DataType Value::getType() const {
    if (std::holds_alternative<int64_t>(data_)) return DataType::INTEGER;
    if (std::holds_alternative<double>(data_)) return DataType::REAL;
    if (std::holds_alternative<std::string>(data_)) return DataType::TEXT;
    if (std::holds_alternative<bool>(data_)) return DataType::BOOLEAN;
    return DataType::NULL_TYPE;
}

std::string Value::toString() const {
    if (isNull()) return "NULL";
    
    switch (getType()) {
        case DataType::INTEGER:
            return std::to_string(std::get<int64_t>(data_));
        case DataType::REAL:
            return std::to_string(std::get<double>(data_));
        case DataType::TEXT:
            return std::get<std::string>(data_);
        case DataType::BOOLEAN:
            return std::get<bool>(data_) ? "true" : "false";
        default:
            return "NULL";
    }
}

bool Value::operator==(const Value& other) const {
    return data_ == other.data_;
}

bool Value::operator<(const Value& other) const {
    if (getType() != other.getType()) {
        return getType() < other.getType();
    }
    
    switch (getType()) {
        case DataType::INTEGER:
            return std::get<int64_t>(data_) < std::get<int64_t>(other.data_);
        case DataType::REAL:
            return std::get<double>(data_) < std::get<double>(other.data_);
        case DataType::TEXT:
            return std::get<std::string>(data_) < std::get<std::string>(other.data_);
        case DataType::BOOLEAN:
            return std::get<bool>(data_) < std::get<bool>(other.data_);
        default:
            return false;
    }
}

bool Value::operator<=(const Value& other) const {
    return *this < other || *this == other;
}

bool Value::operator>(const Value& other) const {
    return !(*this <= other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

// Schema implementation
void Schema::addColumn(const Column& col) {
    column_index_[col.name] = columns_.size();
    columns_.push_back(col);
}

const Column& Schema::getColumn(size_t index) const {
    if (index >= columns_.size()) {
        throw std::out_of_range("Column index out of range");
    }
    return columns_[index];
}

const Column* Schema::getColumn(const std::string& name) const {
    auto it = column_index_.find(name);
    if (it == column_index_.end()) {
        return nullptr;
    }
    return &columns_[it->second];
}

size_t Schema::getColumnIndex(const std::string& name) const {
    auto it = column_index_.find(name);
    if (it == column_index_.end()) {
        throw std::runtime_error("Column not found: " + name);
    }
    return it->second;
}

// Table implementation
Table::Table(const std::string& name, const Schema& schema)
    : name_(name), schema_(schema) {}

void Table::insertRow(const Row& row) {
    if (!validateRow(row)) {
        throw std::runtime_error("Row validation failed");
    }
    rows_.push_back(row);
}

void Table::insertRow(Row&& row) {
    if (!validateRow(row)) {
        throw std::runtime_error("Row validation failed");
    }
    rows_.push_back(std::move(row));
}

bool Table::validateRow(const Row& row) const {
    if (row.size() != schema_.getColumnCount()) {
        return false;
    }
    
    for (size_t i = 0; i < row.size(); ++i) {
        const auto& column = schema_.getColumn(i);
        const auto& value = row[i];
        
        // Check null constraints
        if (!column.nullable && value.isNull()) {
            return false;
        }
        
        // Check type compatibility (basic check)
        if (!value.isNull()) {
            DataType valueType = value.getType();
            if (valueType != column.type) {
                return false;
            }
        }
    }
    
    return true;
}

// Database implementation
void Database::createTable(const std::string& name, const Schema& schema) {
    if (hasTable(name)) {
        throw std::runtime_error("Table already exists: " + name);
    }
    tables_[name] = std::make_unique<Table>(name, schema);
}

Table* Database::getTable(const std::string& name) {
    auto it = tables_.find(name);
    return (it != tables_.end()) ? it->second.get() : nullptr;
}

const Table* Database::getTable(const std::string& name) const {
    auto it = tables_.find(name);
    return (it != tables_.end()) ? it->second.get() : nullptr;
}

bool Database::hasTable(const std::string& name) const {
    return tables_.find(name) != tables_.end();
}

void Database::dropTable(const std::string& name) {
    tables_.erase(name);
}

std::vector<std::string> Database::getTableNames() const {
    std::vector<std::string> names;
    for (const auto& pair : tables_) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace sqlengine