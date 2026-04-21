#include "RefCell.hpp"
#include <cassert>
#include <iostream>

void test_basic() {
    RefCell<int> cell(10);
    {
        auto r1 = cell.borrow();
        assert(*r1 == 10);
        auto r2 = cell.borrow();
        assert(*r2 == 10);
    }
    {
        auto rm = cell.borrow_mut();
        *rm = 20;
        assert(*rm == 20);
    }
    assert(*cell.borrow() == 20);
    std::cout << "test_basic passed" << std::endl;
}

void test_panic_borrow_mut() {
    RefCell<int> cell(10);
    auto r1 = cell.borrow();
    try {
        auto rm = cell.borrow_mut();
        assert(false);
    } catch (const BorrowMutError& e) {
        std::cout << "Caught expected BorrowMutError" << std::endl;
    }
}

void test_panic_borrow() {
    RefCell<int> cell(10);
    auto rm = cell.borrow_mut();
    try {
        auto r1 = cell.borrow();
        assert(false);
    } catch (const BorrowError& e) {
        std::cout << "Caught expected BorrowError" << std::endl;
    }
}

void test_try_borrow() {
    RefCell<int> cell(10);
    auto rm = cell.borrow_mut();
    auto r1 = cell.try_borrow();
    assert(!r1.has_value());
    auto rm2 = cell.try_borrow_mut();
    assert(!rm2.has_value());
    std::cout << "test_try_borrow passed" << std::endl;
}

void test_move() {
    RefCell<int> cell(10);
    auto rm = cell.borrow_mut();
    auto rm2 = std::move(rm);
    *rm2 = 30;
    assert(*rm2 == 30);
    std::cout << "test_move passed" << std::endl;
}

void test_destruction_error() {
    try {
        std::optional<RefCell<int>::Ref> r;
        {
            RefCell<int> cell(10);
            r = cell.borrow();
        }
        assert(false);
    } catch (const DestructionError& e) {
        std::cout << "Caught expected DestructionError" << std::endl;
    }
}

int main() {
    test_basic();
    test_panic_borrow_mut();
    test_panic_borrow();
    test_try_borrow();
    test_move();
    test_destruction_error();
    return 0;
}
