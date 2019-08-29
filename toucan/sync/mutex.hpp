#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/core/scheduler.hpp>

#include <toucan/support/spinlock.hpp>
#include <toucan/support/assert.hpp>

#include <iostream>

namespace toucan {

class Mutex {
  private:
    struct Node {
        Node* next = nullptr;
        Fiber* fiber = nullptr;
    };

  public:
    Mutex() {
    }

    ~Mutex() {
    }

    void Lock() {
        Node me;
        me.fiber = GetCurrentFiber();
        sl_.lock();
        ASSERT(owner_ != me.fiber, "Cannot lock by owner");
        if (!Empty() || owner_) {
            Push(&me);
            GetCurrentScheduler()->Suspend(sl_);
            ASSERT(head_ == &me, "Me should be in the head");
            Pop();
        }
        ASSERT(owner_ == nullptr, "Owner shouldn't exists");
        owner_ = me.fiber;
        sl_.unlock();
    }

    void lock() {
        Lock();
    }

    void Unlock() {
        sl_.lock();
        ASSERT(owner_ == GetCurrentFiber(), "Only owner_ can unlock");
        owner_ = nullptr;
        if (!Empty()) {
            GetCurrentScheduler()->WakeUp(head_->fiber);
        }
        sl_.unlock();
    }

    void unlock() {
        Unlock();
    }

    bool TryLock() {
        sl_.lock();
        bool success = !owner_ && Empty();
        if (success) {
            owner_ = GetCurrentFiber();
        }
        sl_.unlock();
        return success;
    }

    bool try_lock() {
        return TryLock();
    }

  private:
    void Push(Node* node) {
        Node* prev_tail = tail_;
        tail_ = node;
        if (prev_tail) {
            prev_tail->next = tail_;
        } else {
            head_ = tail_;
        }
    }

    void Pop() {
        ASSERT(head_ != nullptr, "Can pop only in not empty queue");
        head_ = head_->next;
        if (!head_) {
            tail_ = nullptr;
        }
    }

    bool Empty() {
        return !head_;
    }

    Fiber* owner_ = nullptr;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    SpinLock sl_;
};

using mutex = Mutex;

}  // namespace toucan

