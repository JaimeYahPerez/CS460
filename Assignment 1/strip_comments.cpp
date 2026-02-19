#include <iostream>
#include <fstream>
#include <string>

enum class State {
    NORMAL,
    SLASH_MODE,
    LINE_COMMENT,
    BLOCK_COMMENT,
    BLOCK_STAR,
    DQUOTE,
    DQUOTE_ESC,
    SQUOTE,
    SQUOTE_ESC,
    SAW_STAR
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: strip_comments <input_file>\n";
        std::exit(1);
    }

    std::ifstream in(argv[1]);

    if (!in) {
        std::cerr << "Error: Cannot open input file\n";
        return 1;
    }

    State st = State::NORMAL;

    int line = 1;
    int blockStartLine = -1;

    auto put_space_or_newline = [&](char c) {
        if (c == '\n')
            std::cout.put('\n');
        else 
            std::cout.put(' ');
        };

    int input;
    while ((input = in.get()) != EOF) {
        char c = static_cast<char>(input);

        switch (st) {
        case State::NORMAL:
            if (c == '/') {
                st = State::SLASH_MODE;
            }
            else if (c == '"') {
                std::cout.put(c);
                st = State::DQUOTE;
            }
            else if (c == '\'') {
                std::cout.put(c);
                st = State::SQUOTE;
            }
            else if (c == '*') {
                st = State::SAW_STAR;
            }
            else {
                std::cout.put(c);
            }
            break;

        case State::SLASH_MODE:
            if (c == '/') {
                std::cout.put(' ');
                std::cout.put(' ');
                st = State::LINE_COMMENT;
            }
            else if (c == '*') {
                std::cout.put(' ');
                std::cout.put(' ');
                st = State::BLOCK_COMMENT;
                blockStartLine = line;
            }
            else {
                std::cout.put('/');
                if (c == '"') {
                    std::cout.put(c);
                    st = State::DQUOTE;
                }
                else if (c == '\'') {
                    std::cout.put(c);
                    st = State::SQUOTE;
                }
                else if (c == '/') {
                    st = State::SLASH_MODE;
                }
                else {
                    std::cout.put(c);
                    st = State::NORMAL;
                }
            }
            break;

        case State::LINE_COMMENT:
            if (c == '\n') {
                std::cout.put('\n');
                st = State::NORMAL;
            }
            else {
                std::cout.put(' ');
            }
            break;

        case State::BLOCK_COMMENT:
            if (c == '*') {
                std::cout.put(' ');
                st = State::BLOCK_STAR;
            }
            else {
                put_space_or_newline(c);
            }
            break;

        case State::BLOCK_STAR:
            if (c == '/') {
                std::cout.put(' ');
                st = State::NORMAL;
            }
            else if (c == '*') {
                std::cout.put(' ');
                st = State::BLOCK_STAR;
            }
            else {
                put_space_or_newline(c);
                st = State::BLOCK_COMMENT;
            }
            break;

        case State::DQUOTE:
            std::cout.put(c);
            if (c == '\\') 
                st = State::DQUOTE_ESC;
            else if (c == '"') 
                st = State::NORMAL;
            break;

        case State::DQUOTE_ESC:
            std::cout.put(c);
            st = State::DQUOTE;
            break;

        case State::SQUOTE:
            std::cout.put(c);
            if (c == '\\') 
                st = State::SQUOTE_ESC;
            else if (c == '\'') 
                st = State::NORMAL;
            break;

        case State::SQUOTE_ESC:
            std::cout.put(c);
            st = State::SQUOTE;
            break;

        case State::SAW_STAR:
            if (c == '/') {
                std::cerr << "WARNING: Program contains stray C-style comment terminator on line " << line << "\n";
                std::cout.put(' ');
                std::cout.put(' ');
                st = State::NORMAL;
            }
            else {
                std::cout.put('*');
                if (c == '/') st = State::SLASH_MODE;
                else if (c == '*') st = State::SAW_STAR;
                else if (c == '"') { std::cout.put(c); st = State::DQUOTE; }
                else if (c == '\'') { std::cout.put(c); st = State::SQUOTE; }
                else { std::cout.put(c); st = State::NORMAL; }
            }
        }

        if (c == '\n') { 
            line++; 
        }

    }
    if(st == State::SLASH_MODE) {
        std::cout.put('/');
        st = State::NORMAL;
    }

    if (st == State::BLOCK_COMMENT || st == State::BLOCK_STAR) {
        std::cerr << "ERROR: Program contains C-style, unterminated comment on line "
            << blockStartLine << "\n";
        return 1;
    }

    return 0;
   }

