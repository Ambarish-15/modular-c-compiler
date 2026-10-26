#include "ir.h"
int main(){
    IRInstruction instruction(
        IROp::ADD,
        "t1",
        "a",
        "b"
    );
    return instruction.result=="t1"?0:1;
}