# Known Limitations

## Language Features

The compiler currently supports a restricted subset of C, primarily for educational demonstrations.

Not yet supported:

- Standard library function calls such as `printf()` and `scanf()`
- C preprocessor directives
- Pointers and arrays
- Structures and unions
- Dynamic memory allocation
- Complete function-call support
- Full floating-point execution
- All standard loop and control-flow constructs

## Intermediate Representation

The IR represents operands and variable names as strings. Scope-aware renaming is not yet implemented, so variable shadowing may cause incorrect results.

## Optimization

The optimizer includes basic constant folding, constant propagation, algebraic simplification, and limited dead-code elimination.

The implementation does not yet perform full control-flow or liveness analysis. Some transformations may be unsafe across loops and branches.

Use `--no-opt` when debugging unexpected output.

## Backend

The code generator targets 32-bit Windows x86 assembly.

It does not currently provide:

- Native x86-64 assembly
- Complete function-call ABI support
- Floating-point machine instructions
- Register allocation
- Cross-platform code generation

## Testing and Reliability

The compiler is under active development. Individual modules and the complete compilation pipeline require systematic regression testing.

Unsupported features and incorrect source programs may produce compilation errors. Some valid C programs may also be rejected because they fall outside the supported subset.
