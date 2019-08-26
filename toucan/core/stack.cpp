#include <toucan/core/stack.hpp>

#include <sys/mman.h>
#include <unistd.h>

// TODO: remove
#include <stdexcept>

namespace toucan {

static const size_t kFiberStackSize = 4;

FiberStack::FiberStack() {
    Reset();
}

FiberStack::FiberStack(FiberStack&& other) {
    start_ = other.start_;
    size_ = other.size_;
    other.Reset();
}

FiberStack& FiberStack::operator=(FiberStack&& other) {
    Release();
    start_ = other.start_;
    size_ = other.size_;
    other.Reset();
    return *this;
}

FiberStack::~FiberStack() {
    Release();
}

static size_t PagesToBytes(size_t pages_count) {
    return getpagesize() * pages_count;
}

FiberStack FiberStack::Allocate() {
    FiberStack stack;
    stack.size_ = PagesToBytes(kFiberStackSize);
    void* mmaped_data = mmap(nullptr, stack.size_,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS,
                  -1, 0);
    if (mmaped_data == MAP_FAILED) {
        throw std::runtime_error("mmap: cannot allocate");
    }
    stack.start_ = static_cast<char*>(mmaped_data);
    return stack;
}

void FiberStack::Reset() {
    start_ = nullptr;
    size_ = 0;
}

void FiberStack::Release() {
    if (start_) {
        int err = munmap(start_, size_);
        if (err) {
            throw std::runtime_error("cannot unmap");
        }
        Reset();
    }
}

char* FiberStack::Start() const {
    return start_;
}

char* FiberStack::End() const {
    return start_ + size_;
}

size_t FiberStack::Size() const {
    return size_;
}

}  // namespace toucan

