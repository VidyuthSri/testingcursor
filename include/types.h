#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>

namespace sqlengine {

// Basic SQL data types
enum class DataType {
    INTEGER,
    REAL,
    TEXT,
    BOOLEAN,
    NULL_TYPE
};

// Value representation
class Value {
public:
    using ValueType = std::variant<int64_t, double, std::string, bool, std::nullptr_t>;
    
    Value() : data_(nullptr) {}
    Value(int64_t i) : data_(i) {}
    Value(double d) : data_(d) {}
    Value(const std::string& s) : data_(s) {}
    Value(bool b) : data_(b) {}
    Value(std::nullptr_t) : data_(nullptr) {}
    
    DataType getType() const;
    std::string toString() const;
    
    template<typename T>
    T get() const {
        return std::get<T>(data_);
    }
    
    bool isNull() const {
        return std::holds_alternative<std::nullptr_t>(data_);
    }
    
    // Comparison operators
    bool operator==(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;
    bool operator!=(const Value& other) const;

private:
    ValueType data_;
};

// Column definition
struct Column {
    std::string name;
    DataType type;
    bool nullable = true;
    bool primary_key = false;
    
    Column(const std::string& n, DataType t, bool null = true, bool pk = false)
        : name(n), type(t), nullable(null), primary_key(pk) {}
};

// Row representation
using Row = std::vector<Value>;

// Schema definition
class Schema {
public:
    Schema() = default;
    
    void addColumn(const Column& col);
    const Column& getColumn(size_t index) const;
    const Column* getColumn(const std::string& name) const;
    size_t getColumnIndex(const std::string& name) const;
    size_t getColumnCount() const { return columns_.size(); }
    
    const std::vector<Column>& getColumns() const { return columns_; }

private:
    std::vector<Column> columns_;
    std::unordered_map<std::string, size_t> column_index_;
};

} // namespace sqlengine