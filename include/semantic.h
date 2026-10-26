
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <memory>
#include <vector>

#include "ast.h"
#include "symbol_table.h"

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;

    // Track the function currently being analyzed
    std::string currentFunction;
    std::string currentReturnType;

    // Error messages
    std::vector<std::string> errors;

    // Register function declarations in global scope
    void declareFunctions(const Program& program);

    // Analyze function definitions
    void analyzeFunction(const FunctionDeclaration& function);

    // Analyze statements and blocks
    void analyzeBlock(
        const BlockStatement& block,
        bool createScope = true
    );

    void analyzeStatement(const Statement& statement);

    void analyzeVariableDeclaration(
        const VariableDeclaration& declaration
    );

    void analyzeAssignment(
        const AssignmentStatement& assignment
    );

    void analyzeReturn(
        const ReturnStatement& statement
    );

    void analyzeIf(const IfStatement& statement);

    void analyzeWhile(const WhileStatement& statement);

    void analyzeFor(const ForStatement& statement);

    // Determine expression types
    std::string analyzeExpression(
        const Expression& expression
    );

    std::string analyzeBinaryExpression(
        const BinaryExpression& expression
    );

    std::string analyzeUnaryExpression(
        const UnaryExpression& expression
    );

    // Type compatibility checks
    bool isNumericType(const std::string& type) const;

    bool isTypeCompatible(
        const std::string& targetType,
        const std::string& sourceType
    ) const;

    // Store semantic errors
    void reportError(const std::string& message);

public:
    SemanticAnalyzer();

    // Analyze the complete AST
    // Returns true if no semantic errors are found
    bool analyze(const Program& program);

    // Retrieve semantic errors
    const std::vector<std::string>& getErrors() const;

    // Print semantic errors or success message
    void printErrors() const;

    // Access symbol table for debugging
    const SymbolTable& getSymbolTable() const;
};

#endif
