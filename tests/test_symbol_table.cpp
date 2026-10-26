
#include <iostream>
#include "symbol_table.h"

using namespace std;

int main() {
    SymbolTable table;

    // Declare a function in global scope
    table.declare(
        Symbol("main", "int", SymbolKind::FUNCTION)
    );

    // Enter function scope
    table.enterScope();

    // Declare two variables
    table.declare(
        Symbol("a", "int", SymbolKind::VARIABLE)
    );

    table.declare(
        Symbol("b", "float", SymbolKind::VARIABLE)
    );

    // Test duplicate declaration
    bool inserted = table.declare(
        Symbol("a", "int", SymbolKind::VARIABLE)
    );

    cout << "Duplicate declaration accepted: "
         << (inserted ? "Yes" : "No") << endl;

    // Test variable lookup
    const Symbol* symbol = table.lookup("a");

    if (symbol != nullptr) {
        cout << "Found variable: "
             << symbol->name << endl;

        cout << "Data type: "
             << symbol->type << endl;
    }

    // Enter nested scope
    table.enterScope();

    table.declare(
        Symbol("c", "char", SymbolKind::VARIABLE)
    );

    cout << "Current scope level: "
         << table.getScopeDepth() << endl;

    // Display all active scopes
    table.print();

    // Exit nested scope
    table.exitScope();

    // Check whether c is still accessible
    if (table.lookup("c") == nullptr) {
        cout << "Variable c is out of scope" << endl;
    }

    return 0;
}
