#pragma once

#include <cstddef>
#include <cstdint>

namespace toucan {

class FiberStack {
  public:
    FiberStack();

    FiberStack(const FiberStack&) = delete;
    FiberStack& operator=(const FiberStack&) = delete;

    FiberStack(FiberStack&& other);
    FiberStack& operator=(FiberStack&& other);

    ~FiberStack();

    static FiberStack Allocate();
    void Release();

    char* Start() const;
    char* End() const;
    size_t Size() const;

  private:
    void Reset();

    char* start_;
    size_t size_;
};

class StackBuilder {
    using Word = std::uintptr_t;
    static const size_t kWordSize = sizeof(Word);

  public:
    StackBuilder(char* top) : top_(top) {
    }


    void AlignNextPush(size_t alignment) {
        size_t shift = (size_t)(top_ - kWordSize) % alignment;
        top_ -= shift;
    }

    void* Top() const {
        return top_;
    }

    StackBuilder& Allocate(size_t bytes) {
        top_ -= bytes;
        return *this;
    }

  private:
    char* top_;
};

}  // namespace toucan

