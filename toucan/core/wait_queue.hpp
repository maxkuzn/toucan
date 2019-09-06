#pragma once

#include <toucan/support/intrusive_list_node.hpp>
#include <toucan/support/spinlock.hpp>
#include <toucan/core/fiber.hpp>

namespace toucan {

class WaitQueue {
  using Node = IntrusiveListNode<Fiber>;

  public:
    void Wait();

    void WakeOne();
    void WakeAll();

    bool Empty();

  private:
    SpinLock sl_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
};

}  // namespace toucan
