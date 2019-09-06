#include <toucan/sync/futex.hpp>

#include <toucan/core/scheduler.hpp>

namespace toucan {

void Futex::Wait(size_t value) {
    sl_.Lock();
    if (value != value_) {
        sl_.Unlock();
        return;
    }
    Node* curr = GetCurrentFiber();
    if (tail_) {
        tail_->next = curr;
        tail_ = curr;
    } else {
        head_ = curr;
        tail_ = curr;
    }
    GetCurrentScheduler()->Suspend(sl_);
}

void Futex::WakeOne() {
    sl_.Lock();
    if (!head_) {
        sl_.Unlock();
        return;
    }
    auto to_wake_up = head_;
    head_ = head_->next;
    if (!head_) {
        tail_ = nullptr;
    }
    sl_.Unlock();
    to_wake_up->Unlink();
    GetCurrentScheduler()->WakeUp(to_wake_up->Get());
}

void Futex::WakeAll() {
    Node* head = nullptr;
    sl_.Lock();
    head = head_;
    head_ = nullptr;
    tail_ = nullptr;
    sl_.Unlock();
    while (head) {
        Node* next = head->next;
        head->Unlink();
        GetCurrentScheduler()->WakeUp(head->Get());
        head = next;
    }
}

void Futex::SetValue(size_t value) {
    sl_.Lock();
    value_ = value;
    sl_.Unlock();
}

void Futex::IncrValue() {
    sl_.Lock();
    value_++;
    sl_.Unlock();
}

size_t Futex::GetValue() {
    sl_.Lock();
    size_t value = value_;
    sl_.Unlock();
    return value;
}

}  // namespace toucan
