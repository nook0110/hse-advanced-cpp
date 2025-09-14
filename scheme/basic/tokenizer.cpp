#include <tokenizer.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <regex>
#include <string_view>

Token Tokenizer::GetToken() {
    if (auto token = GetToken<ConstantToken>()) {
        return *token;
    }
    if (auto token = GetToken<SymbolToken>()) {
        return *token;
    }
    if (auto token = GetToken<QuoteToken>()) {
        return *token;
    }
    if (auto token = GetToken<DotToken>()) {
        return *token;
    }
    if (auto token = GetToken<BracketToken>()) {
        return *token;
    }

    assert(false);
}

template <>
std::regex Tokenizer::GetRegex<SymbolToken>() {
    return std::regex(R"(([a-zA-Z<=>*\/#][a-zA-Z<=>*\/#0-9?!-]*)|([+-]))");
}
template <>
std::regex Tokenizer::GetRegex<QuoteToken>() {
    return std::regex(R"(['])");
}
template <>
std::regex Tokenizer::GetRegex<DotToken>() {
    return std::regex(R"([.])");
}
template <>
std::regex Tokenizer::GetRegex<BracketToken>() {
    return std::regex(R"([()])");
}
template <>
std::regex Tokenizer::GetRegex<ConstantToken>() {
    return std::regex(R"([+-]?\d+)");
}

template <>
SymbolToken Tokenizer::Create<SymbolToken>(const std::string& match) {
    return SymbolToken{match};
}
template <>
QuoteToken Tokenizer::Create<QuoteToken>([[maybe_unused]] const std::string& match) {
    return QuoteToken{};
}
template <>
DotToken Tokenizer::Create<DotToken>([[maybe_unused]] const std::string& match) {
    return DotToken{};
}
template <>
BracketToken Tokenizer::Create<BracketToken>(const std::string& match) {
    if (match[0] == '(') {
        return BracketToken::OPEN;
    }
    return BracketToken::CLOSE;
}
template <>
ConstantToken Tokenizer::Create<ConstantToken>(const std::string& match) {
    return ConstantToken{std::stoi(match)};
}

template <class TokenType>
std::optional<TokenType> Tokenizer::GetToken() {
    SkipWhitespace();
    if (auto match = GetMatch<TokenType>()) {
        return Create<TokenType>(*match);
    }
    return {};
}

template <class TokenType>
std::optional<std::string> Tokenizer::GetMatch() {
    static const auto kRegex = GetRegex<TokenType>();

    std::smatch match;

    if (std::string next = GetNextWord();
        std::regex_search(next, match, kRegex, std::regex_constants::match_continuous)) {
        return match.str();
    }
    return {};
}

size_t Tokenizer::SkipSize() {
    if (auto match = GetMatch<QuoteToken>()) {
        return match->size();
    }
    if (auto match = GetMatch<DotToken>()) {
        return match->size();
    }
    if (auto match = GetMatch<BracketToken>()) {
        return match->size();
    }
    if (auto match = GetMatch<ConstantToken>()) {
        return match->size();
    }
    if (auto match = GetMatch<SymbolToken>()) {
        return match->size();
    }
    return 0;
}

std::string Tokenizer::GetNextWord() {
    auto save = in_->tellg();

    std::string s;
    (*in_) >> s;

    in_->seekg(save);
    return s;
}