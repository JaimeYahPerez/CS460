#include "parser.h"

#include <initializer_list>
#include <sstream>
#include <utility>

ParseError::ParseError(int line, const std::string& message)
    : std::runtime_error("Syntax error on line " + std::to_string(line) + ": " + message),
      line_(line),
      plainMessage_(message) {
}

int ParseError::line() const noexcept {
    return line_;
}

const std::string& ParseError::plain_message() const noexcept {
    return plainMessage_;
}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) {
        return type == TokenType::END_OF_FILE;
    }
    return peek().type == type;
}

const Token& Parser::advance() {
    if (!is_at_end()) {
        ++current_;
    }
    return previous();
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match_any(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    syntax_error(peek(), message);
}

[[noreturn]] void Parser::syntax_error(const Token& tok, const std::string& message) const {
    throw ParseError(tok.line, message);
}

bool Parser::is_type_specifier(TokenType type) const {
    return type == TokenType::INT
        || type == TokenType::CHAR
        || type == TokenType::BOOL
        || type == TokenType::VOID;
}

bool Parser::is_statement_start(TokenType type) const {
    return type == TokenType::IF
        || type == TokenType::WHILE
        || type == TokenType::FOR
        || type == TokenType::RETURN
        || type == TokenType::IDENTIFIER
        || type == TokenType::PRINTF
        || type == TokenType::L_BRACE
        || (is_type_specifier(type) && type != TokenType::VOID);
}

bool Parser::is_reserved_word_token(TokenType type) const {
    switch (type) {
    case TokenType::FUNCTION:
    case TokenType::PROCEDURE:
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::BOOL:
    case TokenType::VOID:
    case TokenType::IF:
    case TokenType::ELSE:
    case TokenType::WHILE:
    case TokenType::FOR:
    case TokenType::RETURN:
    case TokenType::TRUE_TOKEN:
    case TokenType::FALSE_TOKEN:
    case TokenType::PRINTF:
        return true;
    default:
        return false;
    }
}

CSTNode* Parser::make_token_node(const Token& tok) const {
    return make_node(tok.lexeme, tok.line);
}

CSTNode* Parser::parse_decl_identifier(const std::string& role_name) {
    const Token& tok = peek();

    if (tok.type == TokenType::IDENTIFIER) {
        return make_token_node(advance());
    }

    if (is_reserved_word_token(tok.type)) {
        syntax_error(tok, "reserved word \"" + tok.lexeme + "\" cannot be used for the name of a " + role_name + ".");
    }

    syntax_error(tok, "expected identifier for " + role_name);
}

CSTNode* Parser::parse_program() {
    CSTNode* root = make_node("program");

    while (!is_at_end()) {
        add_child(root, parse_top_level_decl());
    }

    return root;
}

CSTNode* Parser::parse_top_level_decl() {
    if (check(TokenType::FUNCTION)) {
        return parse_function_decl();
    }
    if (check(TokenType::PROCEDURE)) {
        return parse_procedure_decl();
    }
    if (is_type_specifier(peek().type) && !check(TokenType::VOID)) {
        return parse_declaration_stmt();
    }

    syntax_error(peek(), "expected top-level declaration beginning with \"function\", \"procedure\", or a variable declaration");
}

CSTNode* Parser::parse_function_decl() {
    CSTNode* node = make_node("function_decl");

    add_child(node, make_token_node(expect(TokenType::FUNCTION, "expected \"function\"")));
    add_child(node, parse_type_specifier());
    add_child(node, parse_decl_identifier("function"));
    add_child(node, make_token_node(expect(TokenType::L_PAREN, "expected \"(\" after function name")));
    add_child(node, parse_parameter_list());
    add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after parameter list")));
    add_child(node, parse_block());

    return node;
}

CSTNode* Parser::parse_procedure_decl() {
    CSTNode* node = make_node("procedure_decl");

    add_child(node, make_token_node(expect(TokenType::PROCEDURE, "expected \"procedure\"")));
    add_child(node, parse_decl_identifier("procedure"));
    add_child(node, make_token_node(expect(TokenType::L_PAREN, "expected \"(\" after procedure name")));
    add_child(node, parse_parameter_list());
    add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after parameter list")));
    add_child(node, parse_block());

    return node;
}

CSTNode* Parser::parse_parameter_list() {
    CSTNode* node = make_node("parameter_list");

    if (check(TokenType::VOID)) {
        add_child(node, make_token_node(advance()));
        return node;
    }

    if (is_type_specifier(peek().type) && !check(TokenType::VOID)) {
        add_child(node, parse_parameter());
        while (match(TokenType::COMMA)) {
            add_child(node, make_token_node(previous()));
            add_child(node, parse_parameter());
        }
    }

    return node;
}

CSTNode* Parser::parse_parameter() {
    CSTNode* node = make_node("parameter");

    add_child(node, parse_type_specifier());
    add_child(node, parse_decl_identifier("variable"));

    if (match(TokenType::L_BRACKET)) {
        add_child(node, make_token_node(previous()));
        if (!check(TokenType::R_BRACKET)) {
            add_child(node, parse_array_size());
        }
        add_child(node, make_token_node(expect(TokenType::R_BRACKET, "expected \"]\" in parameter declaration")));
    }

    return node;
}

CSTNode* Parser::parse_type_specifier() {
    CSTNode* node = make_node("type_specifier");

    if (match_any({TokenType::INT, TokenType::CHAR, TokenType::BOOL, TokenType::VOID})) {
        add_child(node, make_token_node(previous()));
        return node;
    }

    syntax_error(peek(), "expected type specifier");
}

CSTNode* Parser::parse_block() {
    CSTNode* node = make_node("block");

    add_child(node, make_token_node(expect(TokenType::L_BRACE, "expected \"{\"")));

    while (!check(TokenType::R_BRACE) && !is_at_end()) {
        add_child(node, parse_statement());
    }

    add_child(node, make_token_node(expect(TokenType::R_BRACE, "expected \"}\"")));
    return node;
}

CSTNode* Parser::parse_statement() {
    if (is_type_specifier(peek().type) && !check(TokenType::VOID)) {
        return parse_declaration_stmt();
    }
    if (check(TokenType::IF)) {
        return parse_if_stmt();
    }
    if (check(TokenType::WHILE)) {
        return parse_while_stmt();
    }
    if (check(TokenType::FOR)) {
        return parse_for_stmt();
    }
    if (check(TokenType::RETURN)) {
        return parse_return_stmt();
    }
    if (check(TokenType::L_BRACE)) {
        return parse_block();
    }
    if (check(TokenType::IDENTIFIER) || check(TokenType::PRINTF)) {
        return parse_assignment_or_call_stmt();
    }

    syntax_error(peek(), "unexpected token at start of statement");
}

CSTNode* Parser::parse_array_size() {
    const Token& tok = peek();

    if (match(TokenType::INTEGER)) {
        CSTNode* node = make_node("integer_literal");
        add_child(node, make_token_node(previous()));
        return node;
    }

    if (match(TokenType::PLUS)) {
        Token plusTok = previous();
        if (!match(TokenType::INTEGER)) {
            syntax_error(peek(), "array declaration size must be a positive integer.");
        }
        CSTNode* node = make_node("unary_expr");
        add_child(node, make_token_node(plusTok));
        CSTNode* lit = make_node("integer_literal");
        add_child(lit, make_token_node(previous()));
        add_child(node, lit);
        return node;
    }

    if (check(TokenType::MINUS)) {
        syntax_error(tok, "array declaration size must be a positive integer.");
    }

    syntax_error(tok, "array declaration size must be a positive integer.");
}

CSTNode* Parser::parse_declaration_stmt() {
    CSTNode* node = make_node("declaration_stmt");

    add_child(node, parse_type_specifier());
    add_child(node, parse_decl_identifier("variable"));

    if (match(TokenType::L_BRACKET)) {
        add_child(node, make_token_node(previous()));
        add_child(node, parse_array_size());
        add_child(node, make_token_node(expect(TokenType::R_BRACKET, "expected \"]\" in array declaration")));
    }

    while (match(TokenType::COMMA)) {
        add_child(node, make_token_node(previous()));
        add_child(node, parse_decl_identifier("variable"));

        if (match(TokenType::L_BRACKET)) {
            add_child(node, make_token_node(previous()));
            add_child(node, parse_array_size());
            add_child(node, make_token_node(expect(TokenType::R_BRACKET, "expected \"]\" in array declaration")));
        }
    }

    add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected \";\" after declaration")));
    return node;
}

