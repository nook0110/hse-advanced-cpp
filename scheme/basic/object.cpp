#include "object.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <vector>
#include <list>
#include "error.h"

using FlattenedCell = std::list<std::shared_ptr<Object>>;

static FlattenedCell Flatten(const Cell& cell) {
    FlattenedCell ans = {cell.GetFirst()};
    if (auto next_cell = As<Cell>(cell.GetSecond())) {
        ans.splice(ans.end(), Flatten(*next_cell));
    } else {
        ans.emplace_back(cell.GetSecond());
    }
    return ans;
}

bool Symbol::IsFunction() const {
    for (const auto& func : Function::kFunctions) {
        if (func->Check(*this)) {
            return true;
        }
    }
    return false;
}

const std::array<std::unique_ptr<Function>, 27> Function::kFunctions = {
    {std::make_unique<QuoteFunction>("quote"),
     std::make_unique<IsBooleanFunction>("boolean?"),
     std::make_unique<IsNumberFunction>("number?"),
     std::make_unique<NotFunction>("not"),
     std::make_unique<IsPairFunction>("pair?"),
     std::make_unique<IsNullFunction>("null?"),
     std::make_unique<IsListFunction>("list?"),
     std::make_unique<ConsFunction>("cons"),
     std::make_unique<CarFunction>("car"),
     std::make_unique<CdrFunction>("cdr"),
     std::make_unique<ListFunction>("list"),
     std::make_unique<ListRefFunction>("list-ref"),
     std::make_unique<ListTailFunction>("list-tail"),
     std::make_unique<UnaryFunction<decltype([](int64_t x) { return std::abs(x); })>>("abs"),
     std::make_unique<CmpFunction<std::equal_to<>>>("="),
     std::make_unique<CmpFunction<std::less<>>>("<"),
     std::make_unique<CmpFunction<std::greater<>>>(">"),
     std::make_unique<CmpFunction<std::less_equal<>>>("<="),
     std::make_unique<CmpFunction<std::greater_equal<>>>(">="),
     std::make_unique<LogicFunction<false, std::logical_or<>>>("or"),
     std::make_unique<LogicFunction<true, std::logical_and<>>>("and"),
     std::make_unique<FoldFunction<0, std::plus<>>>("+"),
     std::make_unique<FoldFunction<1, std::multiplies<>>>("*"),
     std::make_unique<
         BinaryFunction<decltype([](int64_t l, int64_t r) { return std::min(l, r); })>>("min"),
     std::make_unique<
         BinaryFunction<decltype([](int64_t l, int64_t r) { return std::max(l, r); })>>("max"),
     std::make_unique<BinaryFunction<std::minus<>>>("-"),
     std::make_unique<BinaryFunction<std::divides<>>>("/")}};

void Function::CheckCtx(std::shared_ptr<Object> ctx) {
    if (!ctx || !Is<Cell>(ctx)) {
        throw RuntimeError("Expected argument for " + name_ + "!");
    }
}
void Function::CheckAmountOfArgs(std::shared_ptr<Object> ctx, size_t amount) {
    CheckCtx(ctx);
    auto args = Flatten(*As<Cell>(ctx));
    if (args.back() != nullptr) {
        throw RuntimeError("Args are not a proper list!");
    }
    args.pop_back();  // remove nullptr
    if (args.size() != amount) {
        throw RuntimeError("Incorrect amount of args!");
    }
}

std::string Cell::Serialize() {
    auto elements = Flatten(*this);
    auto add_point = elements.back() != nullptr;
    if (!add_point) {
        elements.pop_back();  // remove nullptr
    }

    std::vector<std::string> names;
    std::ranges::transform(elements, std::back_insert_iterator(names),
                           [](const std::shared_ptr<Object>& object) -> std::string {
                               if (!object) {
                                   return "()";
                               }
                               return object->Serialize();
                           });
    if (add_point) {
        names.emplace(std::prev(names.end()), ".");
    }
    return "(" +
           std::accumulate(std::next(names.begin()), names.end(), names[0],
                           [](const std::string& a, const std::string& b) { return a + " " + b; }) +
           ")";
}

