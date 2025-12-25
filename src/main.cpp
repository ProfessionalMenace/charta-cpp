#include "make_c.hpp"
#include "parser.hpp"
#include "traverser.hpp"
#include "utf.hpp"
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <variant>

int main(int argc, char *argv[]) {
    if (argc < 2)
        return 1;
    std::filesystem::path exe_dir{
        std::filesystem::weakly_canonical(std::filesystem::path(argv[0]))
            .parent_path()};
    std::ifstream file(argv[1]);
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string input{ss.str()};
    try {
        auto toks = parser::Lexer(input).parse_all();
        for (auto const &tok : toks) {
            std::visit(
                [&tok](auto &&val) {
                    using T = std::remove_cvref_t<decltype(val)>;

                    if constexpr (std::is_same_v<T, char32_t>) {
                        std::println("{}:{}/{}, Kind={}, Value={}", tok.start,
                                     tok.end, tok.length,
                                     static_cast<int>(tok.kind),
                                     encode_utf8(val));
                    } else {
                        std::println("{}:{}/{}, Kind={}, Value={}", tok.start,
                                     tok.end, tok.length,
                                     static_cast<int>(tok.kind), val);
                    }
                },
                tok.value);
        }
        auto decls = parser::Parser(toks).parse_program();
        backend::c::Program prog{};
        for (auto &tl : decls) {
            if (auto fn = std::get_if<parser::FnDecl>(&tl)) {
                auto tname = [](parser::TypeSig type) {
                    std::string t{};
                    if (type.is_stack) {
                        t += "[";
                    }
                    t += type.name;
                    if (type.is_stack) {
                        t += "]";
                    }
                    return t;
                };
                std::string args{};
                bool is_first{true};
                for (auto &[name, type] : fn->args.args) {
                    if (!is_first) {
                        args += ", ";
                    } else {
                        is_first = false;
                    }
                    args += name + " : " + tname(type);
                }
                std::string rets{};
                is_first = true;
                for (auto type : fn->rets.args) {
                    if (!is_first) {
                        rets += ", ";
                    } else {
                        is_first = false;
                    }
                    rets += tname(type);
                }
                std::println("fn {}({})\n -> ({}{})", fn->name, args, rets,
                             fn->rets.rest ? ", ..." + tname(*fn->rets.rest)
                                           : "");
                auto instrs = traverser::traverse(fn->body);
                prog.emplace_back(
                    traverser::Function{fn->name, fn->args, fn->rets, instrs});
                for (auto &instr : instrs) {
                    std::println("  {}", instr.show());
                }
            }
            auto out = backend::c::make_c(prog);
            std::ofstream of{std::string(argv[1]) + ".c"};
            of << out;
            of.flush();
            of.close();
        }
    } catch (parser::ParserError e) {
        std::size_t line{1};
        std::size_t start{0};
        std::size_t start2{e.start};
        for (std::size_t i = 0; i < e.start; ++i) {
            if (input[i] == '\n') {
                ++line;
                start = i;
            }
        }
        for (std::size_t i = e.start; i < e.end; ++i) {
            if (input[i] == '\n' || i >= input.size() - 1) {
                std::size_t pad = std::to_string(line).length() + 3;
                std::println("{} | {}\n{}{}", line,
                             input.substr(start, i - start + 1),
                             std::string(pad + start2 - start, ' '),
                             std::string(i - start2 + 1, '^'));
                start = i + 1;
                start2 = i + 1;
                ++line;
            }
        }
        std::println("Err: {}", e.what);
    }
}
