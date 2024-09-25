#pragma once

#include <cstddef>
#include <exception>
#include <optional>
#include <utility>

class BadWeakPtr : public std::exception {};

class ControlBlockBase;
struct SharedTag {};
struct WeakTag {};

template <typename T>
class ControlBlock;

class EnableSharedFromThisBase {};

template <typename T>
class MakeSharedControlBlock ;

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
