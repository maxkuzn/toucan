#pragma once

namespace toucan {

template <typename T>
struct IntrusiveListNode {
    IntrusiveListNode<T>* prev = nullptr;
    IntrusiveListNode<T>* next = nullptr;

    T* Get() {
        return static_cast<T*>(this);
    }

    void Unlink() {
        prev = nullptr;
        next = nullptr;
    }
};

}  // namespace toucan

