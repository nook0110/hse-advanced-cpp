#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

template <typename Signature>
class FunctionRef;

template <class Return, typename... Args>
class FunctionRef<Return(Args...)> {
public:
    template <class T>
    FunctionRef(T&& functor)
        : springboard_([](void* function, Args... args) -> Return {
              if constexpr (std::same_as<void, Return>) {
                  (*reinterpret_cast<std::remove_reference_t<T>*>(function))(
                      std::forward<Args>(args)...);
              } else {
                  return (*reinterpret_cast<std::add_pointer_t<std::decay_t<T>>>(function))(
                      std::forward<Args>(args)...);
              }
          }),
          function_(reinterpret_cast<void*>(std::addressof(functor))) {
    }

    template <class T>
    FunctionRef(T* functor)
        : springboard_([](void* function, Args... args) -> Return {
              if constexpr (std::same_as<void, Return>) {
                  (*reinterpret_cast<T*>(function))(
                      std::forward<Args>(args)...);
              } else {
                  return (*reinterpret_cast<T*>(function))(
                      std::forward<Args>(args)...);
              }
          }),
          function_(reinterpret_cast<void*>(functor)) {
    }

    FunctionRef(const FunctionRef&) = default;
    FunctionRef(FunctionRef&&) = default;

    FunctionRef& operator=(const FunctionRef&) = default;
    FunctionRef& operator=(FunctionRef&&) = default;

    Return operator()(Args... args) {
        return springboard_(function_, std::forward<Args>(args)...);
    }

private:
    using SpringboardType = Return (*)(void*, Args...);
    SpringboardType springboard_;
    void* function_;
};