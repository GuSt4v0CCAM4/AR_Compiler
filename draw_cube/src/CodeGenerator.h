#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "LexicalAnalyzer.h"
#include <iostream>
#include <vector>
#include <unordered_map>

class CodeGenerator {
    static std::unordered_map<int, int> array;
public:
    static void generate(const std::vector<Token>& tokens) {
        if (tokens[0].lexeme == "new" && tokens[1].lexeme == "array") {
            int arraySize = std::stoi(tokens[3].lexeme);
            array.clear();
            for (int i = 0; i < arraySize; ++i) {
                array[i] = 0;
            }
            std::cout << "Array of size " << arraySize << " created." << std::endl;
            displayArray();
        } else if (tokens[0].lexeme == "insert") {
            int index = std::stoi(tokens[2].lexeme);
            int value = std::stoi(tokens[5].lexeme);
            array[index] = value;
            std::cout << "Inserted " << value << " at position " << index << "." << std::endl;
            displayArray();
        } else if (tokens[0].lexeme == "delete") {
            int index = std::stoi(tokens[2].lexeme);
            array.erase(index);
            std::cout << "Deleted element at position " << index << "." << std::endl;
            displayArray();
        }
    }

    static void displayArray() {
        std::cout << "Current array state:" << std::endl;
        for (const auto& element : array) {
            std::cout << "Index " << element.first << ": " << element.second << std::endl;
        }
    }
};

std::unordered_map<int, int> CodeGenerator::array;

#endif // CODE_GENERATOR_H
