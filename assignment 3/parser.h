#pragma once

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

#include "tokens.h"
#include "cst.h"

class ParseError : public std::runtime_error {
public:
    ParseError(int line, const std::string& message);
    int line() const noexcept;
    const std::string& plain_message() const noexcept;

private:
    int line_;
    std::string plainMessage_;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    CSTNode* parse_program();

private:
    const std::vector<Token>& tokens_;
    std::size_t current_;

    const Token& peek() const;
    const Token& previous() const;
    bool is_at_end() const;

    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match_any(std::initializer_list<TokenType> types);

    const Token& advance();
    const Token& expect(TokenType type, const std::string& message);

    [[noreturn]] void syntax_error(const Token& tok, const std::string& message) const;

    bool is_type_specifier(TokenType type) const;
    bool is_statement_start(TokenType type) const;
    bool is_reserved_word_token(TokenType type) const;

    CSTNode* parse_top_level_decl();
    CSTNode* parse_function_decl();
    CSTNode* parse_procedure_decl();
    CSTNode* parse_parameter_list();
    CSTNode* parse_parameter();
    CSTNode* parse_type_specifier();
    CSTNode* parse_block();

    CSTNode* parse_statement();
    CSTNode* parse_declaration_stmt();
    CSTNode* parse_assignment_or_call_stmt();
    CSTNode* parse_if_stmt();
    CSTNode* parse_while_stmt();
    CSTNode* parse_for_stmt();
    CSTNode* parse_return_stmt();

    CSTNode* parse_expression();
    CSTNode* parse_assignment_expr();
    CSTNode* parse_logical_or_expr();
    CSTNode* parse_logical_and_expr();
    CSTNode* parse_equality_expr();
    CSTNode* parse_relational_expr();
    CSTNode* parse_additive_expr();
    CSTNode* parse_multiplicative_expr();
    CSTNode* parse_unary_expr();
    CSTNode* parse_primary_expr();

    CSTNode* parse_array_size();
    CSTNode* parse_decl_identifier(const std::string& role_name);
    CSTNode* make_token_node(const Token& tok) const;
};
