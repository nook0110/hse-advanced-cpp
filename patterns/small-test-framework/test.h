#pragma once
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct Substr {
    explicit Substr(std::string string) : string_(std::move(string)) {
    }

    bool operator()(std::string_view s) const {
        return s.find(string_) != std::string::npos;
    }

private:
    std::string string_;
};

struct FullMatch {
    explicit FullMatch(std::string string) : string_(std::move(string)) {
    }

    bool operator()(std::string_view s) const {
        return s == string_;
    }

private:
    std::string string_;
};

class AbstractTest {
public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
    virtual ~AbstractTest() = default;
};

class TestCreatorBase {
public:
    virtual std::unique_ptr<AbstractTest> CreateTest() const = 0;
};

template <class TestClass>
class TestCreator : public TestCreatorBase {
public:
    std::unique_ptr<AbstractTest> CreateTest() const {
        return std::make_unique<TestClass>();
    }
};

class TestRegistry {
public:
    template <class TestClass>
    void RegisterClass(const std::string& class_name) {
        fabrics_[class_name] = std::make_unique<TestCreator<TestClass>>();
    }

    std::unique_ptr<AbstractTest> CreateTest(const std::string& class_name) {
        if (!fabrics_.contains(class_name)) {
            throw std::out_of_range{"No such test!"};
        }
        return fabrics_[class_name]->CreateTest();
    }

    void RunTest(const std::string& test_name) {
        auto test = CreateTest(test_name);
        test->SetUp();
        try {
            test->Run();
        } catch (...) {
            test->TearDown();
            throw;
        }
        test->TearDown();
    }

    template <class Predicate>
    std::vector<std::string> ShowTests(Predicate callback) const {
        std::vector<std::string> answer;
        for (const auto& [name, _] : fabrics_) {
            if (callback(name)) {
                answer.push_back(name);
            }
        }
        return answer;
    }

    std::vector<std::string> ShowAllTests() const {
        return ShowTests([](const std::string&) { return true; });
    }

    template <class Predicate>
    void RunTests(Predicate callback) {
        auto tests = ShowTests(callback);
        for (const auto& test : tests) {
            RunTest(test);
        }
    }

    void Clear() {
        fabrics_.clear();
    }

    static TestRegistry& Instance() {
        if (!instance) {
            instance = std::unique_ptr<TestRegistry>(new TestRegistry());
        }
        return *instance;
    }

    TestRegistry& operator=(const TestRegistry& other) = delete;

private:
    TestRegistry() = default;

    std::map<std::string, std::unique_ptr<TestCreatorBase>, std::less<>> fabrics_;
    static std::unique_ptr<TestRegistry> instance;
};

inline std::unique_ptr<TestRegistry> TestRegistry::instance = nullptr;
