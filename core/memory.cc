#include "core/memory.h"

namespace core {

MemoryTracker::MemoryTracker() : total_allocated_(0) {}
MemoryTracker::~MemoryTracker() {}

long MemoryTracker::allocated_bytes() const {
    return total_allocated_;
}

void MemoryTracker::allocate(long bytes) {
    if (bytes > 0) {
        total_allocated_ += bytes;
    }
}

void MemoryTracker::deallocate(long bytes) {
    if (bytes > 0) {
        total_allocated_ -= bytes;
        if (total_allocated_ < 0) {
            total_allocated_ = 0;
        }
    }
}

} // namespace core
