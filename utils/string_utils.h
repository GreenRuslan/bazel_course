#ifndef UTILS_STRING_UTILS_H_
#define UTILS_STRING_UTILS_H_

#include <string>
// We include a header from our core dependency.
// This is possible because we listed '//core:memory' in our 'deps'.
#include "core/memory.h"

namespace utils {

class StringUtils {
public:
    static std::string to_upper(const std::string& input);
    static std::string allocate_string(core::MemoryTracker& tracker, size_t size);
};

} // namespace utils

#endif // UTILS_STRING_UTILS_H_
