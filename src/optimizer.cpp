
#include "optimizer.h"

#include <iostream>
#include <climits>
#include <cctype>
#include <stdexcept>
#include <utility>

using namespace std;

// Constructor
Optimizer::Optimizer() : optimizationsPerformed(0) {}

// Constructor with IR
Optimizer::Optimizer(const vector<IRInstruction>& ir)
    : instructions(ir), optimizationsPerformed(0) {}

// Load IR instructions
void Optimizer::setInstructions(
    const vector<IRInstruction>& ir
) {
    instructions = ir;
    constants.clear();
    optimizationsPerformed = 0;
}

// Check whether a string is an integer constant
bool Optimizer::isConstant(const string& operand) const {
    if (operand.empty()) {
        return false;
    }

    size_t i = 0;

    if (operand[0] == '-' || operand[0] == '+') {
        i = 1;
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

    return true;
}

// Check whether an identifier is a generated temporary
bool Optimizer::isTemporary(
    const string& operand
) const {
    return operand.size() > 3 &&
           operand.compare(0, 3, "__t") == 0;
}

// Evaluate integer constant expressions
bool Optimizer::evaluateConstantExpression(
    IROp op,
    const string& arg1,
    const string& arg2,
    string& result
) const {
    if (!isConstant(arg1) || !isConstant(arg2)) {
        return false;
    }

    try {
        long long a = stoll(arg1);
        long long b = stoll(arg2);

        // Restrict folding to signed 32-bit integers.
        if (a < INT_MIN || a > INT_MAX ||
            b < INT_MIN || b > INT_MAX) {
            return false;
        }

        long long value = 0;

        switch (op) {
            case IROp::ADD:
                value = a + b;
                break;

            case IROp::SUB:
                value = a - b;
                break;

            case IROp::MUL:
                value = a * b;
                break;

            case IROp::DIV:
                if (b == 0 ||
                    (a == INT_MIN && b == -1)) {
                    return false;
                }
                value = a / b;
                break;

            case IROp::MOD:
                if (b == 0 ||
                    (a == INT_MIN && b == -1)) {
                    return false;
                }
                value = a % b;
                break;

            case IROp::EQ:
                value = (a == b);
                break;

            case IROp::NE:
                value = (a != b);
                break;

            case IROp::LT:
                value = (a < b);
                break;

            case IROp::GT:
                value = (a > b);
                break;

            case IROp::LE:
                value = (a <= b);
                break;

            case IROp::GE:
                value = (a >= b);
                break;

            case IROp::LOGICAL_AND:
                value = (a != 0 && b != 0);
                break;

            case IROp::LOGICAL_OR:
                value = (a != 0 || b != 0);
                break;

            default:
                return false;
        }

        if (value < INT_MIN || value > INT_MAX) {
            return false;
        }

        result = to_string(value);
        return true;
    }
    catch (const exception&) {
        return false;
    }
}

// Determine whether an instruction defines a variable
bool Optimizer::definesVariable(
    const IRInstruction& instruction
) const {
    switch (instruction.op) {
        case IROp::ASSIGN:
        case IROp::ADD:
        case IROp::SUB:
        case IROp::MUL:
        case IROp::DIV:
        case IROp::MOD:
        case IROp::NEG:
        case IROp::LOGICAL_NOT:
        case IROp::EQ:
        case IROp::NE:
        case IROp::LT:
        case IROp::GT:
        case IROp::LE:
        case IROp::GE:
        case IROp::LOGICAL_AND:
        case IROp::LOGICAL_OR:
        case IROp::CALL:
            return !instruction.result.empty();

        default:
            return false;
    }
}

// Find variables read by an instruction
vector<string> Optimizer::getUsedVariables(
    const IRInstruction& instruction
) const {
    vector<string> used;

    auto addOperand = [&](const string& operand) {
        if (!operand.empty() &&
            !isConstant(operand) &&
            (isalpha(static_cast<unsigned char>(operand[0])) ||
             operand[0] == '_')) {
            used.push_back(operand);
        }
    };

    switch (instruction.op) {
        case IROp::ASSIGN:
        case IROp::NEG:
        case IROp::LOGICAL_NOT:
        case IROp::IF_FALSE_GOTO:
        case IROp::RETURN:
            addOperand(instruction.arg1);
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
            addOperand(instruction.arg1);
            addOperand(instruction.arg2);
            break;

        default:
            break;
    }

    return used;
}

// Check for operations that must be preserved
bool Optimizer::hasSideEffects(
    const IRInstruction& instruction
) const {
    switch (instruction.op) {
        case IROp::CALL:
        case IROp::RETURN:
        case IROp::GOTO:
        case IROp::IF_FALSE_GOTO:
        case IROp::LABEL:
        case IROp::FUNCTION_BEGIN:
        case IROp::FUNCTION_END:
        case IROp::PARAM:
        case IROp::DIV:
        case IROp::MOD:
            return true;

        default:
            return false;
    }
}

// Reset constant information
void Optimizer::clearConstants() {
    constants.clear();
}

// Replace operands with known constants
bool Optimizer::constantPropagation() {
    bool changed = false;
    clearConstants();

    for (auto& instruction : instructions) {
        // Never propagate constants across basic-block boundaries
        if (instruction.op == IROp::LABEL ||
            instruction.op == IROp::FUNCTION_BEGIN ||
            instruction.op == IROp::FUNCTION_END) {
            clearConstants();
            continue;
        }

        // Incoming parameters invalidate any matching mapping
        if (instruction.op == IROp::PARAM) {
            constants.erase(instruction.result);
            continue;
        }

        auto replaceOperand = [&](string& operand) {
            auto it = constants.find(operand);

            if (it != constants.end()) {
                operand = it->second;
                ++optimizationsPerformed;
                changed = true;
            }
        };

        switch (instruction.op) {
            case IROp::ASSIGN:
            case IROp::NEG:
            case IROp::LOGICAL_NOT:
            case IROp::IF_FALSE_GOTO:
            case IROp::RETURN:
                replaceOperand(instruction.arg1);
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
                replaceOperand(instruction.arg1);
                replaceOperand(instruction.arg2);
                break;

            default:
                break;
        }

        if (definesVariable(instruction)) {
            constants.erase(instruction.result);

            if (instruction.op == IROp::ASSIGN &&
                isConstant(instruction.arg1)) {
                constants[instruction.result] =
                    instruction.arg1;
            }
        }

        // A call may modify values not modeled by this IR
        if (instruction.op == IROp::CALL ||
            instruction.op == IROp::GOTO ||
            instruction.op == IROp::IF_FALSE_GOTO ||
            instruction.op == IROp::RETURN) {
            clearConstants();
        }
    }

    return changed;
}

// Fold expressions with constant operands
bool Optimizer::constantFolding() {
    bool changed = false;

    for (auto& instruction : instructions) {
        string value;

        if (evaluateConstantExpression(
                instruction.op,
                instruction.arg1,
                instruction.arg2,
                value)) {

            instruction.op = IROp::ASSIGN;
            instruction.arg1 = value;
            instruction.arg2.clear();

            ++optimizationsPerformed;
            changed = true;
            continue;
        }

        // Unary operations
        if (isConstant(instruction.arg1)) {
            try {
                long long number = stoll(instruction.arg1);

                if (number < INT_MIN || number > INT_MAX) {
                    continue;
                }

                if (instruction.op == IROp::NEG) {
                    if (number == INT_MIN) {
                        continue;
                    }
                    value = to_string(-number);
                }
                else if (instruction.op == IROp::LOGICAL_NOT) {
                    value = number == 0 ? "1" : "0";
                }
                else {
                    continue;
                }

                instruction.op = IROp::ASSIGN;
                instruction.arg1 = value;
                instruction.arg2.clear();

                ++optimizationsPerformed;
                changed = true;
            }
            catch (const exception&) {
                // Leave out-of-range constants unchanged
            }
        }
    }

    return changed;
}

// Simplify expressions using algebraic identities
bool Optimizer::algebraicSimplification() {
    bool changed = false;

    for (auto& instruction : instructions) {
        string replacement;
        bool simplify = false;

        switch (instruction.op) {
            case IROp::ADD:
                if (instruction.arg2 == "0") {
                    replacement = instruction.arg1;
                    simplify = true;
                }
                else if (instruction.arg1 == "0") {
                    replacement = instruction.arg2;
                    simplify = true;
                }
                break;

            case IROp::SUB:
                if (instruction.arg2 == "0") {
                    replacement = instruction.arg1;
                    simplify = true;
                }
                break;

            case IROp::MUL:
                if (instruction.arg2 == "1") {
                    replacement = instruction.arg1;
                    simplify = true;
                }
                else if (instruction.arg1 == "1") {
                    replacement = instruction.arg2;
                    simplify = true;
                }
                break;

            case IROp::DIV:
                if (instruction.arg2 == "1") {
                    replacement = instruction.arg1;
                    simplify = true;
                }
                break;

            default:
                break;
        }

        if (simplify) {
            instruction.op = IROp::ASSIGN;
            instruction.arg1 = replacement;
            instruction.arg2.clear();

            ++optimizationsPerformed;
            changed = true;
        }
    }

    return changed;
}

// Remove unused generated temporary calculations
bool Optimizer::deadCodeElimination() {
    bool anyChanged = false;
    bool changed;

    do {
        changed = false;

        unordered_set<string> used;

        // Collect all variable references
        for (const auto& instruction : instructions) {
            for (const auto& name :
                 getUsedVariables(instruction)) {
                used.insert(name);
            }
        }

        vector<IRInstruction> result;

        for (const auto& instruction : instructions) {
            if (definesVariable(instruction) &&
                isTemporary(instruction.result) &&
                used.find(instruction.result) == used.end() &&
                !hasSideEffects(instruction)) {

                ++optimizationsPerformed;
                changed = true;
                anyChanged = true;
                continue;
            }

            result.push_back(instruction);
        }

        instructions = move(result);

    } while (changed);

    return anyChanged;
}

// Remove instructions following unconditional jumps
// until a label or function boundary is reached
bool Optimizer::unreachableCodeElimination() {
    bool changed = false;
    bool unreachable = false;

    vector<IRInstruction> result;

    for (const auto& instruction : instructions) {
        if (instruction.op == IROp::LABEL ||
            instruction.op == IROp::FUNCTION_BEGIN ||
            instruction.op == IROp::FUNCTION_END) {
            unreachable = false;
        }

        if (unreachable) {
            ++optimizationsPerformed;
            changed = true;
            continue;
        }

        result.push_back(instruction);

        if (instruction.op == IROp::RETURN ||
            instruction.op == IROp::GOTO) {
            unreachable = true;
        }
    }

    instructions = move(result);
    return changed;
}

// Run optimization passes
void Optimizer::optimize() {
    optimizationsPerformed = 0;
    clearConstants();

    // Remove unreachable instructions first
    unreachableCodeElimination();

    // Repeatedly apply local optimizations
    // until no more changes are found
    bool changed;
    int passes = 0;
    const int maxPasses = 10;

    do {
        changed = false;

        changed |= constantPropagation();
        changed |= constantFolding();
        changed |= algebraicSimplification();

        ++passes;
    } while (changed && passes < maxPasses);

    // Remove unused temporary computations
    deadCodeElimination();

    // Clean up any unreachable instructions
    unreachableCodeElimination();
}

// Retrieve optimized instructions
const vector<IRInstruction>&
Optimizer::getOptimizedInstructions() const {
    return instructions;
}

// Retrieve optimization count
size_t Optimizer::getOptimizationCount() const {
    return optimizationsPerformed;
}

// Print optimized intermediate code
void Optimizer::print() const {
    cout << "\n========== OPTIMIZED THREE ADDRESS CODE ==========\n";

    for (size_t i = 0; i < instructions.size(); ++i) {
        cout << i + 1 << ": "
             << irInstructionToString(instructions[i])
             << endl;
    }

    cout << "==================================================\n";
    cout << "Optimizations performed: "
         << optimizationsPerformed << endl;
}
