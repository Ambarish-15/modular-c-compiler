
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "optimizer.h"

using namespace std;

int main(int argc, char* argv[]) {
    string filename = "tests/basic.c";

    if (argc > 1) {
        filename = argv[1];
    }

    ifstream file(filename);

    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    try {
        Lexer lexer(buffer.str());
        Parser parser(lexer.tokenize());

        unique_ptr<Program> program = parser.parse();

        SemanticAnalyzer semantic;

        if (!semantic.analyze(*program)) {
            semantic.printErrors();
            return 1;
        }

        IRGenerator ir;
        ir.generate(*program);

        cout << "Original IR:";
        ir.print();

        Optimizer optimizer(ir.getInstructions());
        optimizer.optimize();

        optimizer.print();

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