CSTNode* Parser::parse_assignment_or_call_stmt() {
    CSTNode* node = make_node("assignment_or_call_stmt");

    if (check(TokenType::IDENTIFIER) || check(TokenType::PRINTF)) {
        add_child(node, make_token_node(advance()));
    } else {
        syntax_error(peek(), "expected identifier at start of assignment or call");
    }

    if (match(TokenType::L_BRACKET)) {
        add_child(node, make_token_node(previous()));
        add_child(node, parse_expression());
        add_child(node, make_token_node(expect(TokenType::R_BRACKET, "expected \"]\" after index expression")));
    }

    if (match(TokenType::ASSIGNMENT_OPERATOR)) {
        add_child(node, make_token_node(previous()));
        add_child(node, parse_expression());
        add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected \";\" after assignment")));
        return node;
    }

    if (match(TokenType::L_PAREN)) {
        add_child(node, make_token_node(previous()));

        if (!check(TokenType::R_PAREN)) {
            add_child(node, parse_expression());
            while (match(TokenType::COMMA)) {
                add_child(node, make_token_node(previous()));
                add_child(node, parse_expression());
            }
        }

        add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after argument list")));
        add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected \";\" after function/procedure call")));
        return node;
    }

    syntax_error(peek(), "expected assignment operator or call argument list");
}

