#include "huffman.h"
#include <list>
#include <memory>
#include <optional>
#include <cassert>
#include <numeric>
#include <stdexcept>

class HuffmanTree::Impl {
public:
    Impl() = default;

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&&);
    Impl& operator=(Impl&&);

    // code_lengths is the array of size no more than 16 with number of
    // terminated nodes in the Huffman tree.
    // values are the values of the terminated nodes in the consecutive
    // level order.
    void Build(const std::vector<uint8_t>& code_lengths, const std::vector<uint8_t>& values);

    // Moves the state of the Huffman tree by |bit|. If the node is terminal, returns true,
    // overwrites |value| and resets the tree state. If the node is intermediate, false is
    // returned without changing the value.
    bool Move(bool bit, int& value);
    ~Impl() = default;

private:
    struct BuildData {
        BuildData(const std::vector<uint8_t>& code_lengths, const std::vector<uint8_t>& values)
            : remaining_leafs(code_lengths), values(values.begin(), values.end()) {
        }

        bool IsFinished() const {
            return std::accumulate(remaining_leafs.begin() + depth, remaining_leafs.end(), 0) == 0;
        }

        bool IsTerminal() const {
            if (depth == 0) {
                return false;
            }
            return remaining_leafs[depth - 1] > 0;
        }

        uint8_t Pop() {
            uint8_t ans = values.front();
            values.pop_front();
            --remaining_leafs[depth - 1];
            return ans;
        }

        std::vector<uint8_t> remaining_leafs;
        std::list<uint8_t> values;
        int depth = 0;
    };

    struct Node;
    using NodePtr = std::unique_ptr<Node>;

    struct Node {
        NodePtr left;
        NodePtr right;
        std::optional<uint8_t> value;
    };

    void Build(Node* node, BuildData& data) {
        if (data.IsTerminal()) {
            node->value = data.Pop();
            return;
        }

        if (data.IsFinished()) {
            return;
        }

        ++data.depth;
        node->left = std::make_unique<Node>();
        Build(node->left.get(), data);
        --data.depth;

        if (!data.IsFinished()) {
            ++data.depth;
            node->right = std::make_unique<Node>();
            Build(node->right.get(), data);
            --data.depth;
        }
    }
    NodePtr root_ = std::make_unique<Node>();
    Node* cur_node_ = root_.get();
};

void HuffmanTree::Impl::Build(const std::vector<uint8_t>& code_lengths,
                              const std::vector<uint8_t>& values) {
    root_ = std::make_unique<Node>();
    cur_node_ = root_.get();

    if (code_lengths.size() > 16) {
        throw std::invalid_argument("bad");
    }
    if (std::accumulate(code_lengths.begin(), code_lengths.end(), size_t{0}) != values.size()) {
        throw std::invalid_argument("bad");
    }

    auto data = BuildData(code_lengths, values);

    Build(root_.get(), data);

    if (!data.IsFinished()) {
        throw std::invalid_argument("bad");
    }
}

bool HuffmanTree::Impl::Move(bool bit, int& value) {
    if (bit) {
        cur_node_ = cur_node_->right.get();
    } else {
        cur_node_ = cur_node_->left.get();
    }

    if (!cur_node_) {
        cur_node_ = root_.get();
        throw std::invalid_argument("bad");
    }

    if (cur_node_->value) {
        value = *cur_node_->value;
        cur_node_ = root_.get();

        return true;
    }

    return false;
}

HuffmanTree::HuffmanTree() : impl_(std::make_unique<HuffmanTree::Impl>()){};

void HuffmanTree::Build(const std::vector<uint8_t>& code_lengths,
                        const std::vector<uint8_t>& values) {
    impl_->Build(code_lengths, values);
}

bool HuffmanTree::Move(bool bit, int& value) {
    return impl_->Move(bit, value);
}

HuffmanTree::HuffmanTree(HuffmanTree&&) = default;

HuffmanTree& HuffmanTree::operator=(HuffmanTree&&) = default;

HuffmanTree::~HuffmanTree() = default;
