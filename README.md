# 🔬 SLR(1) & LR(1) Parser — Compiler Construction (C++17)

> A complete, modular implementation of bottom-up parsers featuring table construction, conflict detection, parse tree generation, and performance comparison.

---

## 📌 Overview

This project implements two classic **bottom-up parsing algorithms** — **SLR(1)** and **LR(1)** — as part of a Compiler Construction assignment. It reads a Context-Free Grammar (CFG) from an input file, constructs the canonical item sets and parsing tables, then parses input strings while generating detailed traces and parse trees.

---

## ✨ Features

- 📥 Reads CFG from input files
- ➕ Automatic grammar augmentation with a new start symbol
- 🔁 CLOSURE and GOTO operations
- 📊 Builds complete **SLR(1)** and **LR(1)** parsing tables
- ⚡ Stack-based shift-reduce parsing
- ⚠️ Conflict detection: **Shift/Reduce** and **Reduce/Reduce**
- 🌳 Parse tree generation
- 📝 Detailed step-by-step parsing traces
- 📈 Side-by-side performance & comparison analysis

---

## 🧠 Concepts Covered

| Topic | Description |
|---|---|
| Bottom-Up Parsing | Shift-reduce parsing with explicit stack |
| SLR(1) | Parsing table built using FOLLOW sets |
| LR(1) | Parsing table built using lookahead items |
| Item Sets | LR(0) for SLR, LR(1) canonical collection |
| FIRST / FOLLOW | Used in table construction and conflict resolution |
| Conflict Detection | Identifies ambiguous grammars automatically |

---

## 🛠️ Tech Stack

- **Language:** C++17
- **Compiler:** g++ via MSYS2 / MinGW-w64
- **Build System:** GNU Make
- **Platform:** Windows (compatible with Linux & macOS)

---

## 📂 Project Structure

```
├── src/
│   ├── main.cpp              # Entry point
│   ├── grammar.cpp           # CFG parsing & augmentation
│   ├── items.cpp             # LR(0) / LR(1) item set construction
│   ├── slr_parser.cpp        # SLR(1) parser logic
│   ├── lr1_parser.cpp        # LR(1) parser logic
│   ├── parsing_table.cpp     # Table construction (ACTION/GOTO)
│   ├── stack.cpp             # Parsing stack
│   └── tree.cpp              # Parse tree generation
│
├── input/
│   ├── grammar1.txt
│   ├── grammar2.txt
│   ├── grammar3.txt
│   ├── grammar_with_conflict.txt
│   ├── input_valid.txt
│   └── input_invalid.txt
│
├── output/
│   ├── slr_items.txt
│   ├── slr_parsing_table.txt
│   ├── slr_trace.txt
│   ├── lr1_items.txt
│   ├── lr1_parsing_table.txt
│   ├── lr1_trace.txt
│   ├── comparison.txt
│   └── parse_trees.txt
│
├── Makefile
└── README.md
```

---

## ⚙️ Compilation

**Using Makefile (recommended):**
```bash
make
```

**Manually:**
```bash
g++ -std=c++17 src/*.cpp -o parser.exe
```

**Clean build artifacts:**
```bash
make clean
```

---

## ▶️ Usage

**Run the SLR(1) Parser:**
```bash
./parser.exe slr input/grammar1.txt input/input_valid.txt
```

**Run the LR(1) Parser:**
```bash
./parser.exe lr1 input/grammar1.txt input/input_valid.txt
```

**Test with a conflicting grammar:**
```bash
./parser.exe slr input/grammar_with_conflict.txt input/input_valid.txt
```

---

## 📊 Output Files

All outputs are written to the `output/` directory:

| File | Contents |
|---|---|
| `slr_items.txt` | LR(0) canonical item sets |
| `slr_parsing_table.txt` | SLR(1) ACTION/GOTO table |
| `slr_trace.txt` | Step-by-step SLR parsing trace |
| `lr1_items.txt` | LR(1) canonical item sets |
| `lr1_parsing_table.txt` | LR(1) ACTION/GOTO table |
| `lr1_trace.txt` | Step-by-step LR(1) parsing trace |
| `parse_trees.txt` | Generated parse trees |
| `comparison.txt` | Performance & feature comparison |

---

## ⚠️ Conflict Demonstration

The included `grammar_with_conflict.txt` demonstrates a classic ambiguous grammar:

- **SLR(1)** → produces a **shift/reduce conflict** ❌
- **LR(1)** → resolves the conflict using lookaheads ✅

This highlights why LR(1) is strictly more powerful than SLR(1).

---

## 📈 SLR(1) vs LR(1) — Comparison

| Feature | SLR(1) | LR(1) |
|---|---|---|
| Number of States | Fewer | More |
| Table Size | Smaller | Larger |
| Construction Time | Faster | Slower |
| Conflict Handling | Limited (uses FOLLOW) | Strong (uses lookaheads) |
| Grammars Accepted | Subset of LR(1) | All deterministic CFGs |

---

## 📌 Notes

- Supports **epsilon** productions (use `epsilon` or `@`)
- Supports **multi-character** non-terminals
- Fully modular — each component is independently reusable
- Designed for educational and demonstration purposes

---

## 👥 Authors

| Name | Roll Number |
|---|---|
| Shehryar Faisal | 23I-0604 |
| Zaki Nabeel | 23I-0508 |

---

## 🔮 Future Improvements

- [ ] LALR(1) parser implementation
- [ ] Graphical parse tree visualization
- [ ] GUI-based interface
- [ ] Support for more grammar formats

---

> *Built as part of the Compiler Construction course — demonstrating the practical trade-offs between parsing power and computational complexity.*
