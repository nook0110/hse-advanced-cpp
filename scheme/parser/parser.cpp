#include <parser.h>
#include <cstddef>
#include <memory>
#include <variant>
#include <vector>
#include "error.h"
#include "object.h"
#include "tokenizer.h"
std::shared_ptr<Object> ReadList(Tokenizer* tokenizer);

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("empty!");
    }
    return std::visit(
        Overloaded{
            [tokenizer](const BracketToken& token) -> std::shared_ptr<Object> {
                if (token == BracketToken::OPEN) {
                    tokenizer->Next();  // Skip open bracket
                    auto ans = ReadList(tokenizer);
                    tokenizer->Next();  // Skip close bracket
                    return ans;
                }
                throw SyntaxError("closing bracket before, opening!");
            },
            [tokenizer](const ConstantToken& token) -> std::shared_ptr<Object> {
                tokenizer->Next();
                return std::make_shared<Number>(token.value);
            },
            [tokenizer](const SymbolToken& token) -> std::shared_ptr<Object> {
                tokenizer->Next();
                return std::make_shared<Symbol>(token.name);
            },
            [](const auto&) -> std::shared_ptr<Object> { throw SyntaxError("syntax error!"); }},
        tokenizer->GetToken());
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Empty list!");
    }

    auto is_close_bracket = [](const Token& token) {
        if (auto bracket = std::get_if<BracketToken>(&token)) {
            return *bracket == BracketToken::CLOSE;
        }
        return false;
    };

    if (is_close_bracket(tokenizer->GetToken())) {
        return nullptr;
    }

    std::vector<std::shared_ptr<Object>> elements;

    size_t amount_of_dots = 0;
    bool add_empty = true;
    while (!is_close_bracket(tokenizer->GetToken())) {
        add_empty = !std::holds_alternative<DotToken>(tokenizer->GetToken());

        if (std::holds_alternative<DotToken>(tokenizer->GetToken())) {
            ++amount_of_dots;
            tokenizer->Next();
        }

        elements.push_back(Read(tokenizer));

        if (tokenizer->IsEnd()) {
            throw SyntaxError{"No closing bracket!"};
        }
    }

    /* checking corrrectness */
    if (amount_of_dots != 0) {                           // if not proper list
        if (amount_of_dots != 1 || add_empty == true) {  // if not improper list
            throw SyntaxError{"Incorrect dots placement!"};
        }
    }

    if (add_empty && elements.empty()) {
        throw SyntaxError{"Incorrect list!"};
    }
    if (!add_empty && elements.size() < 2) {
        throw SyntaxError{"Incorrect list!"};
    }

    std::shared_ptr<Cell> answer;
    if (add_empty) {
        answer = std::make_shared<Cell>(elements.back());
        elements.pop_back();
    } else {
        answer =
            std::make_shared<Cell>(elements[elements.size() - 2], elements[elements.size() - 1]);
        elements.resize(elements.size() - 2);
    }
    while (!elements.empty()) {
        answer = std::make_shared<Cell>(elements.back(), answer);
        elements.pop_back();
    }

    return answer;
}