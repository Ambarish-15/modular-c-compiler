
#include "parser.h"

#include <stdexcept>
#include <utility>

using namespace std;

// Constructor
Parser::Parser(const vector<Token>& tokenList)
    : tokens(tokenList), position(0) {

    if (tokens.empty() ||
        tokens.back().type != TokenType::END_OF_FILE) {
        throw runtime_error(
            "Parser requires tokens ending with EOF"
        );
    }
}

// Token navigation
const Token& Parser::currentToken() const {
    return tokens[position];
}

const Token& Parser::previousToken() const {
    return tokens[position == 0 ? 0 : position - 1];
}

const Token& Parser::peekToken(size_t offset) const {
    if (offset >= tokens.size() - position) {
        return tokens.back();
    }

    return tokens[position + offset];
}

bool Parser::isAtEnd() const {
    return currentToken().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    return currentToken().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(
    TokenType type,
    const string& message
) {
    if (!check(type)) {
        reportError(message);
    }

    Token token = currentToken();
    advance();
    return token;
}

void Parser::advance() {
    if (!isAtEnd()) {
        position++;
    }
}

// Error reporting
void Parser::reportError(const string& message) const {
    const Token& token = currentToken();

    throw runtime_error(
        "Syntax error at line " +
        to_string(token.line) +
        ", column " +
        to_string(token.column) +
        ": " + message +
        " (found '" + token.value + "')"
    );
}

// Check whether token represents a type
bool Parser::isTypeSpecifier(TokenType type) const {
    return type == TokenType::INT ||
           type == TokenType::FLOAT ||
           type == TokenType::CHAR ||
           type == TokenType::VOID;
}

string Parser::parseType() {
    if (!isTypeSpecifier(currentToken().type)) {
        reportError("Expected a data type");
    }

    string type = currentToken().value;
    advance();
    return type;
}

// Parse complete program
unique_ptr<Program> Parser::parse() {
    auto program = make_unique<Program>();

    while (!isAtEnd()) {
        program->addFunction(parseFunction());
    }

    return program;
}

// Parse function definition
unique_ptr<FunctionDeclaration> Parser::parseFunction() {
    string returnType = parseType();

    string name = consume(
        TokenType::IDENTIFIER,
        "Expected function name"
    ).value;

    consume(
        TokenType::LEFT_PAREN,
        "Expected '(' after function name"
    );

    vector<Parameter> parameters = parseParameters();

    consume(
        TokenType::RIGHT_PAREN,
        "Expected ')' after parameters"
    );

    auto body = parseBlock();

    return make_unique<FunctionDeclaration>(
        returnType,
        name,
        move(parameters),
        move(body)
    );
}

// Parse function parameters
vector<Parameter> Parser::parseParameters() {
    vector<Parameter> parameters;

    if (check(TokenType::RIGHT_PAREN)) {
        return parameters;
    }

    do {
        string type = parseType();

        if (type == "void") {
            reportError("Void parameter must be the only parameter");
        }

        string name = consume(
            TokenType::IDENTIFIER,
            "Expected parameter name"
        ).value;

        parameters.emplace_back(type, name);

    } while (match(TokenType::COMMA));

    return parameters;
}

// Parse block: { statements }
unique_ptr<BlockStatement> Parser::parseBlock() {
    consume(
        TokenType::LEFT_BRACE,
        "Expected '{' to start block"
    );

    auto block = make_unique<BlockStatement>();

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        block->addStatement(parseStatement());
    }

    consume(
        TokenType::RIGHT_BRACE,
        "Expected '}' to close block"
    );

    return block;
}

// Parse a statement
unique_ptr<Statement> Parser::parseStatement() {
    if (check(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }

    if (isTypeSpecifier(currentToken().type)) {
        return parseVariableDeclaration();
    }

    if (check(TokenType::RETURN)) {
        return parseReturnStatement();
    }

    if (check(TokenType::IF)) {
        return parseIfStatement();
    }

    if (check(TokenType::WHILE)) {
        return parseWhileStatement();
    }

    if (check(TokenType::FOR)) {
        return parseForStatement();
    }

    if (check(TokenType::IDENTIFIER) &&
        peekToken().type == TokenType::ASSIGN) {
        return parseAssignment();
    }

    return parseExpressionStatement();
}

// Parse variable declaration
unique_ptr<Statement> Parser::parseVariableDeclaration() {
    string type = parseType();

    if (type == "void") {
        reportError("Variable cannot have type void");
    }

    string name = consume(
        TokenType::IDENTIFIER,
        "Expected variable name"
    ).value;

    unique_ptr<Expression> initializer = nullptr;

    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after variable declaration"
    );

    return make_unique<VariableDeclaration>(
        type,
        name,
        move(initializer)
    );
}

// Parse assignment: a = expression;
unique_ptr<Statement> Parser::parseAssignment() {
    string name = consume(
        TokenType::IDENTIFIER,
        "Expected variable name"
    ).value;

    consume(
        TokenType::ASSIGN,
        "Expected '=' in assignment"
    );

    auto value = parseExpression();

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after assignment"
    );

    return make_unique<AssignmentStatement>(
        name,
        move(value)
    );
}

// Parse return statement
unique_ptr<Statement> Parser::parseReturnStatement() {
    consume(
        TokenType::RETURN,
        "Expected 'return'"
    );

    unique_ptr<Expression> value = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after return statement"
    );

    return make_unique<ReturnStatement>(move(value));
}

