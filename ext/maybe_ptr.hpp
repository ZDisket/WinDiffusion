#pragma once
#include <iostream>


/*
 maybe_ptr: Special smart pointer that can handle:
 1. Owning an object dynamically allocated by the owner class
 2. Using, but not owning, an object allocated on the stack of another class.
*/
// Thanks GPT-4
template<typename T>
class maybe_ptr {
    T* ptr = nullptr;
    bool owns = false; // Flag to indicate ownership

public:
    // Constructor for potentially owning a raw pointer
    maybe_ptr(T* rawPtr = nullptr, bool isOwner = false) : ptr(rawPtr), owns(isOwner) {
        if (!rawPtr && isOwner)
            throw std::invalid_argument("maybe_ptr:: Cannot own a nullptr!");
    }

    operator bool(){return ptr != nullptr;}

    // Destructor
    ~maybe_ptr() {
        if (owns) {
            delete ptr;
        }
    }

    // Delete copy constructor and copy assignment to prevent shallow copy
    maybe_ptr(const maybe_ptr&) = delete;
    maybe_ptr& operator=(const maybe_ptr&) = delete;

    // Move constructor
    maybe_ptr(maybe_ptr&& other) noexcept : ptr(other.ptr), owns(other.owns) {
        other.ptr = nullptr;
        other.owns = false;
    }

    // Move assignment operator
    maybe_ptr& operator=(maybe_ptr&& other) noexcept {
        if (this != &other) {
            if (owns) {
                delete ptr;
            }
            ptr = other.ptr;
            owns = other.owns;
            other.ptr = nullptr;
            other.owns = false;
        }
        return *this;
    }

    // Modifier functions
    void reset(T* rawPtr = nullptr, bool isOwner = false) {
        if (owns) {
            delete ptr;
        }
        ptr = rawPtr;
        owns = isOwner;
    }

    // Accessor functions
    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    // Ownership management
    bool is_owned() const { return owns; }
};
