
#include "semantic.h"

#include <iostream>
#include <utility>

using namespace std;

// Constructor
SemanticAnalyzer::SemanticAnalyzer()
    : currentFunction(""),
      currentReturnType("") {}

// Report a semantic error
void SemanticAnalyzer::reportError(const string& message) {
    errors.push_back("Semantic Error: " + message);
}

// Check numeric types
bool SemanticAnalyzer::isNumericType(
    const string& type
) const {
    return type == "int" ||
           type == "float" ||
           type == "char";
}

// Check assignment compatibility
bool SemanticAnalyzer::isTypeCompatible(
    const string& targetType,
    const string& sourceType
) const {
    // Invalid expressions are reported elsewhere
    if (sourceType == "error" || targetType == "error") {
        return true;
    }

    if (targetType == sourceType) {
        return true;
    }

    // Allow implicit conversion between numeric types
    if (isNumericType(targetType) &&
        isNumericType(sourceType)) {
        return true;
    }

    return false;
}

// Analyze the complete program
bool SemanticAnalyzer::analyze(const Program& program) {
    errors.clear();
    symbolTable = SymbolTable();

    currentFunction.clear();
    currentReturnType.clear();

    // First register all function names
    declareFunctions(program);

    // Then analyze their bodies
    for (const auto& function : program.functions) {
        if (function) {
            analyzeFunction(*function);
        }
    }

    return errors.empty();
}

// Register functions in global scope
void SemanticAnalyzer::declareFunctions(
    const Program& program
) {
    for (const auto& function : program.functions) {
        if (!function) {
            continue;
        }

        vector<string> parameterTypes;

        for (const auto& parameter : function->parameters) {
            parameterTypes.push_back(parameter.type);
        }

        Symbol symbol(
            function->name,
            function->returnType,
            SymbolKind::FUNCTION,
            parameterTypes
        );

        if (!symbolTable.declare(symbol)) {
            reportError(
                "Function '" + function->name +
                "' is already declared"
            );
        }
    }
}

// Analyze a function definition
void SemanticAnalyzer::analyzeFunction(
    const FunctionDeclaration& function
) {
    currentFunction = function.name;
    currentReturnType = function.returnType;

    symbolTable.enterScope();

    // Add parameters to function scope
    for (const auto& parameter : function.parameters) {
        Symbol symbol(
            parameter.name,
            parameter.type,
            SymbolKind::PARAMETER
        );

        if (!symbolTable.declare(symbol)) {
            reportError(
                "Duplicate parameter '" +
                parameter.name +
                "' in function '" +
                function.name + "'"
            );
        }
    }

    if (function.body) {
        // Parameters and outer function body share a scope
        analyzeBlock(*function.body, false);
    }

    symbolTable.exitScope();

    currentFunction.clear();
    currentReturnType.clear();
}

// Analyze a block of statements
void SemanticAnalyzer::analyzeBlock(
    const BlockStatement& block,
    bool createScope
) {
    if (createScope) {
        symbolTable.enterScope();
    }

    for (const auto& statement : block.statements) {
        if (statement) {
            analyzeStatement(*statement);
        }
    }

    if (createScope) {
        symbolTable.exitScope();
    }
}

// Dispatch statements to the correct analyzer
void SemanticAnalyzer::analyzeStatement(
    const Statement& statement
) {
    if (auto node =
        dynamic_cast<const VariableDeclaration*>(&statement)) {
        analyzeVariableDeclaration(*node);
    }
    else if (auto node =
        dynamic_cast<const AssignmentStatement*>(&statement)) {
        analyzeAssignment(*node);
    }
    else if (auto node =
        dynamic_cast<const ReturnStatement*>(&statement)) {
        analyzeReturn(*node);
    }
    else if (auto node =
        dynamic_cast<const IfStatement*>(&statement)) {
        analyzeIf(*node);
    }
    else if (auto node =
        dynamic_cast<const WhileStatement*>(&statement)) {
        analyzeWhile(*node);
    }
    else if (auto node =
        dynamic_cast<const ForStatement*>(&statement)) {
        analyzeFor(*node);
    }
    else if (auto node =
        dynamic_cast<const BlockStatement*>(&statement)) {
        analyzeBlock(*node);
    }
    else if (auto node =
        dynamic_cast<const ExpressionStatement*>(&statement)) {
        if (node->expression) {
            analyzeExpression(*node->expression);
        }
    }
    else {
        reportError("Unsupported statement type");
    }
}

