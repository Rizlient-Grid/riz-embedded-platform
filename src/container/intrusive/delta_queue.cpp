#include <riz/container/intrusive/delta_queue.h>

using namespace riz::container::intrusive;

bool delta_queue::empty() const noexcept {
    return size_ == 0;
}

std::size_t delta_queue::size() const noexcept {
    return size_;
}

void delta_queue::insert(key_type abs_key, node_type& entry) {
    delta_type accum = 0;
    node_type** indirect = &head_;
    while (*indirect && (*indirect)->delta + accum <= abs_key) {
        accum += (*indirect)->delta;
        indirect = &(*indirect)->next;
    }

    entry.delta = abs_key - accum;
    if (*indirect) {
        (*indirect)->delta -= entry.delta;
    }
    entry.next = *indirect;
    *indirect = &entry;
    ++size_;
}

bool delta_queue::erase(node_type& entry) noexcept {
    node_type** indirect = &head_;
    while (*indirect) {
        if (*indirect == &entry) {
            if (entry.next) {
                entry.next->delta += entry.delta;
            }
            *indirect = entry.next;
            entry.next = nullptr;
            --size_;
            return true;
        }
        indirect = &(*indirect)->next;
    }

    return false;
}

delta_queue::node_type* delta_queue::pop_front() noexcept {
    node_type* ret = head_;
    if (head_) {
        head_ = head_->next;
        --size_;
    }

    return ret;
}
