
#include <iostream>
#include <memory>
#include <vector>
#include "ast.h"

using namespace std;

int main() {
    // Create the root AST node
    auto program = make_unique<Program>();

    // Create the body of main()
    auto body = make_unique<BlockStatement>();

    // Create: int a = 10;
    body->addStatement(
        make_unique<VariableDeclaration>(
            "int",
            "a",
            make_unique<IntegerLiteral>("10")
        )
    );

    // Create: return a;
    body->addStatement(
        make_unique<ReturnStatement>(
            make_unique<Identifier>("a")
        )
    );

    // Create: int main() { ... }
    program->addFunction(
        make_unique<FunctionDeclaration>(
            "int",
            "main",
            vector<Parameter>{},
            move(body)
        )
    );

    // Verify AST structure
    cout << "AST created successfully!" << endl;
    cout << "Functions: " << program->functions.size() << endl;
    cout << "Function name: " << program->functions[0]->name << endl;
    cout << "Return type: " << program->functions[0]->returnType << endl;
    cout << "Statements: "
         << program->functions[0]->body->statements.size()
         << endl;

    return 0;
}
