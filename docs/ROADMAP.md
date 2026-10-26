# Development Roadmap

## Milestone 1 — Core Compiler

- [x] Token representation
- [x] Lexer implementation
- [x] Parser implementation
- [x] AST representation
- [x] Symbol table
- [x] Semantic analyzer
- [x] Intermediate code generator
- [x] Basic optimizer
- [x] x86 assembly generator
- [x] Integrated command-line driver
- [ ] Verify end-to-end compilation across representative programs

## Milestone 2 — Testing and Reliability

- [ ] Automated lexer tests
- [ ] Automated parser tests
- [ ] Semantic error test suite
- [ ] IR correctness tests
- [ ] Code generation regression tests
- [ ] Optimization correctness comparisons
- [ ] Improved diagnostic messages

## Milestone 3 — Language Features

- [ ] Built-in integer printing
- [ ] General function calls
- [ ] `printf()` support
- [ ] Arrays
- [ ] Pointers
- [ ] Improved `for` loop support
- [ ] Full floating-point support

## Milestone 4 — Compiler Improvements

- [ ] Control-flow graph construction
- [ ] Data-flow analysis
- [ ] Scope-aware IR
- [ ] Register allocation
- [ ] Additional optimization passes
- [ ] x86-64 backend
- [ ] Cross-platform build configuration

## Long-Term Goal

Develop a reliable, modular educational C compiler that can compile increasingly complex C programs and produce executable machine code on modern systems.
