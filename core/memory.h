#ifndef CORE_MEMORY_H_
#define CORE_MEMORY_H_

namespace core {

// A simple utility class to calculate memory metrics for our tutorial.
// Any component needing memory tracking relies on this core module.
class MemoryTracker {
public:
    MemoryTracker();
    ~MemoryTracker();

    long allocated_bytes() const;
    void allocate(long bytes);
    void deallocate(long bytes);

private:
    long total_allocated_;
};

} // namespace core

#endif // CORE_MEMORY_H_
