#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "comment_stripper.h"
#include "lexer.h"
#include "parser.h"
#include "cst.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: assignment3 <input_file>\n";
        return 1;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::cout << "ERROR: Cannot open input file\n";
        return 1;
    }

    std::ostringstream cleaned_out;
    StripResult sr = strip_comments(in, cleaned_out);
    if (!sr.ok) {
        std::cout << sr.error_message << "\n";
        return 1;
    }

    std::string cleaned = cleaned_out.str();

    std::string err;
    std::vector<Token> toks = tokenize(cleaned, err);
    if (!err.empty()) {
        std::cout << err << "\n";
        return 1;
    }

    try {
        Parser parser(toks);
        CSTNode* root = parser.parse_program();
        print_cst_surface(root, std::cout);
        delete_cst(root);
    }
    catch (const ParseError& e) {
        std::cout << e.what() << "\n";
        return 1;
    }

    return 0;
}