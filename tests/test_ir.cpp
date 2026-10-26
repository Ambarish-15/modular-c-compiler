
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"

using namespace std;

int main(int argc, char* argv[]) {
    string filename = "tests/basic.c";

    if (argc > 1) {
        filename = argv[1];
    }

    ifstream file(filename);

    if (!file) {
        cerr << "Cannot open file: "
             << filename << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    try {
        // Lexical analysis
        Lexer lexer(buffer.str());
        vector<Token> tokens = lexer.tokenize();

        // Syntax analysis
        Parser parser(tokens);
        unique_ptr<Program> program = parser.parse();

        // Semantic analysis
        SemanticAnalyzer semantic;

        if (!semantic.analyze(*program)) {
            semantic.printErrors();
            return 1;
        }

        // Intermediate code generation
        IRGenerator ir;
        ir.generate(*program);

        cout << "Lexical Analysis Successful!\n";
        cout << "Syntax Analysis Successful!\n";
        cout << "Semantic Analysis Successful!\n";
        cout << "IR Generation Successful!\n";

        ir.print();

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
