# SQL Engine with LLVM

A basic SQL implementation with in-memory storage and a query engine written in C++ using LLVM for code generation.

## Features

- **In-Memory Storage**: Tables and data are stored in memory for fast access
- **SQL Parser**: Full lexical analysis and parsing of SQL statements
- **LLVM Code Generation**: Query execution using LLVM for JIT compilation
- **Basic SQL Operations**: Support for CREATE TABLE, INSERT, SELECT, and DROP TABLE
- **Expression Evaluation**: Support for arithmetic, comparison, and logical operations
- **Type System**: Support for INTEGER, REAL, TEXT, and BOOLEAN data types

## Supported SQL Statements

### CREATE TABLE
```sql
CREATE TABLE users (
    id INTEGER,
    name TEXT,
    age INTEGER,
    active BOOLEAN
)
```

### INSERT
```sql
INSERT INTO users VALUES (1, 'Alice', 30, true)
INSERT INTO users VALUES (2, 'Bob', 25, false)
```

### SELECT
```sql
SELECT * FROM users
SELECT * FROM users WHERE true
```

### DROP TABLE
```sql
DROP TABLE users
```

## Architecture

The SQL engine consists of several key components:

1. **Lexer** (`lexer.h/cpp`): Tokenizes SQL input into tokens
2. **Parser** (`parser.h/cpp`): Converts tokens into an Abstract Syntax Tree (AST)
3. **AST** (`ast.h/cpp`): Represents SQL statements and expressions as tree structures
4. **Storage** (`storage.h/cpp`): In-memory table and database management
5. **LLVM Code Generator** (`llvm_codegen.h/cpp`): Generates LLVM IR for query execution
6. **Query Engine** (`query_engine.h/cpp`): Coordinates all components

## Building

### Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 6+)
- CMake 3.16+
- LLVM 10+ development libraries

### Installation on Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake llvm-dev libllvm-ocaml-dev
```

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./sql_engine
```

## Example Usage

The main program demonstrates various SQL operations:

```cpp
#include "query_engine.h"

QueryEngine engine;

// Create a table
engine.execute("CREATE TABLE users (id INTEGER, name TEXT, age INTEGER)");

// Insert data
engine.execute("INSERT INTO users VALUES (1, 'Alice', 30)");

// Query data
auto results = engine.execute("SELECT * FROM users");
```

## Design Decisions

### LLVM Integration
- Uses LLVM's ORC JIT for runtime code compilation
- Generates optimized machine code for query execution
- Provides foundation for advanced optimizations

### In-Memory Storage
- All data is stored in memory for simplicity
- Suitable for demonstration and small datasets
- Could be extended to support persistent storage

### Expression System
- Recursive descent parser for SQL expressions
- Visitor pattern for AST traversal
- Type-safe value representation

## Limitations

This is a demonstration/educational implementation with several limitations:

- **No Persistence**: Data is lost when the program exits
- **Limited SQL Support**: Only basic statements are supported
- **No Indexes**: All queries perform full table scans
- **No Transactions**: No ACID properties or transaction support
- **No Joins**: Only single-table queries are supported
- **Simple WHERE Clauses**: Limited expression evaluation in WHERE clauses

## Future Enhancements

Potential improvements could include:

- Persistent storage with a buffer pool manager
- B+ tree indexes for faster queries
- Join operations (nested loop, hash join, sort-merge join)
- Query optimization and cost-based optimization
- Transaction support with MVCC
- More comprehensive SQL standard support
- Network protocol support (e.g., PostgreSQL wire protocol)

## License

This project is for educational purposes. Feel free to use and modify as needed.
