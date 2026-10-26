
#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

#include "ir.h"

// Generates 32-bit x86 assembly from three-address IR
class CodeGenerator {
private:
    // Input intermediate instructions
    std::vector<IRInstruction> instructions;

    // Generated assembly lines
    std::vector<std::string> assembly;

    // Stack offsets for variables and temporaries
    std::unordered_map<std::string, int> stackOffsets;

    // Current function information
    std::string currentFunction;
    std::string returnLabel;

    // Number of bytes reserved for local variables
    int stackSize;

    // Counter for internal assembly labels
    std::size_t internalLabelCounter;

    // Reset state before generating a function
    void resetFunctionState();

    // Determine stack space needed by a function
    void allocateStackVariables(
        std::size_t beginIndex,
        std::size_t endIndex
    );

    // Allocate a stack slot for an identifier
    void allocateVariable(
        const std::string& name
    );

    // Return an operand in AT&T assembly syntax
    // Examples: $10, -4(%ebp)
    std::string getOperand(
        const std::string& operand
    ) const;

    // Return the stack address of a variable
    std::string getVariableAddress(
        const std::string& name
    ) const;

    // Check whether an operand is an integer literal
    bool isIntegerLiteral(
        const std::string& operand
    ) const;

    // Generate a unique internal label
    std::string newInternalLabel();

    // Append an assembly instruction
    void emit(const std::string& line);

    // Generate function prologue and epilogue
    void generateFunctionBegin(
        const std::string& functionName
    );

    void generateFunctionEnd();

    // Translate one IR instruction
    void generateInstruction(
        const IRInstruction& instruction
    );

    // Assignment and arithmetic
    void generateAssignment(
        const IRInstruction& instruction
    );

    void generateArithmetic(
        const IRInstruction& instruction
    );

    void generateUnary(
        const IRInstruction& instruction
    );

    // Comparisons and logical operators
    void generateComparison(
        const IRInstruction& instruction
    );

    void generateLogical(
        const IRInstruction& instruction
    );

    // Labels, jumps, and returns
    void generateLabel(
        const IRInstruction& instruction
    );

    void generateGoto(
        const IRInstruction& instruction
    );

    void generateConditionalJump(
        const IRInstruction& instruction
    );

    void generateReturn(
        const IRInstruction& instruction
    );

    // Convert IR comparison operation to x86 setcc
    std::string comparisonInstruction(
        IROp op
    ) const;

public:
    CodeGenerator();

    explicit CodeGenerator(
        const std::vector<IRInstruction>& ir
    );

    // Replace the input IR
    void setInstructions(
        const std::vector<IRInstruction>& ir
    );

    // Translate the IR into assembly
    void generate();

    // Retrieve the generated assembly lines
    const std::vector<std::string>&
    getAssembly() const;

    // Print generated assembly
    void print() const;

    // Save assembly to a .s file
    bool writeToFile(
        const std::string& filename
    ) const;
};

#endif
