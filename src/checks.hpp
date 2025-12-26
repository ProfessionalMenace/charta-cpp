#pragma once

#include "parser.hpp"
#include "traverser.hpp"
#include <memory>
#include <optional>
#include <unordered_map>

namespace checks {
struct CheckError {
    std::string what;
    std::string fname;
};

struct Type;

struct StackType {
    enum Kind { Exact, Many, Unknown } kind;
    std::variant<std::vector<Type>, std::shared_ptr<Type>> val;

    std::string show();
};

struct Type {
    enum Kind { Int, Float, Bool, Char, String, Stack, Generic } kind;
    std::variant<std::string, StackType> val;

    std::string show();
};

struct Function {
    std::vector<Type> args;
    std::vector<Type> rets;
};

class TypeChecker {
    std::vector<traverser::Function> fns;
    std::unordered_map<std::string, Function> sigs{};

    void collect_sigs();
    void verify(traverser::Function fn);

  public:
    TypeChecker(std::vector<traverser::Function> fns) : fns(std::move(fns)) {}

    void check();
};
}; // namespace checks
