#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

using std::string;

std::vector<std::unique_ptr<string>> Duplicate(const std::vector<std::shared_ptr<string>>& items) {
    // Your code here
    std::vector<std::unique_ptr<string>> answer;
    for (const auto& item : items) {
        answer.emplace_back(std::make_unique<string>(*item));
    }
    return answer;
}

std::vector<std::shared_ptr<string>> DeDuplicate(
    const std::vector<std::unique_ptr<string>>& items) {
    std::unordered_map<string, std::shared_ptr<string>> pointers;

    std::vector<std::shared_ptr<string>> answer;
    for (const auto& item : items) {
        if (!pointers.contains(*item)) {
            pointers[*item] = std::make_shared<string>(*item);
        }
        answer.emplace_back(pointers.at(*item));
    }
    return answer;
}
