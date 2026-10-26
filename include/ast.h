
#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "token.h"

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Base class for expressions
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Base class for statements
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Integer literal: 10, 25, 100
class IntegerLiteral : public Expression {
public:
    std::string value;

    explicit IntegerLiteral(const std::string& value)
        : value(value) {}
};

// Floating-point literal: 3.14
class FloatLiteral : public Expression {
public:
    std::string value;

    explicit FloatLiteral(const std::string& value)
        : value(value) {}
};

// Character literal: 'a'
class CharacterLiteral : public Expression {
public:
    std::string value;

    explicit CharacterLiteral(const std::string& value)
        : value(value) {}
};

// Variable reference: a, sum, count
class Identifier : public Expression {
public:
    std::string name;

    explicit Identifier(const std::string& name)
        : name(name) {}
};

// Binary expression: a + b, x * y
class BinaryExpression : public Expression {
public:
    TokenType op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryExpression(
        TokenType op,
        std::unique_ptr<Expression> left,
        std::unique_ptr<Expression> right
    )
        : op(op),
          left(std::move(left)),
          right(std::move(right)) {}
};

// Unary expression: -a, !flag
class UnaryExpression : public Expression {
public:
    TokenType op;
    std::unique_ptr<Expression> operand;

    UnaryExpression(
        TokenType op,
        std::unique_ptr<Expression> operand
    )
        : op(op),
          operand(std::move(operand)) {}
};

// Variable declaration: int a = 10;
class VariableDeclaration : public Statement {
public:
    std::string type;
    std::string name;
    std::unique_ptr<Expression> initializer;

    VariableDeclaration(
        const std::string& type,
        const std::string& name,
        std::unique_ptr<Expression> initializer = nullptr
    )
        : type(type),
          name(name),
          initializer(std::move(initializer)) {}
};

// Assignment: a = 20;
class AssignmentStatement : public Statement {
public:
    std::string name;
    std::unique_ptr<Expression> value;

    AssignmentStatement(
        const std::string& name,
        std::unique_ptr<Expression> value
    )
        : name(name),
          value(std::move(value)) {}
};

// Return statement: return a + b;
class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(
        std::unique_ptr<Expression> value = nullptr
    )
        : value(std::move(value)) {}
};

// Expression statement: a + b;
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(
        std::unique_ptr<Expression> expression
    )
        : expression(std::move(expression)) {}
};

// Block statement: { statement1; statement2; }
class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    void addStatement(std::unique_ptr<Statement> statement) {
        statements.push_back(std::move(statement));
    }
};

// If-else statement
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;

    IfStatement(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> thenBranch,
        std::unique_ptr<Statement> elseBranch = nullptr
    )
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};

// While loop
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileStatement(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> body
    )
        : condition(std::move(condition)),
          body(std::move(body)) {}
};

// For loop
class ForStatement : public Statement {
public:
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;

    ForStatement(
        std::unique_ptr<Statement> initializer,
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment,
        std::unique_ptr<Statement> body
    )
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}
};

// Function parameter: int a
struct Parameter {
    std::string type;
    std::string name;

    Parameter(
        const std::string& type,
        const std::string& name
    )
        : type(type), name(name) {}
};

// Function definition: int main() { ... }
class FunctionDeclaration : public ASTNode {
public:
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::unique_ptr<BlockStatement> body;

    FunctionDeclaration(
        const std::string& returnType,
        const std::string& name,
        std::vector<Parameter> parameters,
        std::unique_ptr<BlockStatement> body
    )
        : returnType(returnType),
          name(name),
          parameters(std::move(parameters)),
          body(std::move(body)) {}
};

// Root node of the complete program
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDeclaration>> functions;

    void addFunction(
        std::unique_ptr<FunctionDeclaration> function
    ) {
        functions.push_back(std::move(function));
    }
};

#endif
