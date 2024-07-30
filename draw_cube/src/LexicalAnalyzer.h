#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <string>
#include <vector>
#include <regex>
#include <iostream>

enum TokenType { KEYWORD, IDENTIFIER, NUMBER, OPERATOR, SEPARATOR, UNKNOWN, FUNCTION };

struct Token {
    std::string lexeme;
    TokenType type;
};

class LexicalAnalyzer {
public:
    static std::vector<Token> analyze(const std::string& input) {
        std::vector<Token> tokens;
        std::regex token_regex(R"((new|array|insert|delete|\d+|=|;|\(|\)|\s+))");
        auto words_begin = std::sregex_iterator(input.begin(), input.end(), token_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::string part = (*i).str();
            TokenType type = UNKNOWN;
            if (part == "new") type = KEYWORD;
            else if (part == "array") type = IDENTIFIER;
            else if (part == "insert" || part == "delete") type = FUNCTION;
            else if (std::regex_match(part, std::regex(R"(\d+)"))) type = NUMBER;
            else if (part == "=") type = OPERATOR;
            else if (part == ";" || part == "(" || part == ")") type = SEPARATOR;
            else continue; // Skip whitespace
            tokens.push_back({ part, type });
        }

        std::cout << "Generated Tokens: " << std::endl;
        for (const auto& token : tokens) {
            std::cout << "{ lexeme: \"" << token.lexeme << "\", type: " << token.type << " }" << std::endl;
        }

        return tokens;
    }
};

#endif // LEXICAL_ANALYZER_H
