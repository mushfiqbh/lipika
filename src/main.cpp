#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open source file: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./lipika <source.lipi>\n";
        return 1;
    }

    try {
        std::string source = readFile(argv[1]);

        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        for (const auto& t : tokens) {
            if (t.type == TokenType::Invalid) {
                std::cerr << "Lexical Error at line " << t.line
                          << ": invalid token '" << t.lexeme << "'\n";
                return 1;
            }
        }

        Parser parser(tokens);
        Program program = parser.parse();

        if (!parser.errors().empty()) {
            for (const auto& e : parser.errors()) std::cerr << e << "\n";
            return 1;
        }

        SemanticAnalyzer semantic;
        if (!semantic.analyze(program)) {
            for (const auto& e : semantic.errors()) std::cerr << e << "\n";
            return 1;
        }

        PythonCodeGenerator generator;
        std::string python = generator.generate(program);

        std::ofstream out("output.py");
        out << python;
        out.close();

        std::cout << "Compilation successful.\n";
        std::cout << "Generated target: output.py\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Compiler Error: " << e.what() << "\n";
        return 1;
    }
}
