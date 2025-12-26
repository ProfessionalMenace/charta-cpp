#include "checks.hpp"
#include "parser.hpp"
#include <print>

checks::Type decl2type(parser::TypeSig decl, std::string fname) {
    if (decl.is_stack) {
        decl.is_stack = false;
        auto type = decl2type(decl, fname);
        auto stk = checks::Type{
            checks::Type::Stack,
            checks::StackType{checks::StackType::Many,
                              std::make_shared<checks::Type>(type)}};
        return stk;
    }

    if (decl.name == "int") {
        return {checks::Type::Int, {}};
    } else if (decl.name == "float") {
        return {checks::Type::Float, {}};
    } else if (decl.name == "bool") {
        return {checks::Type::Bool, {}};
    } else if (decl.name == "char") {
        return {checks::Type::Char, {}};
    } else if (decl.name == "string") {
        return {checks::Type::String, {}};
    } else {
        if (decl.name.starts_with("#")) {
            return {checks::Type::Generic, {}};
        } else {
            throw checks::CheckError(
                "Type '" + decl.name + "' is not recognized.", fname);
        }
    }
}

void checks::TypeChecker::collect_sigs() {
    for (auto &decl : fns) {
        Function func{};
        func.args.reserve(decl.args.args.size());
        if (decl.args.kind == parser::Argument::Ellipses) {
            func.args.emplace_back(
                Type{Type::Stack, StackType{StackType::Unknown, {}}});
        }
        for (auto &[name, type] : decl.args.args) {
            func.args.emplace_back(decl2type(type, decl.name));
        }
        if (decl.rets.rest) {
            func.args.emplace_back(
                Type{Type::Stack, StackType{StackType::Many,
                                            std::make_shared<Type>(decl2type(
                                                *decl.rets.rest, decl.name))}});
        }
        func.rets.reserve(decl.rets.args.size());
        for (auto &type : decl.rets.args) {
            func.rets.emplace_back(decl2type(type, decl.name));
        }
        sigs.emplace(decl.name, func);
        std::string args{};
        if (!func.args.empty()) {
            args += func.args.front().show();
            for (auto elem = func.args.begin() + 1; elem != func.args.end();
                 ++elem) {
                args += ", " + elem->show();
            }
        }
        std::string rets{};
        if (!func.rets.empty()) {
            rets += func.rets.front().show();
            for (auto elem = func.rets.begin() + 1; elem != func.rets.end();
                 ++elem) {
                rets += ", " + elem->show();
            }
        }
        std::println("fn {} :: ({}) -> ({})", decl.name, args, rets);
    }
}

void checks::TypeChecker::verify(traverser::Function fn) {
    auto expected = sigs.at(fn.name);
    auto stack = expected.args;
}

void checks::TypeChecker::check() {
    collect_sigs();
    for (auto &fn : fns) {
        verify(fn);
    }
}

std::string checks::Type::show() {
    switch (kind) {
    case Int:
        return "int";
    case Float:
        return "float";
    case Bool:
        return "bool";
    case Char:
        return "char";
    case String:
        return "string";
    case Stack: {
        return std::get<StackType>(val).show();
    }
    case Generic: {
        return std::get<std::string>(val);
    }
    }
}

std::string checks::StackType::show() {
    switch (kind) {
    case Exact: {
        auto elems = std::get<std::vector<Type>>(val);
        std::string stk{};
        if (!elems.empty()) {
            stk += elems.front().show();
            for (auto elem = elems.begin() + 1; elem != elems.end(); ++elem) {
                stk += ", " + elem->show();
            }
        }
        return "[" + stk + "]";
    }
    case Many:
        return "[..." + std::get<std::shared_ptr<Type>>(val)->show() + "]";
    case Unknown:
        return "[?]";
    }
}