std::shared_ptr<Object> Symbol::Evaluate(std::shared_ptr<Object> ctx) {
    for (const auto& func : Function::kFunctions) {
        if (!func->Check(*this)) {
            continue;
        }

        return func->Evaluate(ctx);
    }

    return std::make_shared<Symbol>(name_);
};

std::shared_ptr<Object> QuoteFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    return As<Cell>(ctx)->GetFirst();
}

std::shared_ptr<Object> IsBooleanFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    if (auto symbol = As<Symbol>(As<Cell>(ctx)->GetFirst()->Evaluate())) {
        return std::make_shared<Symbol>(symbol->GetName() == "#f" || symbol->GetName() == "#t");
    } else {
        return std::make_shared<Symbol>(false);
    }
}

std::shared_ptr<Object> IsNumberFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    return std::make_shared<Symbol>(Is<Number>(As<Cell>(ctx)->GetFirst()));
}

std::shared_ptr<Object> IsPairFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    return std::make_shared<Symbol>(Is<Cell>(As<Cell>(ctx)->GetFirst()->Evaluate()));
}

std::shared_ptr<Object> IsNullFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    return std::make_shared<Symbol>(!As<Cell>(ctx)->GetFirst()->Evaluate());
}

std::shared_ptr<Object> IsListFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    if (auto list = As<Cell>(As<Cell>(ctx)->GetFirst()->Evaluate())) {
        auto flattened = Flatten(*list);
        return std::make_shared<Symbol>(!flattened.empty() && flattened.back() == nullptr);
    }

    if (As<Cell>(ctx)->GetFirst()->Evaluate() == nullptr) {
        return std::make_shared<Symbol>(true);
    }

    return std::make_shared<Symbol>(false);
}

std::shared_ptr<Object> NotFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    if (auto symbol = As<Symbol>(As<Cell>(ctx)->GetFirst()->Evaluate())) {
        return std::make_shared<Symbol>(symbol->GetName() == "#f");
    } else {
        return std::make_shared<Symbol>(false);
    }
}

std::shared_ptr<Object> ConsFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 2);

    auto args = Flatten(*As<Cell>(ctx));
    args.pop_back();

    return std::make_shared<Cell>(args.front(), args.back());
}

std::shared_ptr<Object> CarFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    if (auto cell = As<Cell>(As<Cell>(ctx)->GetFirst()->Evaluate())) {
        return cell->GetFirst();
    } else {
        throw RuntimeError("Expected list for 'car'!");
    }
}

std::shared_ptr<Object> CdrFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    if (auto cell = As<Cell>(As<Cell>(ctx)->GetFirst()->Evaluate())) {
        return cell->GetSecond();
    } else {
        throw RuntimeError("Expected list for 'cdr'!");
    }
}

std::shared_ptr<Object> ListFunction::Evaluate(std::shared_ptr<Object> ctx) {
    return ctx;
}

std::shared_ptr<Object> ListRefFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 2);

    auto args = Flatten(*As<Cell>(ctx));
    args.pop_back();  // remove nullptr

    auto list = As<Cell>(args.front()->Evaluate());
    if (!list) {
        throw RuntimeError("First arg should be list for 'list-ref'");
    }
    if (!Is<Number>(args.back())) {
        throw RuntimeError("Second arg should be number for 'list-ref'");
    }

    auto idx = static_cast<size_t>(As<Number>(args.back())->GetValue());
    auto flattened = Flatten(*list);
    if (flattened.back()) {
        throw RuntimeError("Not a proper list!");
    }
    flattened.pop_back();
    if (idx >= flattened.size()) {
        throw RuntimeError("Out of bounds!");
    }
    auto ans = flattened.begin();
    std::advance(ans, idx);
    return *ans;
}

