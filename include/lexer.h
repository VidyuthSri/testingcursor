#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace sqlengine {

enum class TokenType {
    // Literals
    INTEGER_LITERAL,
    REAL_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    
    // Identifiers
    IDENTIFIER,
    
    // Keywords
    SELECT,
    FROM,
    WHERE,
    INSERT,
    INTO,
    VALUES,
    CREATE,
    TABLE,
    DROP,
    UPDATE,
    SET,
    DELETE,
    AND,
    OR,
    NOT,
    TRUE,
    FALSE,
    NULL_KW,
    AS,
    ORDER,
    BY,
    ASC,
    DESC,
    LIMIT,
    
    // Data types
    INTEGER_TYPE,
    REAL_TYPE,
    TEXT_TYPE,
    BOOLEAN_TYPE,
    
    // Operators
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    
    // Punctuation
    LEFT_PAREN,
    RIGHT_PAREN,
    COMMA,
    SEMICOLON,
    DOT,
    
    // Special
    EOF_TOKEN,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    size_t position;
    size_t line;
    size_t column;
    
    Token(TokenType t, const std::string& v, size_t pos, size_t ln, size_t col)
        : type(t), value(v), position(pos), line(ln), column(col) {}
};

class Lexer {
public:
    Lexer(const std::string& input);
    
    std::vector<Token> tokenize();
    Token nextToken();
    
    bool hasNext() const { return position_ < input_.length(); }
    
private:
    std::string input_;
    size_t position_;
    size_t line_;
    size_t column_;
    
    static std::unordered_map<std::string, TokenType> keywords_;
    
    void skipWhitespace();
    void skipComment();
    char peek() const;
    char advance();
    Token makeToken(TokenType type, const std::string& value) const;
    Token readString();
    Token readNumber();
    Token readIdentifier();
    
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }
};

} // namespace sqlengine