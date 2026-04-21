#ifndef REFCELL_HPP
#define REFCELL_HPP

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

class RefCellError : public std::runtime_error {
public:
    explicit RefCellError(const std::string& message) : std::runtime_error(message) {}
    virtual ~RefCellError() = default;
};

class BorrowError : public RefCellError {
public:
    explicit BorrowError(const std::string& message) : RefCellError(message) {}
};

class BorrowMutError : public RefCellError {
public:
    explicit BorrowMutError(const std::string& message) : RefCellError(message) {}
};

class DestructionError : public RefCellError {
public:
    explicit DestructionError(const std::string& message) : RefCellError(message) {}
};

template <typename T>
class RefCell {
private:
    T value;
    mutable int borrow_count; // 0: no borrow, > 0: immutable borrows, -1: mutable borrow

public:
    class Ref;
    class RefMut;

    explicit RefCell(const T& initial_value) : value(initial_value), borrow_count(0) {}
    explicit RefCell(T&& initial_value) : value(std::move(initial_value)), borrow_count(0) {}

    RefCell(const RefCell&) = delete;
    RefCell& operator=(const RefCell&) = delete;
    RefCell(RefCell&&) = delete;
    RefCell& operator=(RefCell&&) = delete;

    Ref borrow() const {
        if (borrow_count < 0) {
            throw BorrowError("Already mutably borrowed");
        }
        return Ref(this);
    }

    std::optional<Ref> try_borrow() const {
        if (borrow_count < 0) {
            return std::nullopt;
        }
        return Ref(this);
    }

    RefMut borrow_mut() {
        if (borrow_count != 0) {
            throw BorrowMutError("Already borrowed");
        }
        return RefMut(this);
    }

    std::optional<RefMut> try_borrow_mut() {
        if (borrow_count != 0) {
            return std::nullopt;
        }
        return RefMut(this);
    }

    class Ref {
    private:
        const RefCell* cell;
        friend class RefCell;
        explicit Ref(const RefCell* c) : cell(c) {
            if (cell) {
                cell->borrow_count++;
            }
        }

    public:
        Ref() : cell(nullptr) {}

        ~Ref() {
            if (cell) {
                cell->borrow_count--;
            }
        }

        const T& operator*() const {
            return cell->value;
        }

        const T* operator->() const {
            return &(cell->value);
        }

        Ref(const Ref& other) : cell(other.cell) {
            if (cell) {
                cell->borrow_count++;
            }
        }

        Ref& operator=(const Ref& other) {
            if (this != &other) {
                if (cell) {
                    cell->borrow_count--;
                }
                cell = other.cell;
                if (cell) {
                    cell->borrow_count++;
                }
            }
            return *this;
        }

        Ref(Ref&& other) noexcept : cell(other.cell) {
            other.cell = nullptr;
        }

        Ref& operator=(Ref&& other) noexcept {
            if (this != &other) {
                if (cell) {
                    cell->borrow_count--;
                }
                cell = other.cell;
                other.cell = nullptr;
            }
            return *this;
        }
    };

    class RefMut {
    private:
        RefCell* cell;
        friend class RefCell;
        explicit RefMut(RefCell* c) : cell(c) {
            if (cell) {
                cell->borrow_count = -1;
            }
        }

    public:
        RefMut() : cell(nullptr) {}

        ~RefMut() {
            if (cell) {
                cell->borrow_count = 0;
            }
        }

        T& operator*() {
            return cell->value;
        }

        T* operator->() {
            return &(cell->value);
        }

        RefMut(const RefMut&) = delete;
        RefMut& operator=(const RefMut&) = delete;

        RefMut(RefMut&& other) noexcept : cell(other.cell) {
            other.cell = nullptr;
        }

        RefMut& operator=(RefMut&& other) noexcept {
            if (this != &other) {
                if (cell) {
                    cell->borrow_count = 0;
                }
                cell = other.cell;
                other.cell = nullptr;
            }
            return *this;
        }
    };

    ~RefCell() noexcept(false) {
        if (borrow_count != 0) {
            throw DestructionError("RefCell destroyed while borrowed");
        }
    }
};

#endif // REFCELL_HPP
