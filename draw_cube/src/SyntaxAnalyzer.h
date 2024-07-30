#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

#include "LexicalAnalyzer.h"
#include <iostream>
#include <vector>

class SyntaxAnalyzer {
public:
    static bool parse(const std::vector<Token>& tokens) {
        std::cout << "Syntax Analyzer Tokens: " << std::endl;
        for (const auto& token : tokens) {
            std::cout << "{ lexeme: \"" << token.lexeme << "\", type: " << token.type << " }" << std::endl;
        }

        if (tokens.size() == 5 &&
            tokens[0].type == KEYWORD &&
            tokens[1].type == IDENTIFIER &&
            tokens[2].type == OPERATOR &&
            tokens[3].type == NUMBER &&
            tokens[4].type == SEPARATOR) {
            return true;
            } else if (tokens.size() == 7 &&
                       tokens[0].type == FUNCTION &&
                       tokens[1].type == SEPARATOR &&
                       tokens[2].type == NUMBER &&
                       tokens[3].type == SEPARATOR &&
                       tokens[4].type == OPERATOR &&
                       tokens[5].type == NUMBER &&
                       tokens[6].type == SEPARATOR) {
                return true;
                       } else if (tokens.size() == 5 &&
                                  tokens[0].type == FUNCTION &&
                                  tokens[1].type == SEPARATOR &&
                                  tokens[2].type == NUMBER &&
                                  tokens[3].type == SEPARATOR &&
                                  tokens[4].type == SEPARATOR) {
                           return true;
                                  }

        std::cerr << "Failed due to incorrect number of tokens or unexpected tokens." << std::endl;
        return false;
    }
};

#endif // SYNTAX_ANALYZER_H
