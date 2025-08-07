#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>

namespace sqlengine {

std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"SELECT", TokenType::SELECT},
    {"FROM", TokenType::FROM},
    {"WHERE", TokenType::WHERE},
    {"INSERT", TokenType::INSERT},
    {"INTO", TokenType::INTO},
    {"VALUES", TokenType::VALUES},
    {"CREATE", TokenType::CREATE},
    {"TABLE", TokenType::TABLE},
    {"DROP", TokenType::DROP},
    {"UPDATE", TokenType::UPDATE},
    {"SET", TokenType::SET},
    {"DELETE", TokenType::DELETE},
    {"AND", TokenType::AND},
    {"OR", TokenType::OR},
    {"NOT", TokenType::NOT},
    {"TRUE", TokenType::TRUE},
    {"FALSE", TokenType::FALSE},
    {"NULL", TokenType::NULL_KW},
    {"AS", TokenType::AS},
    {"ORDER", TokenType::ORDER},
    {"BY", TokenType::BY},
    {"ASC", TokenType::ASC},
    {"DESC", TokenType::DESC},
    {"LIMIT", TokenType::LIMIT},
    {"INTEGER", TokenType::INTEGER_TYPE},
    {"REAL", TokenType::REAL_TYPE},
    {"TEXT", TokenType::TEXT_TYPE},
    {"BOOLEAN", TokenType::BOOLEAN_TYPE},
};

Lexer::Lexer(const std::string& input)
    : input_(input), position_(0), line_(1), column_(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (hasNext()) {
        Token token = nextToken();
        if (token.type != TokenType::INVALID) {
            tokens.push_back(token);
        }
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    
    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (!hasNext()) {
        return makeToken(TokenType::EOF_TOKEN, "");
    }
    
    char c = peek();
    
    // Handle comments
    if (c == '-' && position_ + 1 < input_.length() && input_[position_ + 1] == '-') {
        skipComment();
        return nextToken();
    }
    
    // Handle strings
    if (c == '\'' || c == '"') {
        return readString();
    }
    
    // Handle numbers
    if (isDigit(c)) {
        return readNumber();
    }
    
    // Handle identifiers and keywords
    if (isAlpha(c)) {
        return readIdentifier();
    }
    
    // Handle operators and punctuation
    switch (c) {
        case '(':
            advance();
            return makeToken(TokenType::LEFT_PAREN, "(");
        case ')':
            advance();
            return makeToken(TokenType::RIGHT_PAREN, ")");
        case ',':
            advance();
            return makeToken(TokenType::COMMA, ",");
        case ';':
            advance();
            return makeToken(TokenType::SEMICOLON, ";");
        case '.':
            advance();
            return makeToken(TokenType::DOT, ".");
        case '+':
            advance();
            return makeToken(TokenType::PLUS, "+");
        case '-':
            advance();
            return makeToken(TokenType::MINUS, "-");
        case '*':
            advance();
            return makeToken(TokenType::MULTIPLY, "*");
        case '/':
            advance();
            return makeToken(TokenType::DIVIDE, "/");
        case '=':
            advance();
            return makeToken(TokenType::EQUAL, "=");
        case '<':
            advance();
            if (hasNext() && peek() == '=') {
                advance();
                return makeToken(TokenType::LESS_EQUAL, "<=");
            } else if (hasNext() && peek() == '>') {
                advance();
                return makeToken(TokenType::NOT_EQUAL, "<>");
            }
            return makeToken(TokenType::LESS_THAN, "<");
        case '>':
            advance();
            if (hasNext() && peek() == '=') {
                advance();
                return makeToken(TokenType::GREATER_EQUAL, ">=");
            }
            return makeToken(TokenType::GREATER_THAN, ">");
        case '!':
            advance();
            if (hasNext() && peek() == '=') {
                advance();
                return makeToken(TokenType::NOT_EQUAL, "!=");
            }
            return makeToken(TokenType::INVALID, "!");
        default:
            advance();
            return makeToken(TokenType::INVALID, std::string(1, c));
    }
}

void Lexer::skipWhitespace() {
    while (hasNext() && std::isspace(peek())) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
    }
}

void Lexer::skipComment() {
    while (hasNext() && peek() != '\n') {
        advance();
    }
}

char Lexer::peek() const {
    if (position_ >= input_.length()) {
        return '\0';
    }
    return input_[position_];
}

char Lexer::advance() {
    if (position_ >= input_.length()) {
        return '\0';
    }
    char c = input_[position_++];
    column_++;
    return c;
}

Token Lexer::makeToken(TokenType type, const std::string& value) const {
    return Token(type, value, position_, line_, column_);
}

Token Lexer::readString() {
    char quote = advance(); // consume opening quote
    std::string value;
    
    while (hasNext() && peek() != quote) {
        char c = advance();
        if (c == '\\' && hasNext()) {
            // Handle escape sequences
            char escaped = advance();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '\'': value += '\''; break;
                case '"': value += '"'; break;
                default: value += escaped; break;
            }
        } else {
            value += c;
        }
    }
    
    if (hasNext() && peek() == quote) {
        advance(); // consume closing quote
    }
    
    return makeToken(TokenType::STRING_LITERAL, value);
}

Token Lexer::readNumber() {
    std::string value;
    bool hasDecimal = false;
    
    while (hasNext() && (isDigit(peek()) || (!hasDecimal && peek() == '.'))) {
        if (peek() == '.') {
            hasDecimal = true;
        }
        value += advance();
    }
    
    TokenType type = hasDecimal ? TokenType::REAL_LITERAL : TokenType::INTEGER_LITERAL;
    return makeToken(type, value);
}

Token Lexer::readIdentifier() {
    std::string value;
    
    while (hasNext() && isAlphaNumeric(peek())) {
        value += advance();
    }
    
    // Convert to uppercase for keyword lookup
    std::string upper_value = value;
    std::transform(upper_value.begin(), upper_value.end(), upper_value.begin(), ::toupper);
    
    auto it = keywords_.find(upper_value);
    if (it != keywords_.end()) {
        return makeToken(it->second, value);
    }
    
    return makeToken(TokenType::IDENTIFIER, value);
}

} // namespace sqlengine