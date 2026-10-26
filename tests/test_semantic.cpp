#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<memory>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"

using namespace std;

int main(int argc, char* argv[]){
    string filename="tests/basic.c";

    if(argc>1){
        filename=argv[1];

    }
    ifstream file(filename);

    if(!file){
        cerr<<"Cannot open file: "<<filename<<endl;
        return 1;
    }
    stringstream buffer;
    buffer<<file.rdbuf();

    try{
        Lexer lexer(buffer.str());
        vector<Token> tokens=lexer.tokenize();

        Parser parser(tokens);
        unique_ptr<Program> program=parser.parse();

        cout<<"Lexical Analysis Successful!"<<endl;
        cout<<"Syntax Analysis Successful!"<<endl;

        SemanticAnalyzer semantic;

        bool success=semantic.analyze(*program);
        semantic.printErrors();

        return success?0:1;
    }
    catch(const exception& e){
        cerr<<e.what()<<endl;
        return 1;
    }
}