
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <stdexcept>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "optimizer.h"
#include "codegen.h"

using namespace std;

// Read the complete C source file
string readSourceFile(const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        throw runtime_error(
            "Cannot open source file: " + filename
        );
    }

    stringstream buffer;
    buffer << file.rdbuf();

    if (file.bad()) {
        throw runtime_error(
            "Error reading source file: " + filename
        );
    }

    return buffer.str();
}

// Display command-line usage
void printUsage(const string& programName) {
    cout << "Modular C Compiler\n\n";
    cout << "Usage:\n";
    cout << "  " << programName
         << " <source.c> [-o output.exe] [options]\n\n";

    cout << "Options:\n";
    cout << "  -o <file>       Set output executable filename\n";
    cout << "  -S              Generate assembly only\n";
    cout << "  --no-opt        Disable optimization\n";
    cout << "  --show-tokens   Display lexer tokens\n";
    cout << "  --show-ir       Display original IR\n";
    cout << "  --show-opt      Display optimized IR\n";
    cout << "  --show-asm      Display generated assembly\n";
    cout << "  -h, --help      Show this help message\n";
}

// Quote a filename for shell execution
string quotePath(const string& path) {
    if (path.find('"') != string::npos ||
        path.find('\n') != string::npos ||
        path.find('\r') != string::npos) {
        throw runtime_error(
            "Invalid quote or newline in output path"
        );
    }

    return "\"" + path + "\"";
}

// Replace extension or append a new extension
string replaceExtension(
    const string& filename,
    const string& extension
) {
    size_t separator = filename.find_last_of("/\\");
    size_t dot = filename.find_last_of('.');

    if (dot == string::npos ||
        (separator != string::npos && dot < separator)) {
        return filename + extension;
    }

    return filename.substr(0, dot) + extension;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    string sourceFile;
    string outputFile = "build/program.exe";

    bool assemblyOnly = false;
    bool enableOptimization = true;

    bool showTokens = false;
    bool showIR = false;
    bool showOptimizedIR = false;
    bool showAssembly = false;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-o") {
            if (i + 1 >= argc) {
                cerr << "Error: Missing filename after -o\n";
                return 1;
            }

            outputFile = argv[++i];
        }
        else if (arg == "-S") {
            assemblyOnly = true;
        }
        else if (arg == "--no-opt") {
            enableOptimization = false;
        }
        else if (arg == "--show-tokens") {
            showTokens = true;
        }
        else if (arg == "--show-ir") {
            showIR = true;
        }
        else if (arg == "--show-opt") {
            showOptimizedIR = true;
        }
        else if (arg == "--show-asm") {
            showAssembly = true;
        }
        else if (!arg.empty() && arg[0] == '-') {
            cerr << "Error: Unknown option " << arg << endl;
            return 1;
        }
        else {
            if (!sourceFile.empty()) {
                cerr << "Error: Multiple source files "
                     << "are not supported yet\n";
                return 1;
            }

            sourceFile = arg;
        }
    }

    if (sourceFile.empty()) {
        cerr << "Error: No C source file provided\n";
        return 1;
    }

    try {
        cout << "\n========== MODULAR C COMPILER ==========\n";
        cout << "Input file: " << sourceFile << endl;

        // -----------------------------------
        // Phase 1: Lexical Analysis
        // -----------------------------------
        string source = readSourceFile(sourceFile);

        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();

        cout << "[1/6] Lexical Analysis Successful\n";

        if (showTokens) {
            cout << "\n========== TOKENS ==========\n";

            for (const Token& token : tokens) {
                cout << tokenTypeToString(token.type)
                     << " : " << token.value
                     << " (Line " << token.line
                     << ", Column " << token.column
                     << ")\n";
            }
        }

        // -----------------------------------
        // Phase 2: Syntax Analysis
        // -----------------------------------
        Parser parser(tokens);
        unique_ptr<Program> program = parser.parse();

        cout << "[2/6] Syntax Analysis Successful\n";

        // -----------------------------------
        // Phase 3: Semantic Analysis
        // -----------------------------------
        SemanticAnalyzer semantic;

        if (!semantic.analyze(*program)) {
            semantic.printErrors();
            cerr << "\nCompilation stopped: "
                 << "Semantic analysis failed\n";
            return 1;
        }

        cout << "[3/6] Semantic Analysis Successful\n";

        // -----------------------------------
        // Phase 4: IR Generation
        // -----------------------------------
        IRGenerator irGenerator;
        irGenerator.generate(*program);

        cout << "[4/6] IR Generation Successful\n";

        if (showIR) {
            irGenerator.print();
        }

        // -----------------------------------
        // Phase 5: Code Optimization
        // -----------------------------------
        vector<IRInstruction> finalIR;

        if (enableOptimization) {
            Optimizer optimizer(
                irGenerator.getInstructions()
            );

            optimizer.optimize();

            finalIR = optimizer.getOptimizedInstructions();

            cout << "[5/6] Optimization Successful ("
                 << optimizer.getOptimizationCount()
                 << " transformations)\n";

            if (showOptimizedIR) {
                optimizer.print();
            }
        }
        else {
            finalIR = irGenerator.getInstructions();

            cout << "[5/6] Optimization Skipped\n";

            if (showOptimizedIR) {
                cout << "\nOptimization disabled. "
                     << "IR is unchanged.\n";
            }
        }

        // -----------------------------------
        // Phase 6: Code Generation
        // -----------------------------------
        CodeGenerator codeGenerator(finalIR);
        codeGenerator.generate();

        cout << "[6/6] Code Generation Successful\n";

        if (showAssembly) {
            codeGenerator.print();
        }

        // Determine output assembly filename
        string assemblyFile;

        if (assemblyOnly) {
            assemblyFile =
                replaceExtension(outputFile, ".s");
        }
        else {
            assemblyFile =
                replaceExtension(outputFile, ".s");
        }

        if (!codeGenerator.writeToFile(assemblyFile)) {
            throw runtime_error(
                "Cannot write assembly file: " +
                assemblyFile +
                ". Check that its parent directory exists."
            );
        }

        cout << "\nAssembly file: "
             << assemblyFile << endl;

        // Stop after assembly generation if requested
        if (assemblyOnly) {
            cout << "\nAssembly generation completed.\n";
            return 0;
        }

        // -----------------------------------
        // Assemble and link using MinGW GCC
        // -----------------------------------
        cout << "\n[Link] Invoking GCC...\n";

        string command =
            "gcc " + quotePath(assemblyFile) +
            " -o " + quotePath(outputFile);

        int result = system(command.c_str());

        if (result != 0) {
            cerr << "\nError: GCC assembly/linking failed.\n";
            cerr << "Command: " << command << endl;
            return 1;
        }

        cout << "[Link] Executable Generated Successfully\n";

        cout << "\n========== COMPILATION SUCCESSFUL ==========\n";
        cout << "Source:     " << sourceFile << endl;
        cout << "Assembly:   " << assemblyFile << endl;
        cout << "Executable: " << outputFile << endl;
        cout << "============================================\n";

        return 0;
    }
    catch (const exception& e) {
        cerr << "\nCompilation Error: "
             << e.what() << endl;
        return 1;
    }
}
