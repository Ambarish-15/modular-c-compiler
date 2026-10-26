#include "optimizer.h"

int main() {
    IRInstruction instruction(
        IROp::ADD,
        "__t1",
        "10",
        "20"
    );

    std::vector<IRInstruction> instructions;
    instructions.push_back(instruction);

    return instructions.empty() ? 1 : 0;
}