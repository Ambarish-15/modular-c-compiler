
#include "codegen.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <climits>
#include <utility>

using namespace std;

// Constructors
CodeGenerator::CodeGenerator()
    : stackSize(0), internalLabelCounter(0) {}

CodeGenerator::CodeGenerator(
    const vector<IRInstruction>& ir
) : instructions(ir),
    stackSize(0),
    internalLabelCounter(0) {}

// Replace input IR
void CodeGenerator::setInstructions(
    const vector<IRInstruction>& ir
) {
    instructions = ir;
    assembly.clear();
    resetFunctionState();
    internalLabelCounter = 0;
}

// Reset function-specific information
void CodeGenerator::resetFunctionState() {
    stackOffsets.clear();
    currentFunction.clear();
    returnLabel.clear();
    stackSize = 0;
}

// Check signed decimal integer literals
bool CodeGenerator::isIntegerLiteral(
    const string& operand
) const {
    if (operand.empty()) {
        return false;
    }

    size_t i = 0;

    if (operand[i] == '+' || operand[i] == '-') {
        ++i;
    }

    if (i == operand.size()) {
        return false;
    }

    for (; i < operand.size(); ++i) {
        if (!isdigit(
            static_cast<unsigned char>(operand[i]))) {
            return false;
        }
    }

    try {
        size_t consumed = 0;
        long long value = stoll(operand, &consumed);

        return consumed == operand.size() &&
               value >= INT_MIN &&
               value <= INT_MAX;
    }
    catch (const exception&) {
        return false;
    }
}

// Allocate a stack slot for a local identifier
void CodeGenerator::allocateVariable(
    const string& name
) {
    if (name.empty() || isIntegerLiteral(name)) {
        return;
    }

    if (stackOffsets.find(name) != stackOffsets.end()) {
        return;
    }

    stackSize += 4;
    stackOffsets[name] = -stackSize;
}

// Find stack locations for a function
void CodeGenerator::allocateStackVariables(
    size_t beginIndex,
    size_t endIndex
) {
    int parameterOffset = 8;

    // First register incoming function parameters
    for (size_t i = beginIndex; i < endIndex; ++i) {
        const IRInstruction& ins = instructions[i];

        if (ins.op == IROp::PARAM) {
            if (ins.result.empty() ||
                stackOffsets.count(ins.result)) {
                throw runtime_error(
                    "Codegen Error: Invalid or duplicate parameter"
                );
            }

            stackOffsets[ins.result] = parameterOffset;
            parameterOffset += 4;
        }
    }

    auto allocateOperand = [&](const string& operand) {
        if (!operand.empty() && !isIntegerLiteral(operand)) {
            allocateVariable(operand);
        }
    };

    for (size_t i = beginIndex; i < endIndex; ++i) {
        const IRInstruction& ins = instructions[i];

        switch (ins.op) {
            case IROp::ASSIGN:
            case IROp::NEG:
            case IROp::LOGICAL_NOT:
                allocateOperand(ins.result);
                allocateOperand(ins.arg1);
                break;

            case IROp::ADD:
            case IROp::SUB:
            case IROp::MUL:
            case IROp::DIV:
            case IROp::MOD:
            case IROp::EQ:
            case IROp::NE:
            case IROp::LT:
            case IROp::GT:
            case IROp::LE:
            case IROp::GE:
            case IROp::LOGICAL_AND:
            case IROp::LOGICAL_OR:
                allocateOperand(ins.result);
                allocateOperand(ins.arg1);
                allocateOperand(ins.arg2);
                break;

            case IROp::IF_FALSE_GOTO:
            case IROp::RETURN:
                allocateOperand(ins.arg1);
                break;

            default:
                break;
        }
    }

    // Align local stack allocation to 16 bytes
    if (stackSize % 16 != 0) {
        stackSize += 16 - (stackSize % 16);
    }
}

// Get address of a variable
string CodeGenerator::getVariableAddress(
    const string& name
) const {
    auto it = stackOffsets.find(name);

    if (it == stackOffsets.end()) {
        throw runtime_error(
            "Codegen Error: Unknown variable '" + name + "'"
        );
    }

    return to_string(it->second) + "(%ebp)";
}

// Convert an IR operand into an assembly operand
string CodeGenerator::getOperand(
    const string& operand
) const {
    if (isIntegerLiteral(operand)) {
        return "$" + operand;
    }

    return getVariableAddress(operand);
}

// Generate internal assembly label
string CodeGenerator::newInternalLabel() {
    return ".LCG" + to_string(++internalLabelCounter);
}

// Append assembly line
void CodeGenerator::emit(const string& line) {
    assembly.push_back(line);
}

