
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "lexer.h"
#include "token.h"

using namespace std;

int main(int argc, char* argv[]) {
    string filename = "tests/basic.c";

    if (argc > 1) {
        filename = argv[1];
    }

    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Cannot open " << filename << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    try {
        Lexer lexer(buffer.str());
        vector<Token> tokens = lexer.tokenize();

        cout << "Lexical Analysis Successful!\n\n";

        cout << "TOKEN TYPE\t\tVALUE\t\tLINE\tCOLUMN\n";
        cout << "--------------------------------------------------\n";

        for (const Token& token : tokens) {
            cout << tokenTypeToString(token.type)
                 << "\t\t" << token.value
                 << "\t\t" << token.line
                 << "\t" << token.column
                 << endl;
        }

        cout << "\nTotal Tokens: " << tokens.size() << endl;
    }
    catch (const exception& e) {
        cerr << "Lexical Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