std::shared_ptr<Object> ListTailFunction::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 2);

    auto args = Flatten(*As<Cell>(ctx));
    args.pop_back();  // remove nullptr

    auto list = As<Cell>(args.front()->Evaluate());
    if (!list) {
        throw RuntimeError("First arg should be list for 'list-tail'");
    }
    if (!Is<Number>(args.back())) {
        throw RuntimeError("Second arg should be number for 'list-tail'");
    }
    auto idx = static_cast<size_t>(As<Number>(args.back())->GetValue());

    auto ans = list;
    while (idx--) {
        if (!ans) {
            throw RuntimeError("Out of bounds1");
        }
        ans = As<Cell>(ans->GetSecond());
    }
    return ans;
}

template <class Functor>
std::shared_ptr<Object> UnaryFunction<Functor>::Evaluate(std::shared_ptr<Object> ctx) {
    CheckAmountOfArgs(ctx, 1);

    auto number = As<Number>(As<Cell>(ctx)->GetFirst());
    if (!number) {
        throw RuntimeError("Expected number as arg!");
    }
    return std::make_shared<Number>(Functor{}(number->GetValue()));
}

static std::vector<int64_t> GetValues(std::shared_ptr<const Cell> cell) {
    if (!cell) {
        return {};
    }
    auto args = Flatten(*cell);
    if (args.back() != nullptr) {
        throw RuntimeError("Args are not a proper list!");
    }
    args.pop_back();  // remove nullptr
    if (!std::ranges::all_of(args, [](std::shared_ptr<Object> object) {
            return Is<Number>(object) || Is<Cell>(object);
        })) {
        throw RuntimeError("Args are not numbers!");
    }
    std::vector<int64_t> values;
    values.reserve(args.size());
    std::ranges::transform(args, std::back_inserter(values), [](std::shared_ptr<Object> object) {
        if (Is<Cell>(object)) {
            auto val = As<Number>(object->Evaluate());
            if (!val) {
                throw RuntimeError("Args are not numbers!");
            }
            return val->GetValue();
        }
        return As<Number>(object)->GetValue();
    });
    return values;
}

template <class Cmp>
std::shared_ptr<Object> CmpFunction<Cmp>::Evaluate(std::shared_ptr<Object> ctx) {
    auto values = GetValues(As<const Cell>(ctx));
    bool ans = true;
    for (size_t i = 0; i + 1 < values.size(); ++i) {
        if (!Cmp{}(values[i], values[i + 1])) {
            ans = false;
        }
    }
    return std::make_shared<Symbol>(ans);
}

template <int64_t start_value, class Functor>
std::shared_ptr<Object> FoldFunction<start_value, Functor>::Evaluate(std::shared_ptr<Object> ctx) {
    auto values = GetValues(As<const Cell>(ctx));
    return std::make_shared<Number>(
        std::accumulate(values.begin(), values.end(), start_value, Functor{}));
}

template <bool start_value, class Functor>
std::shared_ptr<Object> LogicFunction<start_value, Functor>::Evaluate(std::shared_ptr<Object> ctx) {
    if (!Is<Cell>(ctx)) {
        return std::make_shared<Symbol>(start_value);
    }
    auto args = Flatten(*As<Cell>(ctx));
    if (args.back() != nullptr) {
        throw RuntimeError("Args are not a proper list!");
    }
    args.pop_back();  // remove nullptr
    for (const auto& arg : args) {
        if (Functor{}(static_cast<bool>(*arg->Evaluate()), start_value) != start_value) {
            return arg->Evaluate();
        }
    }
    return args.back()->Evaluate();
}

template <class Functor>
std::shared_ptr<Object> BinaryFunction<Functor>::Evaluate(std::shared_ptr<Object> ctx) {
    auto values = GetValues(As<const Cell>(ctx));
    if (values.empty()) {
        throw RuntimeError("Not enough args!");
    }
    return std::make_shared<Number>(
        std::accumulate(std::next(values.begin()), values.end(), values.front(), Functor{}));
}

std::shared_ptr<Object> Cell::Evaluate(std::shared_ptr<Object>) {
    auto symbol = As<Symbol>(GetFirst());
    if (!(symbol && symbol->IsFunction())) {
        throw RuntimeError("Lists (without functors) are not evaluatable!");
    }
    return symbol->Evaluate(GetSecond());
};
