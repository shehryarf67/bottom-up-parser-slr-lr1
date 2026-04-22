# CS4031 Compiler Construction Assignment 03

## Team Members
- Shehryar Faisal (23I-0604)
- Zaki Nabeel (23I-0508)

## Programming Language
C++17

## Project Overview
This project implements a generic bottom-up parser framework for:
- SLR(1)
- LR(1)

The program reads a grammar from a text file, augments it, builds canonical LR(0) and LR(1) item collections, generates parsing tables, parses input strings using shift-reduce parsing, reports conflicts, generates text-based parse trees, and produces comparison output files.

## Build Instructions
### Using Makefile
```bash
make
```

### Manual Compilation
```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o parser
```

## Execution Instructions
### Generate all deliverables for a grammar and an input file
```bash
./parser all input/grammar2.txt input/grammar2_valid.txt output/run_expr
```

### Run SLR(1) only
```bash
./parser slr input/grammar2.txt input/grammar2_valid.txt output/slr_only
```

### Run LR(1) only
```bash
./parser lr1 input/grammar2.txt input/grammar2_valid.txt output/lr1_only
```

### Generate only comparison-oriented outputs
```bash
./parser compare input/grammar3.txt input/grammar3_valid.txt output/conflict_demo
```

## Output Files Produced
The program writes these files into the selected output directory:
- augmented_grammar.txt
- slr_items.txt
- slr_parsing_table.txt
- slr_trace.txt
- lr1_items.txt
- lr1_parsing_table.txt
- lr1_trace.txt
- comparison.txt
- parse_trees.txt

## Grammar File Format
- One production per line
- Format: `NonTerminal -> production1 | production2 | ...`
- Use spaces between grammar symbols
- Non-terminals should be uppercase-leading names like `Expr`, `Term`, `Factor`, `Stmt`
- Epsilon can be written as `epsilon` or `@`

Example:
```text
Expr -> Expr + Term | Term
Term -> Term * Factor | Factor
Factor -> ( Expr ) | id
```

## Input String File Format
- One input string per line
- Tokens must be space-separated
- Empty line is treated as empty input

Example:
```text
id + id * id
( id + id ) * id
```

## Included Sample Files
### Grammars
- input/grammar1.txt
- input/grammar2.txt
- input/grammar3.txt
- input/grammar_with_conflict.txt

### Input Sets
- input/input_valid.txt
- input/input_invalid.txt
- input/grammar1_valid.txt
- input/grammar1_invalid.txt
- input/grammar2_valid.txt
- input/grammar2_invalid.txt
- input/grammar3_valid.txt
- input/grammar3_invalid.txt
- input/grammar4_valid.txt
- input/grammar4_invalid.txt

## Known Limitations
- Grammar symbols are expected to be space-separated in the grammar file.
- The parser generator supports the assignment format directly and is intended for educational CFGs.
- Parse trees are text-based, as permitted in the assignment PDF.
- `docs/report.pdf` is intentionally not included in this package because it was excluded from the requested deliverables.