// Analyze variable declarations
void SemanticAnalyzer::analyzeVariableDeclaration(
    const VariableDeclaration& declaration
) {
    // Check duplicate declarations
    if (symbolTable.lookupCurrentScope(declaration.name)) {
        reportError(
            "Variable '" + declaration.name +
            "' already declared in this scope"
        );
        return;
    }

    if (declaration.type == "void") {
        reportError(
            "Variable '" + declaration.name +
            "' cannot have type void"
        );
        return;
    }

    Symbol symbol(
        declaration.name,
        declaration.type,
        SymbolKind::VARIABLE
    );

    symbolTable.declare(symbol);

    if (declaration.initializer) {
        string expressionType =
            analyzeExpression(*declaration.initializer);

        if (!isTypeCompatible(
                declaration.type, expressionType)) {
            reportError(
                "Cannot initialize variable '" +
                declaration.name + "' of type '" +
                declaration.type +
                "' with expression of type '" +
                expressionType + "'"
            );
        }
    }
}

// Analyze assignment statements
void SemanticAnalyzer::analyzeAssignment(
    const AssignmentStatement& assignment
) {
    const Symbol* symbol =
        symbolTable.lookup(assignment.name);

    if (!symbol) {
        reportError(
            "Undeclared variable '" +
            assignment.name + "'"
        );

        if (assignment.value) {
            analyzeExpression(*assignment.value);
        }
        return;
    }

    if (symbol->kind == SymbolKind::FUNCTION) {
        reportError(
            "Cannot assign to function '" +
            assignment.name + "'"
        );
        return;
    }

    if (!assignment.value) {
        reportError("Missing assignment expression");
        return;
    }

    string expressionType =
        analyzeExpression(*assignment.value);

    if (!isTypeCompatible(symbol->type, expressionType)) {
        reportError(
            "Type mismatch in assignment to '" +
            assignment.name + "'"
        );
    }
}

// Analyze return statements
void SemanticAnalyzer::analyzeReturn(
    const ReturnStatement& statement
) {
    if (currentFunction.empty()) {
        reportError("Return statement outside function");
        return;
    }

    if (!statement.value) {
        if (currentReturnType != "void") {
            reportError(
                "Function '" + currentFunction +
                "' must return a value of type '" +
                currentReturnType + "'"
            );
        }
        return;
    }

    string returnType =
        analyzeExpression(*statement.value);

    if (currentReturnType == "void") {
        reportError(
            "Void function '" + currentFunction +
            "' cannot return a value"
        );
    }
    else if (!isTypeCompatible(
                 currentReturnType, returnType)) {
        reportError(
            "Return type mismatch in function '" +
            currentFunction + "'"
        );
    }
}

// Analyze if-else statements
void SemanticAnalyzer::analyzeIf(
    const IfStatement& statement
) {
    if (statement.condition) {
        string conditionType =
            analyzeExpression(*statement.condition);

        if (conditionType != "error" &&
            !isNumericType(conditionType)) {
            reportError("Invalid if condition type");
        }
    }

    if (statement.thenBranch) {
        analyzeStatement(*statement.thenBranch);
    }

    if (statement.elseBranch) {
        analyzeStatement(*statement.elseBranch);
    }
}

// Analyze while loops
void SemanticAnalyzer::analyzeWhile(
    const WhileStatement& statement
) {
    if (statement.condition) {
        string conditionType =
            analyzeExpression(*statement.condition);

        if (conditionType != "error" &&
            !isNumericType(conditionType)) {
            reportError("Invalid while condition type");
        }
    }

    if (statement.body) {
        analyzeStatement(*statement.body);
    }
}

// Analyze for loops
void SemanticAnalyzer::analyzeFor(
    const ForStatement& statement
) {
    // For-loop initializer declarations have their own scope
    symbolTable.enterScope();

    if (statement.initializer) {
        analyzeStatement(*statement.initializer);
    }

    if (statement.condition) {
        string conditionType =
            analyzeExpression(*statement.condition);

        if (conditionType != "error" &&
            !isNumericType(conditionType)) {
            reportError("Invalid for condition type");
        }
    }

    if (statement.increment) {
        analyzeExpression(*statement.increment);
    }

    if (statement.body) {
        analyzeStatement(*statement.body);
    }

    symbolTable.exitScope();
}

