#include "parser.h"
#include <stdexcept>
#include <algorithm>

namespace sqlengine {

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens), current_(0) {}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::SELECT)) {
        return parseSelectStatement();
    } else if (match(TokenType::INSERT)) {
        return parseInsertStatement();
    } else if (match(TokenType::CREATE)) {
        return parseCreateTableStatement();
    } else if (match(TokenType::DROP)) {
        return parseDropTableStatement();
    } else {
        error("Expected statement");
        return nullptr;
    }
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    error(message);
}

std::unique_ptr<Statement> Parser::parseSelectStatement() {
    auto stmt = std::make_unique<SelectStatement>();
    
    // Parse SELECT list
    do {
        if (match(TokenType::MULTIPLY)) {
            // SELECT * - add all columns (handled later)
            stmt->select_list.push_back(std::make_unique<ColumnExpression>("*"));
        } else {
            stmt->select_list.push_back(parseExpression());
        }
    } while (match(TokenType::COMMA));
    
    // Parse FROM clause
    consume(TokenType::FROM, "Expected 'FROM' after SELECT list");
    consume(TokenType::IDENTIFIER, "Expected table name after FROM");
    stmt->from_table = previous().value;
    
    // Parse optional WHERE clause
    if (match(TokenType::WHERE)) {
        stmt->where_clause = parseExpression();
    }
    
    // Parse optional ORDER BY clause
    if (match(TokenType::ORDER)) {
        consume(TokenType::BY, "Expected 'BY' after ORDER");
        do {
            consume(TokenType::IDENTIFIER, "Expected column name in ORDER BY");
            stmt->order_by.push_back(previous().value);
        } while (match(TokenType::COMMA));
        
        if (match(TokenType::DESC)) {
            stmt->order_desc = true;
        } else {
            match(TokenType::ASC); // optional
        }
    }
    
    // Parse optional LIMIT clause
    if (match(TokenType::LIMIT)) {
        consume(TokenType::INTEGER_LITERAL, "Expected number after LIMIT");
        stmt->limit = std::stoi(previous().value);
    }
    
    return std::move(stmt);
}

std::unique_ptr<Statement> Parser::parseInsertStatement() {
    auto stmt = std::make_unique<InsertStatement>();
    
    consume(TokenType::INTO, "Expected 'INTO' after INSERT");
    consume(TokenType::IDENTIFIER, "Expected table name");
    stmt->table_name = previous().value;
    
    // Parse optional column list
    if (match(TokenType::LEFT_PAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expected column name");
            stmt->columns.push_back(previous().value);
        } while (match(TokenType::COMMA));
        consume(TokenType::RIGHT_PAREN, "Expected ')' after column list");
    }
    
    consume(TokenType::VALUES, "Expected 'VALUES'");
    
    // Parse value lists
    do {
        consume(TokenType::LEFT_PAREN, "Expected '(' before values");
        std::vector<std::unique_ptr<Expression>> values;
        do {
            values.push_back(parseExpression());
        } while (match(TokenType::COMMA));
        consume(TokenType::RIGHT_PAREN, "Expected ')' after values");
        stmt->values.push_back(std::move(values));
    } while (match(TokenType::COMMA));
    
    return std::move(stmt);
}

std::unique_ptr<Statement> Parser::parseCreateTableStatement() {
    auto stmt = std::make_unique<CreateTableStatement>();
    
    consume(TokenType::TABLE, "Expected 'TABLE' after CREATE");
    consume(TokenType::IDENTIFIER, "Expected table name");
    stmt->table_name = previous().value;
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after table name");
    
    // Parse column definitions
    do {
        consume(TokenType::IDENTIFIER, "Expected column name");
        std::string column_name = previous().value;
        
        DataType data_type = parseDataType();
        
        bool nullable = true;
        bool primary_key = false;
        
        // Parse column constraints (simplified)
        while (match({TokenType::NOT, TokenType::IDENTIFIER})) {
            if (previous().value == "NOT") {
                consume(TokenType::NULL_KW, "Expected 'NULL' after NOT");
                nullable = false;
            } else if (previous().value == "PRIMARY") {
                consume(TokenType::IDENTIFIER, "Expected 'KEY' after PRIMARY");
                if (previous().value != "KEY") {
                    error("Expected 'KEY' after PRIMARY");
                }
                primary_key = true;
            }
        }
        
        stmt->columns.emplace_back(column_name, data_type, nullable, primary_key);
    } while (match(TokenType::COMMA));
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after column definitions");
    
    return std::move(stmt);
}

std::unique_ptr<Statement> Parser::parseDropTableStatement() {
    consume(TokenType::TABLE, "Expected 'TABLE' after DROP");
    consume(TokenType::IDENTIFIER, "Expected table name");
    return std::make_unique<DropTableStatement>(previous().value);
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseOrExpression();
}

