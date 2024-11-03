#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx = nullptr) = 0;
    virtual std::string Serialize() = 0;
    explicit virtual operator bool() {
        return true;
    };

    virtual ~Object() = default;
};

class Number : public Object {
public:
    explicit Number(int64_t number) : number_(number) {
    }

    int64_t GetValue() const {
        return number_;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object>) override {
        return std::make_shared<Number>(number_);
    };

    std::string Serialize() override {
        return std::to_string(number_);
    }

    virtual ~Number() = default;

private:
    int64_t number_;
};

class Symbol : public Object {
public:
    explicit Symbol(std::string name) : name_(std::move(name)) {
    }
    explicit Symbol(bool value) : name_(value ? "#t" : "#f") {
    }

    const std::string& GetName() const {
        return name_;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
    bool IsFunction() const;

    std::string Serialize() override {
        return GetName();
    }

    explicit operator bool() override {
        return name_ != "#f";
    }

private:
    std::string name_;
};

class Function {
public:
    Function(std::string name) : name_(std::move(name)) {
    }
    bool Check(const Symbol& symbol) {
        return symbol.GetName() == name_;
    }
    virtual std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) = 0;

    virtual ~Function() = default;

    static const std::array<std::unique_ptr<Function>, 27> kFunctions;

private:
    std::string name_;
};

class QuoteFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class IsBooleanFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class IsNumberFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class IsPairFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class IsNullFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class IsListFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class NotFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class ConsFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class CarFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class CdrFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class ListFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class ListRefFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

class ListTailFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template <bool start_value, class Functor>
class LogicFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template <class Functor>
class UnaryFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template <class Cmp>
class CmpFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template <int64_t start_value, class Functor>
class FoldFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template <class Functor>
class BinaryFunction : public Function {
    using Function::Function;

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> ctx) override;
};

template class BinaryFunction<std::minus<int64_t>>;
template class BinaryFunction<std::divides<int64_t>>;

class Cell : public Object {
public:
    Cell(std::shared_ptr<Object> first = {}, std::shared_ptr<Object> second = {})
        : first_(first), second_(second) {
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Object>) override;

    std::string Serialize() override;

    virtual ~Cell() = default;

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return dynamic_cast<T*>(obj.get());
}
