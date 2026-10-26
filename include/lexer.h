
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "token.h"

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;

    std::unordered_map<std::string, TokenType> keywords;

    char currentChar() const;
    char peekChar(size_t offset = 1) const;
    void advance();

    void skipWhitespace();
    void skipSingleLineComment();
    void skipMultiLineComment();

    Token readIdentifier();
    Token readNumber();
    Token readString();
    Token readCharacter();
    Token readOperator();

    Token makeToken(TokenType type, const std::string& value,
                    int startLine, int startColumn) const;

    void initializeKeywords();

public:
    explicit Lexer(const std::string& sourceCode);

    Token nextToken();

    std::vector<Token> tokenize();
};

#endif