// Function prologue
void CodeGenerator::generateFunctionBegin(
    const string& functionName
) {
    currentFunction = functionName;
    returnLabel = newInternalLabel();

    // 32-bit Windows COFF symbol convention
    string symbolName = "_" + functionName;

    emit("");
    emit("    .globl " + symbolName);
    emit(symbolName + ":");

    emit("    pushl %ebp");
    emit("    movl %esp, %ebp");

    if (stackSize > 0) {
        emit("    subl $" + to_string(stackSize) + ", %esp");
    }
}

// Function epilogue
void CodeGenerator::generateFunctionEnd() {
    emit(returnLabel + ":");
    emit("    leave");
    emit("    ret");

    currentFunction.clear();
    returnLabel.clear();
}

// Assignment: result = arg1
void CodeGenerator::generateAssignment(
    const IRInstruction& instruction
) {
    emit("    movl " +
         getOperand(instruction.arg1) + ", %eax");

    emit("    movl %eax, " +
         getVariableAddress(instruction.result));
}

// Arithmetic instructions
void CodeGenerator::generateArithmetic(
    const IRInstruction& instruction
) {
    const string left = getOperand(instruction.arg1);
    const string right = getOperand(instruction.arg2);
    const string destination =
        getVariableAddress(instruction.result);

    emit("    movl " + left + ", %eax");

    switch (instruction.op) {
        case IROp::ADD:
            emit("    addl " + right + ", %eax");
            break;

        case IROp::SUB:
            emit("    subl " + right + ", %eax");
            break;

        case IROp::MUL:
            emit("    imull " + right + ", %eax");
            break;

        case IROp::DIV:
        case IROp::MOD:
            emit("    cltd");

            // idivl does not accept an immediate operand
            if (isIntegerLiteral(instruction.arg2)) {
                emit("    movl " + right + ", %ecx");
                emit("    idivl %ecx");
            } else {
                emit("    idivl " + right);
            }
            break;

        default:
            throw runtime_error(
                "Codegen Error: Unsupported arithmetic operation"
            );
    }

    if (instruction.op == IROp::MOD) {
        emit("    movl %edx, " + destination);
    } else {
        emit("    movl %eax, " + destination);
    }
}

// Unary operations
void CodeGenerator::generateUnary(
    const IRInstruction& instruction
) {
    emit("    movl " +
         getOperand(instruction.arg1) + ", %eax");

    switch (instruction.op) {
        case IROp::NEG:
            emit("    negl %eax");
            break;

        case IROp::LOGICAL_NOT:
            emit("    cmpl $0, %eax");
            emit("    sete %al");
            emit("    movzbl %al, %eax");
            break;

        default:
            throw runtime_error(
                "Codegen Error: Unsupported unary operation"
            );
    }

    emit("    movl %eax, " +
         getVariableAddress(instruction.result));
}

// Convert comparison to setcc instruction
string CodeGenerator::comparisonInstruction(
    IROp op
) const {
    switch (op) {
        case IROp::EQ: return "sete";
        case IROp::NE: return "setne";
        case IROp::LT: return "setl";
        case IROp::GT: return "setg";
        case IROp::LE: return "setle";
        case IROp::GE: return "setge";

        default:
            throw runtime_error(
                "Codegen Error: Invalid comparison operation"
            );
    }
}

// Comparison: result = arg1 <op> arg2
void CodeGenerator::generateComparison(
    const IRInstruction& instruction
) {
    emit("    movl " +
         getOperand(instruction.arg1) + ", %eax");

    emit("    cmpl " +
         getOperand(instruction.arg2) + ", %eax");

    emit("    " +
         comparisonInstruction(instruction.op) + " %al");

    emit("    movzbl %al, %eax");

    emit("    movl %eax, " +
         getVariableAddress(instruction.result));
}

// Logical AND / OR
void CodeGenerator::generateLogical(
    const IRInstruction& instruction
) {
    // Convert left operand to boolean
    emit("    movl " +
         getOperand(instruction.arg1) + ", %eax");

    emit("    cmpl $0, %eax");
    emit("    setne %al");
    emit("    movzbl %al, %eax");

    // Convert right operand to boolean
    emit("    movl " +
         getOperand(instruction.arg2) + ", %ecx");

    emit("    cmpl $0, %ecx");
    emit("    setne %cl");
    emit("    movzbl %cl, %ecx");

    if (instruction.op == IROp::LOGICAL_AND) {
        emit("    andl %ecx, %eax");
    }
    else if (instruction.op == IROp::LOGICAL_OR) {
        emit("    orl %ecx, %eax");
    }
    else {
        throw runtime_error(
            "Codegen Error: Invalid logical operation"
        );
    }

    emit("    movl %eax, " +
         getVariableAddress(instruction.result));
}

