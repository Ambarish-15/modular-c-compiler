
#include "ir.h"

#include <iostream>
#include <stdexcept>
#include <utility>

using namespace std;

// Constructor
IRGenerator::IRGenerator()
    : temporaryCounter(0), labelCounter(0) {}

// Create a unique temporary variable
string IRGenerator::newTemporary() {
    return "__t" + to_string(++temporaryCounter);
}

// Create a unique label
string IRGenerator::newLabel() {
    return "__L" + to_string(++labelCounter);
}

// Add an IR instruction
void IRGenerator::emit(
    IROp op,
    const string& result,
    const string& arg1,
    const string& arg2
) {
    instructions.emplace_back(op, result, arg1, arg2);
}

// Convert binary operators to IR operations
IROp IRGenerator::binaryOperatorToIR(
    TokenType type
) const {
    switch (type) {
        case TokenType::PLUS:          return IROp::ADD;
        case TokenType::MINUS:         return IROp::SUB;
        case TokenType::MULTIPLY:      return IROp::MUL;
        case TokenType::DIVIDE:        return IROp::DIV;
        case TokenType::MODULO:        return IROp::MOD;

        case TokenType::EQUAL:         return IROp::EQ;
        case TokenType::NOT_EQUAL:     return IROp::NE;
        case TokenType::LESS:          return IROp::LT;
        case TokenType::GREATER:       return IROp::GT;
        case TokenType::LESS_EQUAL:    return IROp::LE;
        case TokenType::GREATER_EQUAL: return IROp::GE;

        case TokenType::LOGICAL_AND:   return IROp::LOGICAL_AND;
        case TokenType::LOGICAL_OR:    return IROp::LOGICAL_OR;

        default:
            throw runtime_error(
                "IR Error: Unsupported binary operator"
            );
    }
}

// Convert unary operators to IR operations
IROp IRGenerator::unaryOperatorToIR(
    TokenType type
) const {
    switch (type) {
        case TokenType::MINUS:
            return IROp::NEG;

        case TokenType::LOGICAL_NOT:
            return IROp::LOGICAL_NOT;

        default:
            throw runtime_error(
                "IR Error: Unsupported unary operator"
            );
    }
}

// Generate IR for the complete program
void IRGenerator::generate(const Program& program) {
    instructions.clear();
    temporaryCounter = 0;
    labelCounter = 0;

    for (const auto& function : program.functions) {
        if (function) {
            generateFunction(*function);
        }
    }
}

// Generate IR for a function
void IRGenerator::generateFunction(
    const FunctionDeclaration& function
) {
    emit(IROp::FUNCTION_BEGIN, function.name);

    // Function parameters are represented using PARAM.
    // The code generator must interpret these as incoming
    // parameters, not outgoing call arguments.
    for (const auto& parameter : function.parameters) {
        emit(IROp::PARAM, parameter.name);
    }

    if (function.body) {
        generateBlock(*function.body);
    }

    emit(IROp::FUNCTION_END, function.name);
}

// Generate IR for a block
void IRGenerator::generateBlock(
    const BlockStatement& block
) {
    for (const auto& statement : block.statements) {
        if (statement) {
            generateStatement(*statement);
        }
    }
}

// Generate IR for statements
void IRGenerator::generateStatement(
    const Statement& statement
) {
    // Variable declaration
    if (auto node =
        dynamic_cast<const VariableDeclaration*>(&statement)) {

        if (node->initializer) {
            string value =
                generateExpression(*node->initializer);

            emit(IROp::ASSIGN, node->name, value);
        }

        return;
    }

    // Assignment statement
    if (auto node =
        dynamic_cast<const AssignmentStatement*>(&statement)) {

        if (!node->value) {
            throw runtime_error(
                "IR Error: Assignment has no value"
            );
        }

        string value =
            generateExpression(*node->value);

        emit(IROp::ASSIGN, node->name, value);
        return;
    }

    // Return statement
    if (auto node =
        dynamic_cast<const ReturnStatement*>(&statement)) {

        if (node->value) {
            string value =
                generateExpression(*node->value);

            emit(IROp::RETURN, "", value);
        } else {
            emit(IROp::RETURN);
        }

        return;
    }

    // Expression statement
    if (auto node =
        dynamic_cast<const ExpressionStatement*>(&statement)) {

        if (node->expression) {
            generateExpression(*node->expression);
        }

        return;
    }

    // Nested block
    if (auto node =
        dynamic_cast<const BlockStatement*>(&statement)) {

        generateBlock(*node);
        return;
    }

    // If-else
    if (auto node =
        dynamic_cast<const IfStatement*>(&statement)) {

        generateIf(*node);
        return;
    }

    // While loop
    if (auto node =
        dynamic_cast<const WhileStatement*>(&statement)) {

        generateWhile(*node);
        return;
    }

    // For loop
    if (auto node =
        dynamic_cast<const ForStatement*>(&statement)) {

        generateFor(*node);
        return;
    }

    throw runtime_error(
        "IR Error: Unsupported statement type"
    );
}

