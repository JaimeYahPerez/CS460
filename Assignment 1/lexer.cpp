#include "lexer.h"
#include <cctype>

namespace {

    // Helpers (ASCII assumptions are fine for this class)
    bool is_letter(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }
    bool is_digit(char c) { return (c >= '0' && c <= '9'); }

    // From token-BNF intent: LETTER_UNDERSCORE and LETTER_DIGIT_UNDERSCORE
    bool is_ident_start(char c) { return is_letter(c) || c == '_'; }
    bool is_ident_tail(char c) { return is_letter(c) || is_digit(c) || c == '_'; }

    bool is_hex_digit(char c) {
        return is_digit(c) ||
            (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f');
    }

    // Escape set per <ESCAPED_CHARACTER> ::= \a \b \f \n \r \t \v \\ \? \' \" \xH \xHH :contentReference[oaicite:8]{index=8}
    bool consume_escape(const std::string& s, size_t& i) {
        // i is at '\' already; returns true if a valid escape was consumed.
        if (i >= s.size() || s[i] != '\\') return false;
        if (i + 1 >= s.size()) return false;

        char n = s[i + 1];
        switch (n) {
        case 'a': case 'b': case 'f': case 'n': case 'r':
        case 't': case 'v': case '\\': case '?': case '\'': case '"':
            i += 2;
            return true;
        case 'x': {
            // \xH or \xHH
            if (i + 2 >= s.size()) return false;
            if (!is_hex_digit(s[i + 2])) return false;
            if (i + 3 < s.size() && is_hex_digit(s[i + 3])) {
                i += 4;
            }
            else {
                i += 3;
            }
            return true;
        }
        default:
            return false;
        }
    }

} // namespace

std::vector<Token> tokenize(const std::string& input, std::string& errorMsg) {
    errorMsg.clear();
    std::vector<Token> out;

    enum class State { START, IN_IDENT, IN_INT } st = State::START;

    std::string buf;
    int line = 1;
    int token_start_line = 1;

    auto emit = [&](TokenType type, const std::string& lex, int ln) {
        out.push_back(Token{ type, lex, ln });
        };

    auto fail = [&](int ln, const std::string& msg) {
        errorMsg = "Syntax error on line " + std::to_string(ln) + ": " + msg;
        out.clear();
        };

    auto at = [&](size_t i) -> char { return input[i]; };

    for (size_t i = 0; i < input.size(); /* manual increment */) {
        char c = at(i);

        switch (st) {
        case State::START: {
            // Skip whitespace
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (c == '\n') line++;
                i++;
                break;
            }

            token_start_line = line;

            // Multi-char operators first (longest match)
            if (c == '<' && i + 1 < input.size() && at(i + 1) == '=') { emit(TokenType::LT_EQUAL, "<=", line); i += 2; break; }
            if (c == '>' && i + 1 < input.size() && at(i + 1) == '=') { emit(TokenType::GT_EQUAL, ">=", line); i += 2; break; }
            if (c == '&' && i + 1 < input.size() && at(i + 1) == '&') { emit(TokenType::BOOLEAN_AND, "&&", line); i += 2; break; }
            if (c == '|' && i + 1 < input.size() && at(i + 1) == '|') { emit(TokenType::BOOLEAN_OR, "||", line); i += 2; break; }
            if (c == '=' && i + 1 < input.size() && at(i + 1) == '=') { emit(TokenType::BOOLEAN_EQUAL, "==", line); i += 2; break; }
            if (c == '!' && i + 1 < input.size() && at(i + 1) == '=') { emit(TokenType::BOOLEAN_NOT_EQUAL, "!=", line); i += 2; break; }

            // Quoted strings: emit quote token, then STRING token, then quote token
            if (c == '"') {
                emit(TokenType::DOUBLE_QUOTE, "\"", line);
                i++; // consume opening quote

                std::string content;

                while (i < input.size()) {
                    char d = at(i);
                    if (d == '\n') line++; // strings may include newline bytes; still count them
                    if (d == '"') break;

                    if (d == '\\') {
                        size_t old = i;
                        if (!consume_escape(input, i)) {
                            fail(token_start_line, "invalid escaped character");
                            return out;
                        }
                        content.append(input.substr(old, i - old));
                    }
                    else {
                        content.push_back(d);
                        i++;
                    }
                }

                if (i >= input.size()) {
                    fail(token_start_line, "unterminated double-quoted string");
                    return out;
                }

                emit(TokenType::STRING, content, token_start_line);
                emit(TokenType::DOUBLE_QUOTE, "\"", line); // current line at closing quote
                i++; // consume closing quote
                break;
            }

            if (c == '\'') {
                emit(TokenType::SINGLE_QUOTE, "\'", line);
                i++; // opening

                std::string content;
                while (i < input.size()) {
                    char d = at(i);
                    if (d == '\n') line++;
                    if (d == '\'') break;

                    if (d == '\\') {
                        size_t old = i;
                        if (!consume_escape(input, i)) {
                            fail(token_start_line, "invalid escaped character");
                            return out;
                        }
                        content.append(input.substr(old, i - old));
                    }
                    else {
                        content.push_back(d);
                        i++;
                    }
                }

                if (i >= input.size()) {
                    fail(token_start_line, "unterminated single-quoted string");
                    return out;
                }

                emit(TokenType::STRING, content, token_start_line);
                emit(TokenType::SINGLE_QUOTE, "\'", line);
                i++; // closing
                break;
            }

            // Identifiers
            if (is_ident_start(c)) {
                buf.clear();
                buf.push_back(c);
                st = State::IN_IDENT;
                i++;
                break;
            }

            // Integers: digit, or (+|-) followed by digit
            if (is_digit(c) ||
                ((c == '+' || c == '-') && i + 1 < input.size() && is_digit(at(i + 1)))) {
                buf.clear();
                buf.push_back(c);
                st = State::IN_INT;
                i++;
                break;
            }

            // Single-char punctuation / operators
            switch (c) {
            case '(': emit(TokenType::L_PAREN, "(", line); i++; break;
            case ')': emit(TokenType::R_PAREN, ")", line); i++; break;
            case '[': emit(TokenType::L_BRACKET, "[", line); i++; break;
            case ']': emit(TokenType::R_BRACKET, "]", line); i++; break;
            case '{': emit(TokenType::L_BRACE, "{", line); i++; break;
            case '}': emit(TokenType::R_BRACE, "}", line); i++; break;
            case ';': emit(TokenType::SEMICOLON, ";", line); i++; break;
            case ',': emit(TokenType::COMMA, ",", line); i++; break;

            case '=': emit(TokenType::ASSIGNMENT_OPERATOR, "=", line); i++; break;
            case '+': emit(TokenType::PLUS, "+", line); i++; break;
            case '-': emit(TokenType::MINUS, "-", line); i++; break;
            case '*': emit(TokenType::ASTERISK, "*", line); i++; break;
            case '/': emit(TokenType::DIVIDE, "/", line); i++; break;
            case '%': emit(TokenType::MODULO, "%", line); i++; break;
            case '^': emit(TokenType::CARET, "^", line); i++; break;
            case '<': emit(TokenType::LT, "<", line); i++; break;
            case '>': emit(TokenType::GT, ">", line); i++; break;
            case '!': emit(TokenType::BOOLEAN_NOT, "!", line); i++; break;

            default:
                fail(line, std::string("unexpected character '") + c + "'");
                return out;
            }
            break;
        }

        case State::IN_IDENT: {
            if (i < input.size() && is_ident_tail(c)) {
                buf.push_back(c);
                i++;
            }
            else {
                emit(TokenType::IDENTIFIER, buf, token_start_line);
                st = State::START;
            }
            break;
        }

        case State::IN_INT: {
            if (i < input.size() && is_digit(c)) {
                buf.push_back(c);
                i++;
                break;
            }

            // If an integer is immediately followed by letter/_ => invalid integer (PA2 files 5/6 case)
            if (i < input.size() && is_ident_start(c)) {
                // consume the “bad tail” so we can account for all bytes (optional but nice)
                while (i < input.size()) {
                    char d = at(i);
                    if (d == '\n') break;
                    if (d == ' ' || d == '\t' || d == '\r') break;

                    // also stop on punctuation/operators that normally delimit tokens
                    if (d == '(' || d == ')' || d == '{' || d == '}' || d == '[' || d == ']' ||
                        d == ';' || d == ',' || d == '<' || d == '>' || d == '=' || d == '!' ||
                        d == '+' || d == '-' || d == '*' || d == '/' || d == '%' || d == '^' ||
                        d == '&' || d == '|') {
                        break;
                    }
                    i++;
                }
                fail(token_start_line, "invalid integer");
                return out;
            }

            emit(TokenType::INTEGER, buf, token_start_line);
            st = State::START;
            break;
        }
        }

    }

    // Flush any buffered token
    if (st == State::IN_IDENT) emit(TokenType::IDENTIFIER, buf, token_start_line);
    if (st == State::IN_INT) emit(TokenType::INTEGER, buf, token_start_line);

    emit(TokenType::END_OF_FILE, "", line);
    return out;
}