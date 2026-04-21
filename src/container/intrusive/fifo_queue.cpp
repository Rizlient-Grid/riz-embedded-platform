#include <riz/container/intrusive/fifo_queue.h>

using namespace riz::container::intrusive;

std::size_t fifo_queue::size() const noexcept
{
    return size_;
}

bool fifo_queue::empty() const noexcept
{
    return size_ == 0u;
}

void fifo_queue::push(node_type& entry) noexcept
{
    node_type** slot = tail_ ? &tail_->next : &head_;
    *slot = &entry;
    tail_ = &entry;
    entry.next = nullptr;
    ++size_;
}

fifo_queue::node_type* fifo_queue::pop_front() noexcept
{
    node_type* ret = head_;
    if (head_) {
        head_ = head_->next;
        if (!head_) {
            tail_ = nullptr;
        }
        --size_;
    }
    
    return ret;
}