// Emit IR label
void CodeGenerator::generateLabel(
    const IRInstruction& instruction
) {
    emit("." + instruction.result + ":");
}

// Unconditional jump
void CodeGenerator::generateGoto(
    const IRInstruction& instruction
) {
    emit("    jmp ." + instruction.result);
}

// Conditional jump when value is zero
void CodeGenerator::generateConditionalJump(
    const IRInstruction& instruction
) {
    emit("    movl " +
         getOperand(instruction.arg1) + ", %eax");

    emit("    cmpl $0, %eax");

    emit("    je ." + instruction.result);
}

// Return from function
void CodeGenerator::generateReturn(
    const IRInstruction& instruction
) {
    if (!instruction.arg1.empty()) {
        emit("    movl " +
             getOperand(instruction.arg1) + ", %eax");
    }

    emit("    jmp " + returnLabel);
}

// Dispatch IR operations
void CodeGenerator::generateInstruction(
    const IRInstruction& instruction
) {
    switch (instruction.op) {
        case IROp::ASSIGN:
            generateAssignment(instruction);
            break;

        case IROp::ADD:
        case IROp::SUB:
        case IROp::MUL:
        case IROp::DIV:
        case IROp::MOD:
            generateArithmetic(instruction);
            break;

        case IROp::NEG:
        case IROp::LOGICAL_NOT:
            generateUnary(instruction);
            break;

        case IROp::EQ:
        case IROp::NE:
        case IROp::LT:
        case IROp::GT:
        case IROp::LE:
        case IROp::GE:
            generateComparison(instruction);
            break;

        case IROp::LOGICAL_AND:
        case IROp::LOGICAL_OR:
            generateLogical(instruction);
            break;

        case IROp::LABEL:
            generateLabel(instruction);
            break;

        case IROp::GOTO:
            generateGoto(instruction);
            break;

        case IROp::IF_FALSE_GOTO:
            generateConditionalJump(instruction);
            break;

        case IROp::RETURN:
            generateReturn(instruction);
            break;

        case IROp::PARAM:
            // Incoming parameters were assigned stack
            // locations during allocateStackVariables().
            break;

        case IROp::CALL:
            throw runtime_error(
                "Codegen Error: Function calls are not yet supported"
            );

        case IROp::FUNCTION_BEGIN:
        case IROp::FUNCTION_END:
            // Handled by generate()
            break;

        default:
            throw runtime_error(
                "Codegen Error: Unsupported IR instruction"
            );
    }
}

// Generate assembly for the entire IR program
void CodeGenerator::generate() {
    assembly.clear();
    resetFunctionState();
    internalLabelCounter = 0;

    emit("    .text");

    size_t i = 0;

    while (i < instructions.size()) {
        if (instructions[i].op != IROp::FUNCTION_BEGIN) {
            throw runtime_error(
                "Codegen Error: Expected FUNCTION_BEGIN"
            );
        }

        const string functionName = instructions[i].result;

        if (functionName.empty()) {
            throw runtime_error(
                "Codegen Error: Function name is empty"
            );
        }

        // Locate the matching end of this function
        size_t endIndex = i + 1;

        while (endIndex < instructions.size() &&
               instructions[endIndex].op !=
                   IROp::FUNCTION_END) {

            if (instructions[endIndex].op ==
                IROp::FUNCTION_BEGIN) {
                throw runtime_error(
                    "Codegen Error: Nested FUNCTION_BEGIN"
                );
            }

            ++endIndex;
        }

        if (endIndex == instructions.size()) {
            throw runtime_error(
                "Codegen Error: Missing FUNCTION_END"
            );
        }

        if (instructions[endIndex].result != functionName) {
            throw runtime_error(
                "Codegen Error: Function boundary mismatch"
            );
        }

        resetFunctionState();

        // Determine local stack space
        allocateStackVariables(i + 1, endIndex);

        // Generate prologue
        generateFunctionBegin(functionName);

        // Translate function body
        for (size_t j = i + 1; j < endIndex; ++j) {
            generateInstruction(instructions[j]);
        }

        // Fall-through return value for main
        if (functionName == "main") {
            emit("    movl $0, %eax");
        }

        // Generate epilogue
        generateFunctionEnd();

        i = endIndex + 1;
    }
}

// Return assembly lines
const vector<string>&
CodeGenerator::getAssembly() const {
    return assembly;
}

// Print generated assembly
void CodeGenerator::print() const {
    cout << "\n========== GENERATED x86 ASSEMBLY ==========\n";

    for (const string& line : assembly) {
        cout << line << endl;
    }

    cout << "============================================\n";
}

// Write assembly to a file
bool CodeGenerator::writeToFile(
    const string& filename
) const {
    ofstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    for (const string& line : assembly) {
        file << line << '\n';
    }

    return file.good();
}
