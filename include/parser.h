
#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include <string>
#include <cstddef>

#include "token.h"
#include "ast.h"

class Parser {
private:
    std::vector<Token> tokens;
    std::size_t position;

    // Token navigation
    const Token& currentToken() const;
    const Token& previousToken() const;
    const Token& peekToken(std::size_t offset = 1) const;

    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    void advance();

    // Error reporting
    void reportError(const std::string& message) const;

    // Types and functions
    bool isTypeSpecifier(TokenType type) const;
    std::string parseType();
    std::unique_ptr<FunctionDeclaration> parseFunction();
    std::vector<Parameter> parseParameters();

    // Statements
    std::unique_ptr<BlockStatement> parseBlock();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseVariableDeclaration();
    std::unique_ptr<Statement> parseAssignment();
    std::unique_ptr<Statement> parseReturnStatement();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseForStatement();
    std::unique_ptr<Statement> parseExpressionStatement();

    // Expression parsing, from lowest to highest precedence
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseRelational();
    std::unique_ptr<Expression> parseAdditive();
    std::unique_ptr<Expression> parseMultiplicative();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parsePrimary();

public:
    explicit Parser(const std::vector<Token>& tokenList);

    // Parse the complete source program
    std::unique_ptr<Program> parse();
};

#endif
