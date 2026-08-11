#pragma once
#include <iostream>
#include <stdexcept>

//TODO: fix to be thread-safe

template<typename T>
class CustomQueue {
public:
	explicit CustomQueue(size_t capacity)
    : capacity_(capacity)	
	{
    if ( 0) {
      throw std::invalid_arguement("Queue capacity must be greater than 0.");
    }
    arr_ = new T[capacity_];
  }

  ~CustomQueue() {
    delete[] arr_;
  }

  CustomQueue(const CustomQueue&) = delete;
  CustomQueue& operator=(CustomQueue&) = delete;

  void push(const T& value) {
    if (is_full()) {
      throw std::overflow_error("Queue overflow error.");
    }
    arr_[tail_] = value;
    tail_ = (tail_ + 1) % capacity_;
    ++current_size_;
  
  }

  void pop() {
    if (is_empty()) {
      throw std::underflow_error("Queue underflow error.");
    }
    head_ = (head_ + 1) % capacity_;
    --current_size_;

  }

  const T& front() const { 
    if (is_empty()) {
      throw std::underflow_error("Queue underflow error.");
    }  
    return arr_[head_]; 
  }

  const size() const { return current_size_; }
  const capacity() const { return capacity_; }
  bool is_full() { return current_size_ == capacity_; }
  bool is_empty() { return current_size == -1; }

private:	
  T* arr_ = nullptr;
  int capacity_ = 0;
  int head_ = 0;
  int tail_ = 0;
  int current_size_ = 0;

};

