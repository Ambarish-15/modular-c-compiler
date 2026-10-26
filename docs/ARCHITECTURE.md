# Project Architecture

## Design

The project uses a centralized controller architecture. `main.cpp` coordinates the execution of independent compiler modules.

Each module communicates through C++ objects and data structures rather than directly launching the next module.

## Data Flow

```text
C source file
    |
    v
File reader
    |
    v
std::string
    |
    v
Lexer
    |
    v
std::vector<Token>
    |
    v
Parser
    |
    v
std::unique_ptr<Program>
    |
    v
Semantic Analyzer + Symbol Table
    |
    v
Validated AST
    |
    v
IR Generator
    |
    v
std::vector<IRInstruction>
    |
    v
Optimizer (optional)
    |
    v
Optimized IR
    |
    v
Code Generator
    |
    v
Assembly instruction strings
    |
    v
Assembly file (.s)
    |
    v
External GCC toolchain
    |
    v
Executable (.exe)
```

## Controller Responsibilities

`main.cpp` is responsible for:

1. Reading command-line arguments.
2. Opening the source file.
3. Creating and invoking the Lexer.
4. Passing tokens to the Parser.
5. Invoking semantic validation.
6. Passing the AST to the IR Generator.
7. Optionally optimizing the generated IR.
8. Passing the final IR to the Code Generator.
9. Writing the assembly file.
10. Invoking GCC for assembly and linking.
11. Reporting errors and returning the final compilation status.

## Module Interfaces

| Module | Input | Output |
|---|---|---|
| Lexer | `std::string` | `std::vector<Token>` |
| Parser | Token vector | `std::unique_ptr<Program>` |
| Semantic Analyzer | AST reference | Validation status and errors |
| IR Generator | AST reference | `std::vector<IRInstruction>` |
| Optimizer | IR vector | Optimized IR vector |
| Code Generator | IR vector | Assembly instructions |
| File Writer | Assembly instructions | `.s` file |
| GCC | Assembly file | Executable |

## Error Handling

Lexical and syntax errors stop compilation before semantic analysis.

Semantic errors are collected and reported before IR generation.

Code generation or linking errors prevent successful completion of the build.

The implementation does not yet provide comprehensive error recovery.
