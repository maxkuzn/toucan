#pragma once

#include <toucan/support/intrusive_list_node.hpp>
#include <toucan/support/spinlock.hpp>
#include <toucan/core/fiber.hpp>

namespace toucan {

class Futex {
  using Node = IntrusiveListNode<Fiber>;

  public:
    void Wait(size_t value);

    void WakeOne();
    void WakeAll();

    void SetValue(size_t value);

  private:
    SpinLock sl_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_t value_ = 0;
};

}  // namespace toucan
