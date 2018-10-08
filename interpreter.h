#ifndef CLION_LISPY_INTERPRETER_H
#define CLION_LISPY_INTERPRETER_H

#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <unordered_map>
//#include <utility>

using TokenList = std::deque<std::string>;

class Expression;
class Environment;

using ExpressionPtr = std::shared_ptr<Expression>;
using ExpressionsList = std::vector<ExpressionPtr>;
using Locals = std::unordered_map<std::string, ExpressionPtr>;
//using Environment = std::unordered_map<std::string, ExpressionPtr>;
using LambdaContainer = std::function<ExpressionPtr(Environment&, ExpressionsList&)>;

ExpressionPtr eval(ExpressionPtr expr, Environment& env);

enum class Type {
    SYMBOL, NUMBER, LIST, LAMBDA, SPECIAL_FORM //, BOOLEAN
};

class Expression {
public:
    explicit Expression(Type _type): type {_type} {};
    Type type;
};

class Number: public Expression {
public:
    explicit Number(long _number): Expression {Type::NUMBER}, number {_number} {};

    long number;
};

class Symbol: public Expression {
public:
    explicit Symbol(std::string _symbol): Expression {Type::SYMBOL}, symbol {std::move(_symbol)} {};

    std::string symbol;
};

class List: public Expression {
public:
    explicit List(ExpressionsList _list): Expression {Type::LIST}, list {std::move(_list)} {};
    List(): Expression {Type::LIST}, list {ExpressionsList {}} {};

    ExpressionsList list;
};

class Callable: public Expression {
public:
    explicit Callable(Type _type, LambdaContainer _lambda): Expression {_type}, lambda {_lambda} {};

    LambdaContainer lambda;
    virtual ExpressionPtr call(Environment &env, List* list) = 0;

};

class Lambda: public Callable {
public:
    explicit Lambda(LambdaContainer _lambda): Callable {Type::LAMBDA, std::move(_lambda)} {};

    virtual ExpressionPtr call(Environment &env, List* list) {
        ExpressionsList args = {};
        for (auto iter = list->list.begin()+1; iter != list->list.end(); ++iter) {
            args.push_back(eval(*iter, env));
        }

        auto return_value = this->lambda(env, args);
        return return_value;

    }
};

class SpecialForm: public Callable {
public:
    explicit SpecialForm(LambdaContainer _lambda): Callable {Type::SPECIAL_FORM, std::move(_lambda)} {};

    virtual ExpressionPtr call(Environment &env, List* list) {
        ExpressionsList args = {};
        for (auto iter = list->list.begin()+1; iter != list->list.end(); ++iter) {
            args.push_back(*iter);
        }

        auto return_value = this->lambda(env, args);
        return return_value;

    }
};

class Environment {
public:
    explicit Environment(): locals {}, parent {nullptr} {};
    explicit Environment(Environment* parent_): locals {}, parent {parent_} {};

    Locals::iterator find(const std::string& symbol) {
        return this->find_inner(symbol, this->locals.end());
    }
    ExpressionPtr& operator[](const std::string& symbol) {
        auto iter = this->find(symbol);
        if (iter != this->locals.end()) {
            return iter->second;
        } else {
            return this->locals[symbol];
        }
    }
private:
    Locals locals;
    Environment* parent;
    Locals::iterator find_inner(const std::string& symbol, Locals::iterator end_iterator) {
        auto iter = this->locals.find(symbol);
        if (iter != this->locals.end()) {
            return iter;
        } else if (iter == this->locals.end() && this->parent != nullptr) {
            return this->parent->find_inner(symbol, end_iterator);
        } else {
            return end_iterator;
        }
    }
};

TokenList tokenize(const std::string& code) {
    TokenList tokens {};
    std::string current_token;

    for(const char& c : code) {
        switch(c) {
            case '(':
            case ')':
            case ' ':
                if(current_token.length() > 0) {
                    tokens.push_back(current_token);
                    current_token = "";
                }
                if(c != ' ') {
                    std::string one_char_token {c};
                    tokens.push_back(one_char_token);
                }
                break;
            default:
                current_token.push_back(c);
                break;
        }
    }

    return tokens;
}

ExpressionPtr atom(const std::string& token) {
    char* end;
    long number = std::strtol(token.c_str(), &end, 10);

    if(*end == 0) {
        return ExpressionPtr(new Number(number));
    } else {
        return ExpressionPtr(new Symbol(token));
    }
}


ExpressionPtr read_from_tokens(TokenList& tokens) {
    std::string current_token = tokens.front();
    tokens.pop_front();

    if(current_token == "(") {
        ExpressionsList parsed_token_list;
        while(tokens.front() != ")") {
            ExpressionPtr next_parsed_token = read_from_tokens(tokens);
            parsed_token_list.push_back(next_parsed_token);
        }

        tokens.pop_front();
        return ExpressionPtr(new List(parsed_token_list));
    } else if(current_token == ")") {
        exit(1);
    } else {
        ExpressionPtr parsed_token = atom(current_token);
        return parsed_token;
    }
}

void assert_symbolic(Type type) {
    if (type != Type::SYMBOL) {
        exit(1);
    }
}

void assert_list(Type type) {
    if (type != Type::LIST) {
        exit(1);
    }
}


