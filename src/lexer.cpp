
#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <string>

Lexer::Lexer(const std::string& sourceCode)
    : source(sourceCode), position(0), line(1), column(1) {
    initializeKeywords();
}

void Lexer::initializeKeywords() {
    keywords = {
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"char", TokenType::CHAR},
        {"void", TokenType::VOID},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"do", TokenType::DO},
        {"return", TokenType::RETURN},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE}
    };
}

char Lexer::currentChar() const {
    if (position >= source.size()) {
        return '\0';
    }
    return source[position];
}

char Lexer::peekChar(size_t offset) const {
    if (offset >= source.size() - position) {
        return '\0';
    }
    return source[position + offset];
}

void Lexer::advance() {
    if (position >= source.size()) {
        return;
    }

    if (source[position] == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    position++;
}

void Lexer::skipWhitespace() {
    while (position < source.size() &&
           std::isspace(static_cast<unsigned char>(currentChar()))) {
        advance();
    }
}

void Lexer::skipSingleLineComment() {
    advance();
    advance();

    while (position < source.size() && currentChar() != '\n') {
        advance();
    }
}

void Lexer::skipMultiLineComment() {
    int startLine = line;
    int startColumn = column;

    advance();
    advance();

    while (position < source.size()) {
        if (currentChar() == '*' && peekChar() == '/') {
            advance();
            advance();
            return;
        }
        advance();
    }

    throw std::runtime_error(
        "Unterminated block comment at line " +
        std::to_string(startLine) + ", column " +
        std::to_string(startColumn)
    );
}

Token Lexer::makeToken(TokenType type, const std::string& value,
                       int startLine, int startColumn) const {
    return Token(type, value, startLine, startColumn);
}

Token Lexer::readIdentifier() {
    int startLine = line;
    int startColumn = column;
    size_t startPosition = position;

    while (position < source.size()) {
        unsigned char ch = static_cast<unsigned char>(currentChar());

        if (std::isalnum(ch) || ch == '_') {
            advance();
        } else {
            break;
        }
    }

    std::string value = source.substr(
        startPosition, position - startPosition
    );

    auto it = keywords.find(value);

    if (it != keywords.end()) {
        return makeToken(it->second, value, startLine, startColumn);
    }

    return makeToken(
        TokenType::IDENTIFIER, value, startLine, startColumn
    );
}

Token Lexer::readNumber() {
    int startLine = line;
    int startColumn = column;
    size_t startPosition = position;
    bool isFloat = false;

    while (position < source.size() &&
           std::isdigit(static_cast<unsigned char>(currentChar()))) {
        advance();
    }

    if (currentChar() == '.' &&
        std::isdigit(static_cast<unsigned char>(peekChar()))) {
        isFloat = true;
        advance();

        while (position < source.size() &&
               std::isdigit(static_cast<unsigned char>(currentChar()))) {
            advance();
        }
    }

    if (currentChar() == 'e' || currentChar() == 'E') {
        char next = peekChar();

        if (std::isdigit(static_cast<unsigned char>(next)) ||
            ((next == '+' || next == '-') &&
             std::isdigit(static_cast<unsigned char>(peekChar(2))))) {
            isFloat = true;
            advance();

            if (currentChar() == '+' || currentChar() == '-') {
                advance();
            }

            while (position < source.size() &&
                   std::isdigit(static_cast<unsigned char>(currentChar()))) {
                advance();
            }
        }
    }

    std::string value = source.substr(
        startPosition, position - startPosition
    );

    return makeToken(
        isFloat ? TokenType::FLOAT_LITERAL
                : TokenType::INTEGER_LITERAL,
        value, startLine, startColumn
    );
}

Token Lexer::readString() {
    int startLine = line;
    int startColumn = column;
    size_t startPosition = position;

    advance();

    while (position < source.size()) {
        if (currentChar() == '"') {
            advance();

            return makeToken(
                TokenType::STRING_LITERAL,
                source.substr(startPosition, position - startPosition),
                startLine, startColumn
            );
        }

        if (currentChar() == '\n') {
            break;
        }

        if (currentChar() == '\\') {
            advance();

            if (position >= source.size() || currentChar() == '\n') {
                break;
            }
        }

        advance();
    }

    throw std::runtime_error(
        "Unterminated string literal at line " +
        std::to_string(startLine) + ", column " +
        std::to_string(startColumn)
    );
}

Token Lexer::readCharacter() {
    int startLine = line;
    int startColumn = column;
    size_t startPosition = position;

    advance();

    while (position < source.size()) {
        if (currentChar() == '\'') {
            advance();

            return makeToken(
                TokenType::CHAR_LITERAL,
                source.substr(startPosition, position - startPosition),
                startLine, startColumn
            );
        }

        if (currentChar() == '\n') {
            break;
        }

        if (currentChar() == '\\') {
            advance();

            if (position >= source.size() || currentChar() == '\n') {
                break;
            }
        }

        advance();
    }

    throw std::runtime_error(
        "Unterminated character literal at line " +
        std::to_string(startLine) + ", column " +
        std::to_string(startColumn)
    );
}

Token Lexer::readOperator() {
    int startLine = line;
    int startColumn = column;
    char ch = currentChar();
    char next = peekChar();

    TokenType type = TokenType::UNKNOWN;
    std::string value(1, ch);

    if (ch == '+' && next == '+') {
        type = TokenType::INCREMENT;
    } else if (ch == '-' && next == '-') {
        type = TokenType::DECREMENT;
    } else if (ch == '+' && next == '=') {
        type = TokenType::PLUS_ASSIGN;
    } else if (ch == '-' && next == '=') {
        type = TokenType::MINUS_ASSIGN;
    } else if (ch == '*' && next == '=') {
        type = TokenType::MULTIPLY_ASSIGN;
    } else if (ch == '/' && next == '=') {
        type = TokenType::DIVIDE_ASSIGN;
    } else if (ch == '%' && next == '=') {
        type = TokenType::MODULO_ASSIGN;
    } else if (ch == '=' && next == '=') {
        type = TokenType::EQUAL;
    } else if (ch == '!' && next == '=') {
        type = TokenType::NOT_EQUAL;
    } else if (ch == '<' && next == '=') {
        type = TokenType::LESS_EQUAL;
    } else if (ch == '>' && next == '=') {
        type = TokenType::GREATER_EQUAL;
    } else if (ch == '&' && next == '&') {
        type = TokenType::LOGICAL_AND;
    } else if (ch == '|' && next == '|') {
        type = TokenType::LOGICAL_OR;
    } else if (ch == '<' && next == '<') {
        type = TokenType::LEFT_SHIFT;
    } else if (ch == '>' && next == '>') {
        type = TokenType::RIGHT_SHIFT;
    }

    if (type != TokenType::UNKNOWN) {
        value.push_back(next);
        advance();
        advance();
        return makeToken(type, value, startLine, startColumn);
    }

    switch (ch) {
        case '+': type = TokenType::PLUS; break;
        case '-': type = TokenType::MINUS; break;
        case '*': type = TokenType::MULTIPLY; break;
        case '/': type = TokenType::DIVIDE; break;
        case '%': type = TokenType::MODULO; break;

        case '=': type = TokenType::ASSIGN; break;
        case '<': type = TokenType::LESS; break;
        case '>': type = TokenType::GREATER; break;
        case '!': type = TokenType::LOGICAL_NOT; break;

        case '&': type = TokenType::BITWISE_AND; break;
        case '|': type = TokenType::BITWISE_OR; break;
        case '^': type = TokenType::BITWISE_XOR; break;
        case '~': type = TokenType::BITWISE_NOT; break;

        case '(': type = TokenType::LEFT_PAREN; break;
        case ')': type = TokenType::RIGHT_PAREN; break;
        case '{': type = TokenType::LEFT_BRACE; break;
        case '}': type = TokenType::RIGHT_BRACE; break;
        case '[': type = TokenType::LEFT_BRACKET; break;
        case ']': type = TokenType::RIGHT_BRACKET; break;

        case ';': type = TokenType::SEMICOLON; break;
        case ',': type = TokenType::COMMA; break;
        case ':': type = TokenType::COLON; break;
        case '.': type = TokenType::DOT; break;
        case '?': type = TokenType::QUESTION; break;

        default: type = TokenType::UNKNOWN; break;
    }

    advance();

    return makeToken(type, value, startLine, startColumn);
}

Token Lexer::nextToken() {
    while (position < source.size()) {
        skipWhitespace();

        if (position >= source.size()) {
            break;
        }

        if (currentChar() == '/' && peekChar() == '/') {
            skipSingleLineComment();
            continue;
        }

        if (currentChar() == '/' && peekChar() == '*') {
            skipMultiLineComment();
            continue;
        }

        unsigned char ch = static_cast<unsigned char>(currentChar());

        if (std::isalpha(ch) || ch == '_') {
            return readIdentifier();
        }

        if (std::isdigit(ch)) {
            return readNumber();
        }

        if (currentChar() == '"') {
            return readString();
        }

        if (currentChar() == '\'') {
            return readCharacter();
        }

        return readOperator();
    }

    return makeToken(TokenType::END_OF_FILE, "",
                     line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token = nextToken();

        if (token.type == TokenType::UNKNOWN) {
            throw std::runtime_error(
                "Unknown token '" + token.value +
                "' at line " + std::to_string(token.line) +
                ", column " + std::to_string(token.column)
            );
        }

        tokens.push_back(token);

        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return tokens;
}
