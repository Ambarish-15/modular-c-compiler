# Modular C Compiler Using C++

A modular compiler implemented in C++17 for a subset of the C programming language. The project demonstrates source-code analysis, Abstract Syntax Tree construction, semantic validation, intermediate representation, optimization, and x86 assembly generation.

**Developer:** Ambarish Mondal  
**Institution:** Vellore Institute of Technology, Vellore  
**Project:** Compiler Design

## Overview

The compiler reads a C source file, processes it through independently implemented C++ modules, and generates 32-bit Windows x86 assembly. It can then invoke GCC to assemble and link the output into an executable.

The project is educational and is not intended to replace GCC or Clang.

## Architecture

The compiler uses `main.cpp` as its central controller. Each module receives an in-memory representation, processes it, and returns or stores the representation needed by the next module.

```text
User invokes mycompiler.exe
          |
          v
       main.cpp
          |
          v
Read C file into std::string
          |
          v
Lexer::tokenize()
          |
          v
std::vector<Token>
          |
          v
Parser::parse()
          |
          v
std::unique_ptr<Program> (AST)
          |
          v
SemanticAnalyzer::analyze()
          |
          +-- Errors detected --> Stop
          |
          v
IRGenerator::generate()
          |
          v
std::vector<IRInstruction>
          |
          v
Optimizer::optimize() [optional]
          |
          v
Optimized IR
          |
          v
CodeGenerator::generate()
          |
          v
Assembly instructions
          |
          v
build/program.s
          |
          v
GCC assembler and linker
          |
          v
build/program.exe
```

## Technologies

- C++17
- Standard Template Library
- Recursive-descent parsing
- Abstract Syntax Trees
- Three-address intermediate code
- GNU Compiler Collection
- 32-bit x86 assembly, GNU assembler syntax
- Visual Studio Code
- Git and GitHub

## Prerequisites

For the current Windows build:

- Windows 10 or Windows 11
- A C++17-compatible compiler
- GCC and G++ available in PATH
- A compatible 32-bit MinGW toolchain for generated assembly
- PowerShell or a compatible terminal

Verify the installation:

```powershell
g++ --version
gcc --version
g++ -dumpmachine
```

The target architecture must be compatible with the generated 32-bit Windows assembly.

## Installation

Clone the repository:

```powershell
git clone https://github.com/YOUR_USERNAME/modular-c-compiler.git
cd modular-c-compiler
```

Replace `YOUR_USERNAME` with your actual GitHub username.

Create the build directory:

```powershell
New-Item -ItemType Directory -Force build
```

Compile the compiler:

```powershell
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/lexer.cpp src/parser.cpp src/symbol_table.cpp src/semantic.cpp src/ir.cpp src/optimizer.cpp src/codegen.cpp -o build/mycompiler.exe
```

## Running the Compiler

Compile a C source file:

```powershell
.\build\mycompiler.exe tests\basic.c --no-opt -o build\program.exe
```

Run the generated executable:

```powershell
.\build\program.exe
echo "Program returned: $LASTEXITCODE"
```

For this example:

```c
int main() {
    int a = 10;
    int b = 20;
    int sum = a + b;
    return sum;
}
```

The expected return value is `30`.

The example does not print anything to standard output. The value is returned through the process exit code.

**Important:** Running `program.exe` does not automatically recompile `basic.c`. Invoke `mycompiler.exe` again after editing the source file.

## Command-Line Options

| Option | Description |
|---|---|
| `-o <file>` | Specify the output file |
| `-S` | Generate assembly without linking |
| `--no-opt` | Disable optimization |
| `--show-tokens` | Display lexer output |
| `--show-ir` | Display original intermediate code |
| `--show-opt` | Display optimized intermediate code |
| `--show-asm` | Display generated assembly |

Generate assembly only:

```powershell
.\build\mycompiler.exe tests\basic.c --no-opt -S -o build\basic.s
```

Inspect intermediate representations:

```powershell
.\build\mycompiler.exe tests\basic.c --show-tokens --show-ir --show-opt --show-asm -S -o build\basic.s
```

## Testing

The modules can be tested individually using their test drivers.

### Lexer

```powershell
g++ -std=c++17 -Iinclude tests/test_lexer.cpp src/lexer.cpp -o build/test_lexer.exe
.\build\test_lexer.exe tests\basic.c
```

### Parser and AST

```powershell
g++ -std=c++17 -Iinclude tests/test_parser_ast.cpp src/lexer.cpp src/parser.cpp -o build/test_parser_ast.exe
.\build\test_parser_ast.exe tests\basic.c
```

These commands require the corresponding test-driver files to be present in `tests/`.

### Full compiler

```powershell
.\build\mycompiler.exe tests\basic.c --no-opt -o build\program.exe
```

Run the generated executable only after confirming compilation succeeded.

Suggested test cases include valid integer programs, arithmetic expressions, conditional statements, loops, missing semicolons, undeclared variables, and duplicate declarations.

## Current Functionality

The project contains implementations for:

- Lexical analysis with token position tracking
- Recursive-descent parsing
- AST construction
- Variable and function declarations
- Basic arithmetic and logical expressions
- Conditional statements and loops
- Symbol-table-based semantic checking
- Three-address intermediate representation
- Constant folding and constant propagation
- Basic algebraic simplification
- Limited dead-code and unreachable-code elimination
- 32-bit x86 assembly generation for supported integer operations

Implementation does not imply that every combination of these features has been verified end to end.

## Known Limitations

### Language support

- Supports only a subset of C.
- Preprocessor directives such as `#include` are unsupported.
- `printf()` and general function calls are unsupported.
- Arrays, pointers, structures, and unions are unsupported.
- Full floating-point code generation is unavailable.
- Conventional `for` loop increment expressions have limited support.
- Dynamic memory allocation is unavailable.

### Compiler correctness

- Semantic analysis is incomplete compared with a production compiler.
- The IR uses string-based variable identifiers, which can cause problems with variable shadowing.
- Optimization is not fully control-flow-aware and may be incorrect for certain loops and branches.
- Some unsupported constructs may fail during parsing or code generation.
- Error recovery and diagnostics are limited.

### Platform restrictions

- Generated assembly targets 32-bit Windows x86.
- Native 64-bit code generation is not implemented.
- The backend is not yet portable across operating systems.
- Linking depends on an external compatible GCC toolchain.

## Future Improvements

1. Implement built-in integer output, followed by `printf()` support.
2. Add function calls, argument passing, and return-value handling.
3. Support arrays, pointers, and more C language constructs.
4. Implement proper control-flow graphs and data-flow analysis.
5. Improve constant propagation and dead-code elimination.
6. Add complete type checking and scope-aware IR names.
7. Implement 64-bit x86 code generation.
8. Add automated unit tests and regression tests.
9. Improve error messages with source-code locations.
10. Introduce cross-platform build support using CMake.

## Project Status

The major compiler modules have been implemented. Full end-to-end testing, toolchain compatibility checks, and regression testing are ongoing.

This is an educational compiler under active development.

## License

See the `LICENSE` file for the applicable license, if one is included.
