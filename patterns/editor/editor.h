#pragma once
#include <cstddef>
#include <memory>
#include <stack>
#include <string>

class Editor;

class Command;
class TypeCommand;
class ShiftLeftCommand;
class ShiftRightCommand;
class BackspaceCommand;

class Editor {
    friend class Command;
    friend class TypeCommand;
    friend class ShiftLeftCommand;
    friend class ShiftRightCommand;
    friend class BackspaceCommand;

public:
    const std::string& GetText() const;

    void Type(char c);
    void ShiftLeft();
    void ShiftRight();
    void Backspace();

    void Undo();

    void Redo();

private:
    void Execute(std::shared_ptr<Command> command);

    std::string text_;
    size_t cursor_pos_ = 0;

    std::deque<std::shared_ptr<Command>> undo_history_;
    std::deque<std::shared_ptr<Command>> history_;
};

class Command {
public:
    Command(Editor* editor) : editor_(editor), cursor_pos_(editor->cursor_pos_) {
    }
    virtual void Execute() const = 0;
    virtual void Undo() const = 0;
    virtual ~Command() = default;

protected:
    Editor* editor_;
    size_t cursor_pos_;
};

class TypeCommand : public Command {
public:
    TypeCommand(Editor* editor, char c) : Command(editor), character_(c) {
    }
    void Execute() const final {
        editor_->text_.insert(editor_->text_.begin() + cursor_pos_, character_);
        ++editor_->cursor_pos_;
    }
    void Undo() const final {
        editor_->text_.erase(cursor_pos_, 1);
        --editor_->cursor_pos_;
    };

private:
    char character_;
};

class ShiftLeftCommand : public Command {
public:
    ShiftLeftCommand(Editor* editor) : Command(editor) {
    }
    void Execute() const final {
        --editor_->cursor_pos_;
    }
    void Undo() const final {
        ++editor_->cursor_pos_;
    };
};

class ShiftRightCommand : public Command {
public:
    ShiftRightCommand(Editor* editor) : Command(editor) {
    }

    void Execute() const final {
        ++editor_->cursor_pos_;
    }
    void Undo() const final {
        --editor_->cursor_pos_;
    };
};

class BackspaceCommand : public Command {
public:
    BackspaceCommand(Editor* editor)
        : Command(editor), character_(editor->GetText()[cursor_pos_ - 1]) {
    }
    void Execute() const final {
        editor_->text_.erase(cursor_pos_ - 1, 1);
        --editor_->cursor_pos_;
    };

    void Undo() const final {
        editor_->text_.insert(editor_->text_.begin() + cursor_pos_ - 1, character_);
        ++editor_->cursor_pos_;
    }

private:
    char character_;
};

const std::string& Editor::GetText() const {
    return text_;
}

void Editor::Type(char c) {
    Execute(std::make_shared<TypeCommand>(this, c));
}

void Editor::ShiftLeft() {
    if (cursor_pos_ == 0) {
        return;
    }
    Execute(std::make_shared<ShiftLeftCommand>(this));
}

void Editor::ShiftRight() {
    if (cursor_pos_ + 1 > text_.size()) {
        return;
    }
    Execute(std::make_shared<ShiftRightCommand>(this));
}

void Editor::Backspace() {
    if (cursor_pos_ == 0) {
        return;
    }
    Execute(std::make_shared<BackspaceCommand>(this));
}

void Editor::Undo() {
    if (history_.empty()) {
        return;
    }
    history_.back()->Undo();
    undo_history_.push_back(history_.back());
    history_.pop_back();
}

void Editor::Redo() {
    if (undo_history_.empty()) {
        return;
    }
    undo_history_.back()->Execute();
    history_.push_back(undo_history_.back());
    undo_history_.pop_back();
}

void Editor::Execute(std::shared_ptr<Command> command) {
    undo_history_.clear();
    history_.push_back(command);
    history_.back()->Execute();
}
