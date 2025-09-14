#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <regex>
#include <variant>
#include <optional>
#include <istream>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const = default;
};

struct QuoteToken {

    bool operator==(const QuoteToken&) const = default;
};

struct DotToken {
    bool operator==(const DotToken&) const = default;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const = default;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in) : in_(in) {
    }

    bool IsEnd() {
        SkipWhitespace();
        return in_->eof();
    }

    void Next() {
        auto cur = in_->tellg();
        cur += SkipSize();
        in_->seekg(cur);
    }

    Token GetToken();

private:
    template <class TokenType>
    static std::regex GetRegex();

    template <class TokenType>
    static TokenType Create(const std::string& s);

    template <class TokenType>
    std::optional<TokenType> GetToken();

    template <class TokenType>
    std::optional<std::string> GetMatch();

    size_t SkipSize();

    void SkipWhitespace() {
        auto next = in_->peek();
        while (next == ' ' || next == '\n') {
            in_->ignore();
            next = in_->peek();
        }
    }

    std::string GetNextWord();

    std::istream* in_;
};