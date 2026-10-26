#include "symbol_table.h"

int main() {
    Symbol variable("a", "int", SymbolKind::VARIABLE);
    return variable.name.empty() ? 1 : 0;
}