void assert_callable(Type type) {
    if (type != Type::LAMBDA && type != Type::SPECIAL_FORM) {
        exit(1);
    }
}

void assert_numeric(Type type) {
    if (type != Type::NUMBER) {
        exit(1);
    }
}

void assert_arity(ExpressionsList args, size_t arity) {
    if (args.size() != arity) {
        exit(1);
    }
}

bool truthiness(ExpressionPtr expr) {
    if (expr->type == Type::NUMBER) {
        auto number = static_cast<Number*>(expr.get());
        return number->number != 0;
    } else if (expr->type == Type::LIST) {
        auto list = static_cast<List *>(expr.get());
        return list->list.size() > 0;
    } else {
        return true;
    }
}

ExpressionPtr eval(ExpressionPtr expr, Environment& env) {
    switch (expr->type) {
        case Type::NUMBER: {
            return expr;
        }
        case Type::LAMBDA: {
            return expr;
        }
        case Type::SPECIAL_FORM: {
            return expr;
        }
        case Type::SYMBOL: {
            auto symbol = static_cast<Symbol*>(expr.get());
            return env[symbol->symbol];
        }
        case Type::LIST: {
            auto list = static_cast<List*>(expr.get());
            // FIXME handle empty list
            ExpressionPtr head = eval(list->list[0], env);
            auto callable = static_cast<Callable*>(head.get());

            ExpressionPtr return_value = callable->call(env, list); //call(env, list, head);

            return return_value;
        }
    }
}

ExpressionPtr eval_string(const std::string& code, Environment& env) {
    auto tokens = tokenize(code);
    auto expressions = read_from_tokens(tokens);
    return eval(expressions, env);
}

void print_tokens(TokenList tokens) {
    for (auto const& t: tokens) {
        std::cout << t << " ";
    }
}


template<typename F, size_t... S>
ExpressionPtr make_numeric_function(F function, std::index_sequence<S...>) {
    LambdaContainer inner = [function](Environment& env, ExpressionsList& args) {
        assert_arity(args, sizeof...(S));
//        assert_numeric(args[S]->type)...;

        auto result = function((static_cast<Number*>(args[S].get())->number)...);

        return ExpressionPtr(new Number(result));
    };

    return ExpressionPtr(new Lambda(inner));
}

template<size_t N, typename F>
ExpressionPtr make_numeric_function(F function) {
    return make_numeric_function(function, std::make_index_sequence<N>());
}

template<typename F, size_t... S>
ExpressionPtr make_special_form(F function, std::index_sequence<S...>) {
    LambdaContainer inner = [function](Environment& env, ExpressionsList& args) {
        assert_arity(args, sizeof...(S));
        return function(env, args[S]...);
    };

    return ExpressionPtr(new SpecialForm(inner));
}

template<size_t N, typename F>
ExpressionPtr make_special_form(F function) {
    return make_special_form(function, std::make_index_sequence<N>());
}

Environment make_prelude() {
    Environment globals {};
    globals["if"] = make_special_form<3>([](auto& env, auto cond, auto case_true, auto case_false) {
        auto result = eval(cond, env);
        if (truthiness(result)) {
            return eval(case_true, env);
        } else {
            return eval(case_false, env);
        }
    });
    globals["let"] = make_special_form<3>([](auto& env, auto symbol, auto value, auto executable) {
        auto child_env = Environment(&env);
        auto name = static_cast<Symbol*>(symbol.get())->symbol;
        child_env[name] = eval(value, env);

        return eval(executable, child_env);
    });
    globals["lambda"] = make_special_form<2>([](auto& env, auto arg_names, auto executable) {
        assert_list(arg_names->type);
        auto list = static_cast<List*>(arg_names.get());
        std::vector<std::string> raw_arg_names;
        for (auto expr: list->list) {
            assert_symbolic(expr->type);
            auto symbol = static_cast<Symbol*>(expr.get());
            raw_arg_names.push_back(symbol->symbol);
        }

        LambdaContainer inner = [raw_arg_names, executable](Environment& env, ExpressionsList& arg_values) {
            auto child_env = Environment(&env);
            assert_arity(arg_values, raw_arg_names.size());
            for (int arg_idx = 0; arg_idx < raw_arg_names.size(); ++arg_idx) {
                child_env[raw_arg_names[arg_idx]] = arg_values[arg_idx];
            }

            return eval(executable, child_env);
        };

        return ExpressionPtr(new Lambda(inner));
    });

    globals["+"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs + rhs;});
    globals["-"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs - rhs;});
    globals["*"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs * rhs;});
    globals["/"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs / rhs;});

    globals["!"] = make_numeric_function<1>([](auto value) { return !value; });
    globals["&&"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs && rhs;});
    globals["||"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs || rhs;});

    globals[">"] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs > rhs;});
    globals["="] = make_numeric_function<2>([](auto lhs, auto rhs) { return lhs == rhs;});
    globals[">="] = eval_string("(lambda (lhs rhs) (|| (> lhs rhs) (= lhs rhs)))", globals);
    globals["<"] = eval_string("(lambda (lhs rhs) (! (>= lhs rhs)))", globals);
    globals["<="] = eval_string("(lambda (lhs rhs) (! (> lhs rhs)))", globals);

    return globals;
}

#endif //CLION_LISPY_INTERPRETER_H
