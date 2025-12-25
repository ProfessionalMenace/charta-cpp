#pragma once

#include "ir.hpp"
#include "parser.hpp"

namespace traverser {
struct TraverserError : std::exception {
    int x;
    int y;
    std::string what;

    TraverserError(int x, int y, std::string what)
        : std::exception(), x(x), y(y), what(std::move(what)) {}
};
std::vector<ir::Instruction> traverse(parser::Grid grid);
} // namespace traverser
