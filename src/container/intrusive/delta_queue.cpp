#include <riz/container/intrusive/delta_queue.h>

using namespace riz::container::intrusive;

bool delta_queue::empty() const noexcept
{
    return size_ == 0;
}

std::size_t delta_queue::size() const noexcept
{
    return size_;
}

void delta_queue::insert(key_type abs_key, node& entry)
{
    if (!head_) {
        entry.delta = abs_key;
        entry.next = nullptr;
        head_ = &entry;
    } else if (abs_key < head_->delta) {
        entry.delta = abs_key;
        entry.next = head_;
        head_->delta -= abs_key;
        head_ = &entry;
    } else {
        node* cur = head_;
        node* prev = nullptr;
        delta_type accum = 0;
        while (cur && cur->delta + accum <= abs_key) {
            accum += cur->delta;
            prev = cur;
            cur = cur->next;
        }

        entry.delta = abs_key - accum;
        entry.next = cur;
        prev->next = &entry;
        if (cur) {
            cur->delta -= entry.delta;
        }
    }

    ++size_;
}

delta_queue::node* delta_queue::pop_front() noexcept
{
    node* ret = head_;
    if (head_) {
        head_ = head_->next;
        --size_;
    }

    return ret;
}