CSTNode* Parser::parse_if_stmt() {
    CSTNode* node = make_node("if_stmt");

    add_child(node, make_token_node(expect(TokenType::IF, "expected \"if\"")));
    add_child(node, make_token_node(expect(TokenType::L_PAREN, "expected \"(\" after if")));
    add_child(node, parse_expression());
    add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after if condition")));
    add_child(node, parse_statement());

    if (match(TokenType::ELSE)) {
        add_child(node, make_token_node(previous()));
        add_child(node, parse_statement());
    }

    return node;
}

CSTNode* Parser::parse_while_stmt() {
    CSTNode* node = make_node("while_stmt");

    add_child(node, make_token_node(expect(TokenType::WHILE, "expected \"while\"")));
    add_child(node, make_token_node(expect(TokenType::L_PAREN, "expected \"(\" after while")));
    add_child(node, parse_expression());
    add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after while condition")));
    add_child(node, parse_statement());

    return node;
}

CSTNode* Parser::parse_for_stmt() {
    CSTNode* node = make_node("for_stmt");

    add_child(node, make_token_node(expect(TokenType::FOR, "expected \"for\"")));
    add_child(node, make_token_node(expect(TokenType::L_PAREN, "expected \"(\" after for")));

    if (!check(TokenType::SEMICOLON)) {
        add_child(node, parse_assignment_expr());
    }
    add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected first \";\" in for statement")));

    if (!check(TokenType::SEMICOLON)) {
        add_child(node, parse_expression());
    }
    add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected second \";\" in for statement")));

    if (!check(TokenType::R_PAREN)) {
        add_child(node, parse_assignment_expr());
    }
    add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after for clauses")));

    add_child(node, parse_statement());
    return node;
}

CSTNode* Parser::parse_return_stmt() {
    CSTNode* node = make_node("return_stmt");

    add_child(node, make_token_node(expect(TokenType::RETURN, "expected \"return\"")));

    if (!check(TokenType::SEMICOLON)) {
        add_child(node, parse_expression());
    }

    add_child(node, make_token_node(expect(TokenType::SEMICOLON, "expected \";\" after return statement")));
    return node;
}

CSTNode* Parser::parse_expression() {
    return parse_assignment_expr();
}

