#pragma once

#include <toucan/fault/inject_fault.hpp>

#include <atomic>

namespace toucan {
namespace fault {

template <typename T>
class FaultyAtomic {
  public:
    FaultyAtomic() {
        AccessAdversary();
    }

    FaultyAtomic(T desired) : value_(desired) {
        AccessAdversary();
    }

    T operator=(T desired) noexcept {
        store(desired);
        return desired;
    }

    T load(std::memory_order order = std::memory_order_seq_cst) const noexcept {
        InjectFault();
        T value = value_.load(order);
        InjectFault();
        return value;
    }

    void store(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
        InjectFault();
        value_.store(desired, order);
        InjectFault();
    }

    operator T() const noexcept {
        return load();
    }

    T exchange(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
        InjectFault();
        T prev_value = value_.exchange(desired, order);
        InjectFault();
        return prev_value;
    }

    bool compare_exchange_weak(T& expected, T desired,
                               std::memory_order success,
                               std::memory_order failure) noexcept {
        InjectFault();
        bool result = value_.compare_exchange_weak(expected, desired,
                                                   success, failure);
        InjectFault();
        return result;
    }

    bool compare_exchange_weak(T& expected, T desired,
                               std::memory_order order =
                                 std::memory_order_seq_cst) noexcept {
        return compare_exchange_weak(expected, desired, order, order);
    }

    bool compare_exchange_strong(T& expected, T desired,
                               std::memory_order success,
                               std::memory_order failure) noexcept {
        InjectFault();
        bool result = value_.compare_exchange_strong(expected, desired,
                                                     success, failure);
        InjectFault();
        return result;
    }

    bool compare_exchange_strong(T& expected, T desired,
                               std::memory_order order =
                                 std::memory_order_seq_cst) noexcept {
        return compare_exchange_strong(expected, desired, order, order);
    }

    T fetch_add(T arg,
                std::memory_order order =
                  std::memory_order_seq_cst) noexcept {
        InjectFault();
        T prev_value = value_.fetch_add(arg, order);
        InjectFault();
        return prev_value;
    }

    T operator+=(T arg) noexcept {
        return fetch_add(arg) + arg;
    }

    T operator++() noexcept {
        return fetch_add(1) + 1;
    }

    T operator++(int) noexcept {
        return fetch_add(1);
    }

    T fetch_sub(T arg,
                std::memory_order order =
                  std::memory_order_seq_cst) noexcept {
        InjectFault();
        T prev_value = value_.fetch_sub(arg, order);
        InjectFault();
        return prev_value;
    }

    T operator-=(T arg) noexcept {
        return fetch_sub(arg) - arg;
    }

    T operator--() noexcept {
        return fetch_sub(1) - 1;
    }

    T operator--(int) noexcept {
        return fetch_sub(1);
    }

  private:
    std::atomic<T> value_;
};

}  // namespace fault
}  // namespace toucan

