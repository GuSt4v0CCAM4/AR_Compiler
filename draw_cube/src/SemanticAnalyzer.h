#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "LexicalAnalyzer.h"
#include <iostream>
#include <vector>

class SemanticAnalyzer {
public:
    static bool analyze(const std::vector<Token>& tokens) {
        if (tokens.size() == 5 &&
            tokens[0].lexeme == "new" &&
            tokens[1].lexeme == "array" &&
            tokens[2].lexeme == "=" &&
            tokens[3].type == NUMBER &&
            tokens[4].lexeme == ";") {
            return true;
            } else if (tokens.size() == 7 &&
                       tokens[0].lexeme == "insert" &&
                       tokens[1].lexeme == "(" &&
                       tokens[2].type == NUMBER &&
                       tokens[3].lexeme == ")" &&
                       tokens[4].lexeme == "=" &&
                       tokens[5].type == NUMBER &&
                       tokens[6].lexeme == ";") {
                return true;
                       } else if (tokens.size() == 5 &&
                                  tokens[0].lexeme == "delete" &&
                                  tokens[1].lexeme == "(" &&
                                  tokens[2].type == NUMBER &&
                                  tokens[3].lexeme == ")" &&
                                  tokens[4].lexeme == ";") {
                           return true;
                                  }
        std::cerr << "Failed semantic analysis." << std::endl;
        return false;
    }
};

#endif // SEMANTIC_ANALYZER_H