// Generate IR for expressions
string IRGenerator::generateExpression(
    const Expression& expression
) {
    // Integer literal
    if (auto node =
        dynamic_cast<const IntegerLiteral*>(&expression)) {

        return node->value;
    }

    // Float literal
    if (auto node =
        dynamic_cast<const FloatLiteral*>(&expression)) {

        return node->value;
    }

    // Character literal
    if (auto node =
        dynamic_cast<const CharacterLiteral*>(&expression)) {

        return node->value;
    }

    // Identifier
    if (auto node =
        dynamic_cast<const Identifier*>(&expression)) {

        return node->name;
    }

    // Binary expression
    if (auto node =
        dynamic_cast<const BinaryExpression*>(&expression)) {

        return generateBinaryExpression(*node);
    }

    // Unary expression
    if (auto node =
        dynamic_cast<const UnaryExpression*>(&expression)) {

        return generateUnaryExpression(*node);
    }

    throw runtime_error(
        "IR Error: Unsupported expression type"
    );
}

// Generate IR for binary expressions
string IRGenerator::generateBinaryExpression(
    const BinaryExpression& expression
) {
    if (!expression.left || !expression.right) {
        throw runtime_error(
            "IR Error: Incomplete binary expression"
        );
    }

    // Logical AND with short-circuit evaluation
    if (expression.op == TokenType::LOGICAL_AND) {
        string result = newTemporary();
        string endLabel = newLabel();

        emit(IROp::ASSIGN, result, "0");

        string left =
            generateExpression(*expression.left);

        emit(IROp::IF_FALSE_GOTO, endLabel, left);

        string right =
            generateExpression(*expression.right);

        emit(IROp::IF_FALSE_GOTO, endLabel, right);

        emit(IROp::ASSIGN, result, "1");
        emit(IROp::LABEL, endLabel);

        return result;
    }

    // Logical OR with short-circuit evaluation
    if (expression.op == TokenType::LOGICAL_OR) {
        string result = newTemporary();
        string evaluateRight = newLabel();
        string endLabel = newLabel();

        string left =
            generateExpression(*expression.left);

        emit(IROp::IF_FALSE_GOTO, evaluateRight, left);
        emit(IROp::ASSIGN, result, "1");
        emit(IROp::GOTO, endLabel);

        emit(IROp::LABEL, evaluateRight);

        string right =
            generateExpression(*expression.right);

        // Convert right operand into boolean 0 or 1
        emit(IROp::NE, result, right, "0");

        emit(IROp::LABEL, endLabel);

        return result;
    }

    // Evaluate both operands
    string left =
        generateExpression(*expression.left);

    string right =
        generateExpression(*expression.right);

    string temporary = newTemporary();

    IROp operation =
        binaryOperatorToIR(expression.op);

    emit(operation, temporary, left, right);

    return temporary;
}

// Generate IR for unary expressions
string IRGenerator::generateUnaryExpression(
    const UnaryExpression& expression
) {
    if (!expression.operand) {
        throw runtime_error(
            "IR Error: Missing unary operand"
        );
    }

    string operand =
        generateExpression(*expression.operand);

    // Unary plus does not change the value
    if (expression.op == TokenType::PLUS) {
        return operand;
    }

    string temporary = newTemporary();

    IROp operation =
        unaryOperatorToIR(expression.op);

    emit(operation, temporary, operand);

    return temporary;
}

