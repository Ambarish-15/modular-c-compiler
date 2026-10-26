
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "lexer.h"
#include "parser.h"

using namespace std;

int main() {
    try {
        ifstream file("tests/basic.c");

        if (!file) {
            cerr << "Cannot open tests/basic.c" << endl;
            return 1;
        }

        stringstream buffer;
        buffer << file.rdbuf();

        Lexer lexer(buffer.str());
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        unique_ptr<Program> program = parser.parse();

        cout << "Parsing successful!" << endl;
        cout << "Functions: "
             << program->functions.size() << endl;

        for (const auto& function : program->functions) {
            cout << "Function: " << function->name << endl;
            cout << "Return type: "
                 << function->returnType << endl;
            cout << "Statements: "
                 << function->body->statements.size()
                 << endl;
        }

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
