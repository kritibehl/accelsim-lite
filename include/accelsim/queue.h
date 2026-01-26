#pragma once
#include <deque>
#include <stdexcept>

namespace accelsim {

template <typename T>
class BoundedQueue {
 public:
  explicit BoundedQueue(size_t capacity) : cap_(capacity) {}

  bool full() const { return q_.size() >= cap_; }
  bool empty() const { return q_.empty(); }
  size_t size() const { return q_.size(); }
  size_t capacity() const { return cap_; }

  void push(const T& v) {
    if (full()) throw std::runtime_error("Queue full");
    q_.push_back(v);
  }

  T pop() {
    if (empty()) throw std::runtime_error("Queue empty");
    T v = q_.front();
    q_.pop_front();
    return v;
  }

  const T& front() const {
    if (empty()) throw std::runtime_error("Queue empty");
    return q_.front();
  }

 private:
  size_t cap_;
  std::deque<T> q_;
};

} // namespace accelsim
