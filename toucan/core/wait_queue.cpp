#include <toucan/core/wait_queue.hpp>

#include <toucan/core/scheduler.hpp>

namespace toucan {

void WaitQueue::Wait() {
    sl_.Lock();
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

void WaitQueue::WakeOne() {
    sl_.Lock();
    if (!head_) {
        sl_.Unlock();
        return;
    }
    Fiber* to_wake_up = head_->Get();
    head_ = head_->next;
    if (!head_) {
        tail_ = nullptr;
    }
    sl_.Unlock();
    to_wake_up->Unlink();
    GetCurrentScheduler()->WakeUp(to_wake_up);
}

void WaitQueue::WakeAll() {
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

bool WaitQueue::Empty() {
    sl_.Lock();
    bool res = !head_;
    sl_.Unlock();
    return res;
}

}  // namespace toucan
