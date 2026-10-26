
#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

#include "ir.h"

// Optimizes three-address intermediate code
class Optimizer {
private:
    // Original IR instructions
    std::vector<IRInstruction> instructions;

    // Track known constant values
    std::unordered_map<std::string, std::string> constants;

    // Track optimization statistics
    std::size_t optimizationsPerformed;

    // Check whether an operand is a numeric constant
    bool isConstant(const std::string& operand) const;

    // Check whether an operand represents a temporary
    bool isTemporary(const std::string& operand) const;

    // Evaluate constant arithmetic and comparisons
    bool evaluateConstantExpression(
        IROp op,
        const std::string& arg1,
        const std::string& arg2,
        std::string& result
    ) const;

    // Optimization passes
    bool constantFolding();

    bool constantPropagation();

    bool algebraicSimplification();

    bool deadCodeElimination();

    bool unreachableCodeElimination();

    // Identify whether an instruction defines a value
    bool definesVariable(
        const IRInstruction& instruction
    ) const;

    // Identify variables read by an instruction
    std::vector<std::string> getUsedVariables(
        const IRInstruction& instruction
    ) const;

    // Determine whether an instruction has side effects
    bool hasSideEffects(
        const IRInstruction& instruction
    ) const;

    // Clear known constants when control flow changes
    void clearConstants();

public:
    // Construct an empty optimizer
    Optimizer();

    // Construct optimizer with generated IR
    explicit Optimizer(
        const std::vector<IRInstruction>& ir
    );

    // Load or replace instructions
    void setInstructions(
        const std::vector<IRInstruction>& ir
    );

    // Execute optimization passes
    void optimize();

    // Retrieve optimized IR
    const std::vector<IRInstruction>&
    getOptimizedInstructions() const;

    // Number of transformations performed
    std::size_t getOptimizationCount() const;

    // Print optimized three-address code
    void print() const;
};

#endif
