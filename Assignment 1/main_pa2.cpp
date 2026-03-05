#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "comment_stripper.h"
#include "lexer.h"
#include "tokens.h"

static void print_tokens(const std::vector<Token>& toks) {
    std::cout << "Token list:\n\n";
    for (const auto& t : toks) {
        if (t.type == TokenType::END_OF_FILE) break;
        std::cout << "Token type: " << token_type_name(t.type) << "\n";
        std::cout << "Token:      " << t.lexeme << "\n\n";
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: assignment2 <input_file>\n";
        return 1;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::cerr << "ERROR: Cannot open input file\n";
        return 1;
    }

    // 1) Strip comments
    std::ostringstream cleaned_out;
    StripResult sr = strip_comments(in, cleaned_out);
    if (!sr.ok) {
        std::cerr << sr.error_message << "\n";
        return 1;
    }
    std::string cleaned = cleaned_out.str();

    // 2) Tokenize
    std::string err;
    std::vector<Token> toks = tokenize(cleaned, err);
    if (!err.empty()) {
        std::cerr << err << "\n";
        return 1;
    }

    // 3) Print tokens
    print_tokens(toks);
    return 0;
}