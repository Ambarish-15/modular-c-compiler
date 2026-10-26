
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "optimizer.h"
#include "codegen.h"

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

        cout << "\nOriginal IR:";
        ir.print();

        Optimizer optimizer(ir.getInstructions());
        optimizer.optimize();

        cout << "\nOptimized IR:";
        optimizer.print();

        CodeGenerator codegen(
            optimizer.getOptimizedInstructions()
        );

        codegen.generate();

        cout << "\nGenerated Assembly:";
        codegen.print();

        if (!codegen.writeToFile("build/output.s")) {
            cerr << "Failed to write assembly file" << endl;
            return 1;
        }

        cout << "\nAssembly saved to build/output.s\n";

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
