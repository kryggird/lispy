#include <iostream>

#include "interpreter.h"

std::ostream& operator<<(std::ostream& os, ExpressionPtr expr) {
    if (expr->type == Type::NUMBER) {
        auto number = (Number*) expr.get();
        os << "Number(" << number->number << ")";
    } else if (expr->type == Type::SYMBOL) {
        auto symbol = (Symbol*) expr.get();
        os << "Symbol(" << symbol->symbol << ")";
    } else if (expr->type == Type::LAMBDA) {
        os << "Lambda()";
    } else if (expr->type == Type::SPECIAL_FORM) {
        os << "SpecialForm()";
    } else if (expr->type == Type::LIST) {
        auto list = (List*) expr.get();
        os << "(";
        for (auto& elem: list->list) {
            os << elem << ", ";
        }
        os << ")";
    }
    return os;
}

void run_single_example(const std::string& code, Environment globals) {
    auto result = eval_string(code, globals);
    std::cout << code << " -> " << result << std::endl;
}

void run_examples() {
    auto globals = make_prelude();

    run_single_example("(+ (* 5 3) 20)", globals);
    run_single_example("(if (> 6 3) 20 (/ 5 0))", globals);
    run_single_example("(let x 3 (let x 7 (* x 4)))", globals);
    run_single_example("(let x 3 (* x 4))", globals);
    run_single_example("(let myfun (lambda (x y) (+ x y)) (myfun 4 5))", globals);
    run_single_example("(! 0)", globals);
    run_single_example("(>= 3 0)", globals);
    run_single_example("(<= 1 2)", globals);
}

void repl() {
    Environment variables = make_prelude();
    std::string line;

    while(std::cin) {
        std::cout << "> ";
        std::getline(std::cin, line);
        std::cout << eval_string(line, variables) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::strcmp(argv[1], "--examples") == 0) {
        run_examples();
    } else if (argc == 2 && std::strcmp(argv[1], "--repl") == 0) {
        repl();
    }
    return 0;
}
