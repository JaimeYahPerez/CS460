#pragma once
#include <string>

enum class TokenType {
    // Single-char punctuation
    L_PAREN, R_PAREN,
    L_BRACKET, R_BRACKET,
    L_BRACE, R_BRACE,
    DOUBLE_QUOTE, SINGLE_QUOTE,
    SEMICOLON, COMMA,

    // Operators (single-char)
    ASSIGNMENT_OPERATOR, // =
    PLUS, MINUS, ASTERISK, DIVIDE, MODULO, CARET,
    LT, GT,
    BOOLEAN_NOT,          // !

    // Operators (multi-char)
    LT_EQUAL,             // <=
    GT_EQUAL,             // >=
    BOOLEAN_AND,          // &&
    BOOLEAN_OR,           // ||
    BOOLEAN_EQUAL,        // ==
    BOOLEAN_NOT_EQUAL,    // !=

    // “Value” tokens
    IDENTIFIER,
    INTEGER,
    STRING,               // string contents inside quotes

    END_OF_FILE
};

struct Token {
    TokenType type{};
    std::string lexeme;   // exact bytes for this token
    int line = 1;         // 1-based line number where token begins
};

inline const char* token_type_name(TokenType t) {
    switch (t) {
    case TokenType::L_PAREN: return "L_PAREN";
    case TokenType::R_PAREN: return "R_PAREN";
    case TokenType::L_BRACKET: return "L_BRACKET";
    case TokenType::R_BRACKET: return "R_BRACKET";
    case TokenType::L_BRACE: return "L_BRACE";
    case TokenType::R_BRACE: return "R_BRACE";
    case TokenType::DOUBLE_QUOTE: return "DOUBLE_QUOTE";
    case TokenType::SINGLE_QUOTE: return "SINGLE_QUOTE";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::COMMA: return "COMMA";

    case TokenType::ASSIGNMENT_OPERATOR: return "ASSIGNMENT_OPERATOR";
    case TokenType::PLUS: return "PLUS";
    case TokenType::MINUS: return "MINUS";
    case TokenType::ASTERISK: return "ASTERISK";
    case TokenType::DIVIDE: return "DIVIDE";
    case TokenType::MODULO: return "MODULO";
    case TokenType::CARET: return "CARET";
    case TokenType::LT: return "LT";
    case TokenType::GT: return "GT";
    case TokenType::BOOLEAN_NOT: return "BOOLEAN_NOT";

    case TokenType::LT_EQUAL: return "LT_EQUAL";
    case TokenType::GT_EQUAL: return "GT_EQUAL";
    case TokenType::BOOLEAN_AND: return "BOOLEAN_AND";
    case TokenType::BOOLEAN_OR: return "BOOLEAN_OR";
    case TokenType::BOOLEAN_EQUAL: return "BOOLEAN_EQUAL";
    case TokenType::BOOLEAN_NOT_EQUAL: return "BOOLEAN_NOT_EQUAL";

    case TokenType::IDENTIFIER: return "IDENTIFIER";
    case TokenType::INTEGER: return "INTEGER";
    case TokenType::STRING: return "STRING";

    case TokenType::END_OF_FILE: return "END_OF_FILE";
    }
    return "UNKNOWN";
}