std::unique_ptr<Expression> Parser::parseOrExpression() {
    auto expr = parseAndExpression();
    
    while (match(TokenType::OR)) {
        auto op = BinaryExpression::Operator::OR;
        auto right = parseAndExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseAndExpression() {
    auto expr = parseEqualityExpression();
    
    while (match(TokenType::AND)) {
        auto op = BinaryExpression::Operator::AND;
        auto right = parseEqualityExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseEqualityExpression() {
    auto expr = parseComparisonExpression();
    
    while (match({TokenType::NOT_EQUAL, TokenType::EQUAL})) {
        auto op = (previous().type == TokenType::EQUAL) ? 
                  BinaryExpression::Operator::EQUAL : 
                  BinaryExpression::Operator::NOT_EQUAL;
        auto right = parseComparisonExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseComparisonExpression() {
    auto expr = parseTermExpression();
    
    while (match({TokenType::GREATER_THAN, TokenType::GREATER_EQUAL, 
                  TokenType::LESS_THAN, TokenType::LESS_EQUAL})) {
        BinaryExpression::Operator op;
        switch (previous().type) {
            case TokenType::GREATER_THAN: op = BinaryExpression::Operator::GREATER_THAN; break;
            case TokenType::GREATER_EQUAL: op = BinaryExpression::Operator::GREATER_EQUAL; break;
            case TokenType::LESS_THAN: op = BinaryExpression::Operator::LESS_THAN; break;
            case TokenType::LESS_EQUAL: op = BinaryExpression::Operator::LESS_EQUAL; break;
            default: op = BinaryExpression::Operator::EQUAL; break;
        }
        auto right = parseTermExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseTermExpression() {
    auto expr = parseFactorExpression();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        auto op = (previous().type == TokenType::PLUS) ? 
                  BinaryExpression::Operator::ADD : 
                  BinaryExpression::Operator::SUBTRACT;
        auto right = parseFactorExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseFactorExpression() {
    auto expr = parseUnaryExpression();
    
    while (match({TokenType::DIVIDE, TokenType::MULTIPLY})) {
        auto op = (previous().type == TokenType::MULTIPLY) ? 
                  BinaryExpression::Operator::MULTIPLY : 
                  BinaryExpression::Operator::DIVIDE;
        auto right = parseUnaryExpression();
        expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseUnaryExpression() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        auto op = (previous().type == TokenType::NOT) ? 
                  UnaryExpression::Operator::NOT : 
                  UnaryExpression::Operator::MINUS;
        auto expr = parseUnaryExpression();
        return std::make_unique<UnaryExpression>(op, std::move(expr));
    }
    
    return parsePrimaryExpression();
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
    if (match({TokenType::TRUE, TokenType::FALSE})) {
        bool value = (previous().type == TokenType::TRUE);
        return std::make_unique<LiteralExpression>(Value(value));
    }
    
    if (match(TokenType::NULL_KW)) {
        return std::make_unique<LiteralExpression>(Value(nullptr));
    }
    
    if (match({TokenType::INTEGER_LITERAL, TokenType::REAL_LITERAL, TokenType::STRING_LITERAL})) {
        Value value = parseValue(previous());
        return std::make_unique<LiteralExpression>(value);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous().value;
        if (match(TokenType::DOT)) {
            consume(TokenType::IDENTIFIER, "Expected column name after '.'");
            return std::make_unique<ColumnExpression>(name, previous().value);
        }
        return std::make_unique<ColumnExpression>(name);
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    error("Expected expression");
    return nullptr;
}

DataType Parser::parseDataType() {
    if (match(TokenType::INTEGER_TYPE)) {
        return DataType::INTEGER;
    } else if (match(TokenType::REAL_TYPE)) {
        return DataType::REAL;
    } else if (match(TokenType::TEXT_TYPE)) {
        return DataType::TEXT;
    } else if (match(TokenType::BOOLEAN_TYPE)) {
        return DataType::BOOLEAN;
    } else {
        error("Expected data type");
        return DataType::TEXT;
    }
}

Value Parser::parseValue(const Token& token) {
    switch (token.type) {
        case TokenType::INTEGER_LITERAL:
            return Value(static_cast<int64_t>(std::stoll(token.value)));
        case TokenType::REAL_LITERAL:
            return Value(std::stod(token.value));
        case TokenType::STRING_LITERAL:
            return Value(token.value);
        case TokenType::TRUE:
            return Value(true);
        case TokenType::FALSE:
            return Value(false);
        default:
            error("Invalid literal value");
            return Value(nullptr);
    }
}

void Parser::error(const std::string& message) {
    std::string error_msg = "Parse error at token " + std::to_string(current_) + ": " + message;
    if (!isAtEnd()) {
        error_msg += " (got '" + peek().value + "')";
    }
    throw std::runtime_error(error_msg);
}

} // namespace sqlengine