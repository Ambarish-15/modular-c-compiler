
#include "symbol_table.h"

#include <iostream>
#include <stdexcept>

using namespace std;

// Constructor: initialize global scope
SymbolTable::SymbolTable() {
    scopes.emplace_back();
}

// Enter a new nested scope
void SymbolTable::enterScope() {
    scopes.emplace_back();
}

// Exit the current scope
void SymbolTable::exitScope() {
    if (scopes.size() <= 1) {
        throw runtime_error(
            "Cannot exit the global scope"
        );
    }

    scopes.pop_back();
}

// Declare a symbol in the current scope
bool SymbolTable::declare(const Symbol& symbol) {
    auto& currentScope = scopes.back();

    // Check for duplicate declaration
    if (currentScope.find(symbol.name) != currentScope.end()) {
        return false;
    }

    currentScope.emplace(symbol.name, symbol);
    return true;
}

// Search only the current scope
const Symbol* SymbolTable::lookupCurrentScope(
    const string& name
) const {
    const auto& currentScope = scopes.back();

    auto it = currentScope.find(name);

    if (it != currentScope.end()) {
        return &it->second;
    }

    return nullptr;
}

// Search from inner scope to outer scope
const Symbol* SymbolTable::lookup(
    const string& name
) const {
    for (auto it = scopes.rbegin();
         it != scopes.rend(); ++it) {

        auto found = it->find(name);

        if (found != it->end()) {
            return &found->second;
        }
    }

    return nullptr;
}

// Return current scope depth
size_t SymbolTable::getScopeDepth() const {
    return scopes.size() - 1;
}

// Check whether current scope is global
bool SymbolTable::isGlobalScope() const {
    return scopes.size() == 1;
}

// Print the complete active symbol table
void SymbolTable::print() const {
    cout << "\n========== SYMBOL TABLE ==========\n";

    for (size_t i = 0; i < scopes.size(); i++) {
        cout << "\nScope Level: " << i << endl;

        cout << "----------------------------------\n";

        for (const auto& entry : scopes[i]) {
            const Symbol& symbol = entry.second;

            cout << "Name: " << symbol.name << endl;
            cout << "Type: " << symbol.type << endl;

            cout << "Kind: ";

            switch (symbol.kind) {
                case SymbolKind::VARIABLE:
                    cout << "Variable";
                    break;

                case SymbolKind::FUNCTION:
                    cout << "Function";
                    break;

                case SymbolKind::PARAMETER:
                    cout << "Parameter";
                    break;
            }

            cout << endl;

            if (symbol.kind == SymbolKind::FUNCTION) {
                cout << "Parameters: ";

                if (symbol.parameterTypes.empty()) {
                    cout << "None";
                } else {
                    for (size_t j = 0;
                         j < symbol.parameterTypes.size(); j++) {

                        cout << symbol.parameterTypes[j];

                        if (j + 1 < symbol.parameterTypes.size()) {
                            cout << ", ";
                        }
                    }
                }

                cout << endl;
            }

            cout << "----------------------------------\n";
        }
    }

    cout << "==================================\n";
}
