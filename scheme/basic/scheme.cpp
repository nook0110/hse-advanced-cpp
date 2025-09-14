#include "scheme.h"
#include <memory>
#include <sstream>
#include "error.h"
#include "object.h"
#include "parser.h"
#include "tokenizer.h"

std::string Interpreter::Run(const std::string& s) {
    std::stringstream ss;
    ss << s;
    auto tokenizer = std::make_unique<Tokenizer>(&ss);
    auto ast = Read(tokenizer.get());
    if (!ast) {
        throw RuntimeError("Empty ast!");
    }
    auto evaluated_ast = ast->Evaluate();
    if (!evaluated_ast) {
        return "()";
    }
    return evaluated_ast->Serialize();
}