// Parse if-else statement
unique_ptr<Statement> Parser::parseIfStatement() {
    consume(TokenType::IF, "Expected 'if'");

    consume(
        TokenType::LEFT_PAREN,
        "Expected '(' after if"
    );

    auto condition = parseExpression();

    consume(
        TokenType::RIGHT_PAREN,
        "Expected ')' after condition"
    );

    auto thenBranch = parseStatement();

    unique_ptr<Statement> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement();
    }

    return make_unique<IfStatement>(
        move(condition),
        move(thenBranch),
        move(elseBranch)
    );
}

// Parse while loop
unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenType::WHILE, "Expected 'while'");

    consume(
        TokenType::LEFT_PAREN,
        "Expected '(' after while"
    );

    auto condition = parseExpression();

    consume(
        TokenType::RIGHT_PAREN,
        "Expected ')' after condition"
    );

    auto body = parseStatement();

    return make_unique<WhileStatement>(
        move(condition),
        move(body)
    );
}

// Parse for loop
unique_ptr<Statement> Parser::parseForStatement() {
    consume(TokenType::FOR, "Expected 'for'");

    consume(
        TokenType::LEFT_PAREN,
        "Expected '(' after for"
    );

    unique_ptr<Statement> initializer = nullptr;

    if (match(TokenType::SEMICOLON)) {
        // Empty initializer
    }
    else if (isTypeSpecifier(currentToken().type)) {
        initializer = parseVariableDeclaration();
    }
    else if (check(TokenType::IDENTIFIER) &&
             peekToken().type == TokenType::ASSIGN) {
        initializer = parseAssignment();
    }
    else {
        initializer = parseExpressionStatement();
    }

    unique_ptr<Expression> condition = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        condition = parseExpression();
    }

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after for condition"
    );

    unique_ptr<Expression> increment = nullptr;

    if (!check(TokenType::RIGHT_PAREN)) {
        // Current AST does not support assignment expressions
        // or postfix increment expressions.
        if (check(TokenType::IDENTIFIER) &&
            (peekToken().type == TokenType::ASSIGN ||
             peekToken().type == TokenType::INCREMENT ||
             peekToken().type == TokenType::DECREMENT)) {
            reportError(
                "For-loop assignment/increment updates "
                "are not supported by the current AST"
            );
        }

        increment = parseExpression();
    }

    consume(
        TokenType::RIGHT_PAREN,
        "Expected ')' after for clauses"
    );

    auto body = parseStatement();

    return make_unique<ForStatement>(
        move(initializer),
        move(condition),
        move(increment),
        move(body)
    );
}

// Parse expression statement
unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto expression = parseExpression();

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after expression"
    );

    return make_unique<ExpressionStatement>(
        move(expression)
    );
}

// Expression entry point
unique_ptr<Expression> Parser::parseExpression() {
    return parseLogicalOr();
}

// Logical OR: ||
unique_ptr<Expression> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();

    while (match(TokenType::LOGICAL_OR)) {
        TokenType op = previousToken().type;
        auto right = parseLogicalAnd();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Logical AND: &&
unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto left = parseEquality();

    while (match(TokenType::LOGICAL_AND)) {
        TokenType op = previousToken().type;
        auto right = parseEquality();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Equality: == !=
unique_ptr<Expression> Parser::parseEquality() {
    auto left = parseRelational();

    while (check(TokenType::EQUAL) ||
           check(TokenType::NOT_EQUAL)) {

        TokenType op = currentToken().type;
        advance();

        auto right = parseRelational();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Relational: < > <= >=
unique_ptr<Expression> Parser::parseRelational() {
    auto left = parseAdditive();

    while (check(TokenType::LESS) ||
           check(TokenType::GREATER) ||
           check(TokenType::LESS_EQUAL) ||
           check(TokenType::GREATER_EQUAL)) {

        TokenType op = currentToken().type;
        advance();

        auto right = parseAdditive();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Addition and subtraction
unique_ptr<Expression> Parser::parseAdditive() {
    auto left = parseMultiplicative();

    while (check(TokenType::PLUS) ||
           check(TokenType::MINUS)) {

        TokenType op = currentToken().type;
        advance();

        auto right = parseMultiplicative();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Multiplication, division and modulo
unique_ptr<Expression> Parser::parseMultiplicative() {
    auto left = parseUnary();

    while (check(TokenType::MULTIPLY) ||
           check(TokenType::DIVIDE) ||
           check(TokenType::MODULO)) {

        TokenType op = currentToken().type;
        advance();

        auto right = parseUnary();

        left = make_unique<BinaryExpression>(
            op, move(left), move(right)
        );
    }

    return left;
}

// Unary operators: + - !
unique_ptr<Expression> Parser::parseUnary() {
    if (check(TokenType::PLUS) ||
        check(TokenType::MINUS) ||
        check(TokenType::LOGICAL_NOT)) {

        TokenType op = currentToken().type;
        advance();

        auto operand = parseUnary();

        return make_unique<UnaryExpression>(
            op, move(operand)
        );
    }

    return parsePrimary();
}

// Literals, identifiers and parentheses
unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::INTEGER_LITERAL)) {
        string value = currentToken().value;
        advance();
        return make_unique<IntegerLiteral>(value);
    }

    if (check(TokenType::FLOAT_LITERAL)) {
        string value = currentToken().value;
        advance();
        return make_unique<FloatLiteral>(value);
    }

    if (check(TokenType::CHAR_LITERAL)) {
        string value = currentToken().value;
        advance();
        return make_unique<CharacterLiteral>(value);
    }

    if (check(TokenType::IDENTIFIER)) {
        string name = currentToken().value;
        advance();
        return make_unique<Identifier>(name);
    }

    if (match(TokenType::LEFT_PAREN)) {
        auto expression = parseExpression();

        consume(
            TokenType::RIGHT_PAREN,
            "Expected ')' after expression"
        );

        return expression;
    }

    reportError("Expected expression");
    return nullptr;
}
