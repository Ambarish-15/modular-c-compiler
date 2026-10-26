
#ifndef IR_H
#define IR_H

#include <string>
#include <vector>
#include <cstddef>

#include "ast.h"

// Supported intermediate-code operations
enum class IROp {
    LABEL,
    FUNCTION_BEGIN,
    FUNCTION_END,

    ASSIGN,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    NEG,
    LOGICAL_NOT,

    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,

    LOGICAL_AND,
    LOGICAL_OR,

    GOTO,
    IF_FALSE_GOTO,

    PARAM,
    CALL,
    RETURN
};

// One three-address-code instruction
struct IRInstruction {
    IROp op;

    // Destination/result, when applicable
    std::string result;

    // Source operands, when applicable
    std::string arg1;
    std::string arg2;

    IRInstruction(
        IROp op,
        const std::string& result = "",
        const std::string& arg1 = "",
        const std::string& arg2 = ""
    )
        : op(op),
          result(result),
          arg1(arg1),
          arg2(arg2) {}
};

// Intermediate representation generator
class IRGenerator {
private:
    std::vector<IRInstruction> instructions;

    std::size_t temporaryCounter;
    std::size_t labelCounter;

    // Generate unique temporary variable names
    std::string newTemporary();

    // Generate unique labels for control flow
    std::string newLabel();

    // Append an instruction
    void emit(
        IROp op,
        const std::string& result = "",
        const std::string& arg1 = "",
        const std::string& arg2 = ""
    );

    // Generate IR for program structures
    void generateFunction(
        const FunctionDeclaration& function
    );

    void generateBlock(
        const BlockStatement& block
    );

    void generateStatement(
        const Statement& statement
    );

    // Generate IR for expressions
    // Returns the name of a temporary, variable, or literal
    std::string generateExpression(
        const Expression& expression
    );

    // Generate IR for specific expressions
    std::string generateBinaryExpression(
        const BinaryExpression& expression
    );

    std::string generateUnaryExpression(
        const UnaryExpression& expression
    );

    // Generate control-flow instructions
    void generateIf(
        const IfStatement& statement
    );

    void generateWhile(
        const WhileStatement& statement
    );

    void generateFor(
        const ForStatement& statement
    );

    // Convert token operators into IR operations
    IROp binaryOperatorToIR(TokenType type) const;
    IROp unaryOperatorToIR(TokenType type) const;

public:
    IRGenerator();

    // Generate IR from a complete, validated AST
    void generate(const Program& program);

    // Access generated instructions
    const std::vector<IRInstruction>& getInstructions() const;

    // Display the generated three-address code
    void print() const;
};

// Convert an IR instruction to readable text
std::string irInstructionToString(
    const IRInstruction& instruction
);

#endif
