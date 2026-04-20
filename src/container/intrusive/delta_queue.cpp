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

void delta_queue::insert(key_type abs_key, node_type& entry)
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
        node_type* cur = head_;
        node_type* prev = nullptr;
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

bool delta_queue::erase(node_type& entry) noexcept
{
    if (!head_) {
        return false;
    }

    if (head_ == &entry) {
        if (entry.next) {
            entry.next->delta += entry.delta;
        }
        head_ = entry.next;
        entry.next = nullptr;
        --size_;
        return true;
    }

    node_type* prev = head_;
    node_type* cur = head_->next;
    while (cur) {
        if (cur == &entry) {
            if (entry.next) {
                entry.next->delta += entry.delta;
            }
            prev->next = entry.next;
            entry.next = nullptr;
            --size_;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }

    return false;
}

delta_queue::node_type* delta_queue::pop_front() noexcept
{
    node_type* ret = head_;
    if (head_) {
        head_ = head_->next;
        --size_;
    }

    return ret;
}