CSTNode* Parser::parse_assignment_expr() {
    CSTNode* left = parse_logical_or_expr();

    if (match(TokenType::ASSIGNMENT_OPERATOR)) {
        CSTNode* node = make_node("assignment_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_assignment_expr());
        return node;
    }

    return left;
}

CSTNode* Parser::parse_logical_or_expr() {
    CSTNode* left = parse_logical_and_expr();

    while (match(TokenType::BOOLEAN_OR)) {
        CSTNode* node = make_node("logical_or_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_logical_and_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_logical_and_expr() {
    CSTNode* left = parse_equality_expr();

    while (match(TokenType::BOOLEAN_AND)) {
        CSTNode* node = make_node("logical_and_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_equality_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_equality_expr() {
    CSTNode* left = parse_relational_expr();

    while (match_any({TokenType::BOOLEAN_EQUAL, TokenType::BOOLEAN_NOT_EQUAL})) {
        CSTNode* node = make_node("equality_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_relational_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_relational_expr() {
    CSTNode* left = parse_additive_expr();

    while (match_any({TokenType::LT, TokenType::LT_EQUAL, TokenType::GT, TokenType::GT_EQUAL})) {
        CSTNode* node = make_node("relational_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_additive_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_additive_expr() {
    CSTNode* left = parse_multiplicative_expr();

    while (match_any({TokenType::PLUS, TokenType::MINUS})) {
        CSTNode* node = make_node("additive_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_multiplicative_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_multiplicative_expr() {
    CSTNode* left = parse_unary_expr();

    while (match_any({TokenType::ASTERISK, TokenType::DIVIDE, TokenType::MODULO, TokenType::CARET})) {
        CSTNode* node = make_node("multiplicative_expr");
        add_child(node, left);
        add_child(node, make_token_node(previous()));
        add_child(node, parse_unary_expr());
        left = node;
    }

    return left;
}

CSTNode* Parser::parse_unary_expr() {
    if (match_any({TokenType::BOOLEAN_NOT, TokenType::PLUS, TokenType::MINUS})) {
        CSTNode* node = make_node("unary_expr");
        add_child(node, make_token_node(previous()));
        add_child(node, parse_unary_expr());
        return node;
    }

    return parse_primary_expr();
}

CSTNode* Parser::parse_primary_expr() {
    if (match(TokenType::INTEGER)) {
        CSTNode* node = make_node("integer_literal");
        add_child(node, make_token_node(previous()));
        return node;
    }

    if (match(TokenType::STRING_LITERAL)) {
        CSTNode* node = make_node("string_literal");
        add_child(node, make_token_node(previous()));
        return node;
    }

    if (match(TokenType::CHAR_LITERAL)) {
        CSTNode* node = make_node("char_literal");
        add_child(node, make_token_node(previous()));
        return node;
    }

    if (match(TokenType::TRUE_TOKEN) || match(TokenType::FALSE_TOKEN)) {
        CSTNode* node = make_node("boolean_literal");
        add_child(node, make_token_node(previous()));
        return node;
    }

    if (match(TokenType::IDENTIFIER) || match(TokenType::PRINTF)) {
        CSTNode* node = make_node("identifier_expr");
        add_child(node, make_token_node(previous()));

        if (match(TokenType::L_PAREN)) {
            add_child(node, make_token_node(previous()));

            if (!check(TokenType::R_PAREN)) {
                add_child(node, parse_expression());
                while (match(TokenType::COMMA)) {
                    add_child(node, make_token_node(previous()));
                    add_child(node, parse_expression());
                }
            }

            add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after argument list")));
        }

        if (match(TokenType::L_BRACKET)) {
            add_child(node, make_token_node(previous()));
            add_child(node, parse_expression());
            add_child(node, make_token_node(expect(TokenType::R_BRACKET, "expected \"]\" after index expression")));
        }

        return node;
    }

    if (match(TokenType::L_PAREN)) {
        CSTNode* node = make_node("grouped_expr");
        add_child(node, make_token_node(previous()));
        add_child(node, parse_expression());
        add_child(node, make_token_node(expect(TokenType::R_PAREN, "expected \")\" after grouped expression")));
        return node;
    }

    syntax_error(peek(), "expected primary expression");
}