// Determine expression type
string SemanticAnalyzer::analyzeExpression(
    const Expression& expression
) {
    if (dynamic_cast<const IntegerLiteral*>(&expression)) {
        return "int";
    }

    if (dynamic_cast<const FloatLiteral*>(&expression)) {
        return "float";
    }

    if (dynamic_cast<const CharacterLiteral*>(&expression)) {
        return "char";
    }

    if (auto node =
        dynamic_cast<const Identifier*>(&expression)) {

        const Symbol* symbol =
            symbolTable.lookup(node->name);

        if (!symbol) {
            reportError(
                "Undeclared variable '" +
                node->name + "'"
            );
            return "error";
        }

        if (symbol->kind == SymbolKind::FUNCTION) {
            reportError(
                "Function '" + node->name +
                "' cannot be used as a variable"
            );
            return "error";
        }

        return symbol->type;
    }

    if (auto node =
        dynamic_cast<const BinaryExpression*>(&expression)) {
        return analyzeBinaryExpression(*node);
    }

    if (auto node =
        dynamic_cast<const UnaryExpression*>(&expression)) {
        return analyzeUnaryExpression(*node);
    }

    reportError("Unsupported expression type");
    return "error";
}

// Analyze binary expressions
string SemanticAnalyzer::analyzeBinaryExpression(
    const BinaryExpression& expression
) {
    if (!expression.left || !expression.right) {
        reportError("Incomplete binary expression");
        return "error";
    }

    string leftType =
        analyzeExpression(*expression.left);

    string rightType =
        analyzeExpression(*expression.right);

    if (leftType == "error" || rightType == "error") {
        return "error";
    }

    TokenType op = expression.op;

    // Arithmetic operators
    if (op == TokenType::PLUS ||
        op == TokenType::MINUS ||
        op == TokenType::MULTIPLY ||
        op == TokenType::DIVIDE) {

        if (!isNumericType(leftType) ||
            !isNumericType(rightType)) {
            reportError(
                "Arithmetic operands must be numeric"
            );
            return "error";
        }

        if (leftType == "float" || rightType == "float") {
            return "float";
        }

        return "int";
    }

    // Modulo requires integer operands
    if (op == TokenType::MODULO) {
        if (leftType == "float" ||
            rightType == "float" ||
            !isNumericType(leftType) ||
            !isNumericType(rightType)) {
            reportError(
                "Modulo operator requires integer operands"
            );
            return "error";
        }

        return "int";
    }

    // Comparison operators
    if (op == TokenType::EQUAL ||
        op == TokenType::NOT_EQUAL ||
        op == TokenType::LESS ||
        op == TokenType::GREATER ||
        op == TokenType::LESS_EQUAL ||
        op == TokenType::GREATER_EQUAL) {

        if (!isNumericType(leftType) ||
            !isNumericType(rightType)) {
            reportError(
                "Comparison operands must be numeric"
            );
            return "error";
        }

        return "int";
    }

    // Logical operators
    if (op == TokenType::LOGICAL_AND ||
        op == TokenType::LOGICAL_OR) {

        if (!isNumericType(leftType) ||
            !isNumericType(rightType)) {
            reportError(
                "Logical operands must be scalar numeric values"
            );
            return "error";
        }

        return "int";
    }

    reportError("Unsupported binary operator");
    return "error";
}

// Analyze unary expressions
string SemanticAnalyzer::analyzeUnaryExpression(
    const UnaryExpression& expression
) {
    if (!expression.operand) {
        reportError("Missing unary operand");
        return "error";
    }

    string operandType =
        analyzeExpression(*expression.operand);

    if (operandType == "error") {
        return "error";
    }

    if (!isNumericType(operandType)) {
        reportError("Unary operand must be numeric");
        return "error";
    }

    if (expression.op == TokenType::LOGICAL_NOT) {
        return "int";
    }

    if (expression.op == TokenType::PLUS ||
        expression.op == TokenType::MINUS) {
        return operandType == "char" ? "int" : operandType;
    }

    reportError("Unsupported unary operator");
    return "error";
}

// Return all semantic errors
const vector<string>& SemanticAnalyzer::getErrors() const {
    return errors;
}

// Print semantic analysis result
void SemanticAnalyzer::printErrors() const {
    if (errors.empty()) {
        cout << "Semantic Analysis Successful!" << endl;
        return;
    }

    cout << "\nSemantic Analysis Failed!" << endl;

    for (const string& error : errors) {
        cout << error << endl;
    }
}

// Access the symbol table
const SymbolTable& SemanticAnalyzer::getSymbolTable() const {
    return symbolTable;
}