// Generate IR for if-else statements
void IRGenerator::generateIf(
    const IfStatement& statement
) {
    if (!statement.condition || !statement.thenBranch) {
        throw runtime_error(
            "IR Error: Incomplete if statement"
        );
    }

    string condition =
        generateExpression(*statement.condition);

    string elseLabel = newLabel();

    emit(IROp::IF_FALSE_GOTO, elseLabel, condition);

    generateStatement(*statement.thenBranch);

    if (statement.elseBranch) {
        string endLabel = newLabel();

        emit(IROp::GOTO, endLabel);
        emit(IROp::LABEL, elseLabel);

        generateStatement(*statement.elseBranch);

        emit(IROp::LABEL, endLabel);
    } else {
        emit(IROp::LABEL, elseLabel);
    }
}

// Generate IR for while loops
void IRGenerator::generateWhile(
    const WhileStatement& statement
) {
    if (!statement.condition || !statement.body) {
        throw runtime_error(
            "IR Error: Incomplete while statement"
        );
    }

    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);

    string condition =
        generateExpression(*statement.condition);

    emit(IROp::IF_FALSE_GOTO, endLabel, condition);

    generateStatement(*statement.body);

    emit(IROp::GOTO, startLabel);
    emit(IROp::LABEL, endLabel);
}

// Generate IR for for loops
void IRGenerator::generateFor(
    const ForStatement& statement
) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    // Initialization
    if (statement.initializer) {
        generateStatement(*statement.initializer);
    }

    // Loop start
    emit(IROp::LABEL, startLabel);

    // Condition
    if (statement.condition) {
        string condition =
            generateExpression(*statement.condition);

        emit(IROp::IF_FALSE_GOTO, endLabel, condition);
    }

    // Loop body
    if (statement.body) {
        generateStatement(*statement.body);
    }

    // Increment expression
    if (statement.increment) {
        generateExpression(*statement.increment);
    }

    // Repeat loop
    emit(IROp::GOTO, startLabel);

    // Loop exit
    emit(IROp::LABEL, endLabel);
}

// Get all generated instructions
const vector<IRInstruction>&
IRGenerator::getInstructions() const {
    return instructions;
}

// Convert one IR instruction to readable text
string irInstructionToString(
    const IRInstruction& instruction
) {
    const string& result = instruction.result;
    const string& arg1 = instruction.arg1;
    const string& arg2 = instruction.arg2;

    switch (instruction.op) {
        case IROp::LABEL:
            return result + ":";

        case IROp::FUNCTION_BEGIN:
            return "FUNCTION_BEGIN " + result;

        case IROp::FUNCTION_END:
            return "FUNCTION_END " + result;

        case IROp::ASSIGN:
            return result + " = " + arg1;

        case IROp::ADD:
            return result + " = " + arg1 + " + " + arg2;

        case IROp::SUB:
            return result + " = " + arg1 + " - " + arg2;

        case IROp::MUL:
            return result + " = " + arg1 + " * " + arg2;

        case IROp::DIV:
            return result + " = " + arg1 + " / " + arg2;

        case IROp::MOD:
            return result + " = " + arg1 + " % " + arg2;

        case IROp::NEG:
            return result + " = -" + arg1;

        case IROp::LOGICAL_NOT:
            return result + " = !" + arg1;

        case IROp::EQ:
            return result + " = " + arg1 + " == " + arg2;

        case IROp::NE:
            return result + " = " + arg1 + " != " + arg2;

        case IROp::LT:
            return result + " = " + arg1 + " < " + arg2;

        case IROp::GT:
            return result + " = " + arg1 + " > " + arg2;

        case IROp::LE:
            return result + " = " + arg1 + " <= " + arg2;

        case IROp::GE:
            return result + " = " + arg1 + " >= " + arg2;

        case IROp::LOGICAL_AND:
            return result + " = " + arg1 + " && " + arg2;

        case IROp::LOGICAL_OR:
            return result + " = " + arg1 + " || " + arg2;

        case IROp::GOTO:
            return "goto " + result;

        case IROp::IF_FALSE_GOTO:
            return "ifFalse " + arg1 + " goto " + result;

        case IROp::PARAM:
            return "param " + result;

        case IROp::CALL:
            return result + " = call " + arg1 + ", " + arg2;

        case IROp::RETURN:
            return arg1.empty()
                ? "return"
                : "return " + arg1;
    }

    return "UNKNOWN_IR";
}

// Print all generated IR instructions
void IRGenerator::print() const {
    cout << "\n========== THREE ADDRESS CODE ==========\n";

    for (size_t i = 0; i < instructions.size(); ++i) {
        cout << i + 1 << ": "
             << irInstructionToString(instructions[i])
             << endl;
    }

    cout << "========================================\n";
}
