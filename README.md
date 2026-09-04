# Lipika — Bangla Toy Programming Language

Compiler implementation: C++17
Target language: Python

Pipeline:
Lipika source -> Lexer -> Parser -> AST -> Semantic Analysis -> Python Code Generator

Current starter features:
- Integer and floating-point types
- Type checking
- Arithmetic with precedence
- Assignment
- IF-ELSE
- WHILE
- Basic syntax error recovery
- Python target generation

Build directly with g++:
g++ -std=c++17 src/*.cpp -o lipika
lipika examples/demo.lipi
python output.py
