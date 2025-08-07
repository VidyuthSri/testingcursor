#pragma once

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>

namespace sqlengine {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    
    std::unique_ptr<Statement> parseStatement();
    
private:
    std::vector<Token> tokens_;
    size_t current_;
    
    // Utility methods
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    void consume(TokenType type, const std::string& message);
    
    // Parsing methods
    std::unique_ptr<Statement> parseSelectStatement();
    std::unique_ptr<Statement> parseInsertStatement();
    std::unique_ptr<Statement> parseCreateTableStatement();
    std::unique_ptr<Statement> parseDropTableStatement();
    
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseEqualityExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseTermExpression();
    std::unique_ptr<Expression> parseFactorExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
    
    DataType parseDataType();
    Value parseValue(const Token& token);
    
    // Error handling
    void error(const std::string& message);
};

} // namespace sqlengine