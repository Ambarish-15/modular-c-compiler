
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

// Categories of identifiers
enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER
};

// Information stored for each identifier
struct Symbol {
    std::string name;
    std::string type;
    SymbolKind kind;

    // Used when the symbol represents a function
    std::vector<std::string> parameterTypes;

    Symbol(
        const std::string& name,
        const std::string& type,
        SymbolKind kind,
        const std::vector<std::string>& parameterTypes = {}
    )
        : name(name),
          type(type),
          kind(kind),
          parameterTypes(parameterTypes) {}
};

class SymbolTable {
private:
    // Each map represents one scope
    std::vector<std::unordered_map<std::string, Symbol>> scopes;

public:
    // Create a symbol table with a global scope
    SymbolTable();

    // Create a new nested scope
    void enterScope();

    // Leave the current scope
    void exitScope();

    // Add a symbol to the current scope
    // Returns false if the name already exists there
    bool declare(const Symbol& symbol);

    // Search the current scope only
    const Symbol* lookupCurrentScope(
        const std::string& name
    ) const;

    // Search from the innermost scope outward
    const Symbol* lookup(
        const std::string& name
    ) const;

    // Get current scope depth (global scope = 0)
    std::size_t getScopeDepth() const;

    // Check whether the table is at global scope
    bool isGlobalScope() const;

    // Display symbols for debugging
    void print() const;
};

#endif
