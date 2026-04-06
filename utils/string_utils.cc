#include "utils/string_utils.h"
#include <cctype>
#include <algorithm>

namespace utils {

std::string StringUtils::to_upper(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return result;
}

std::string StringUtils::allocate_string(core::MemoryTracker& tracker, size_t size) {
    tracker.allocate(size);
    return std::string(size, 'A');
}

} // namespace utils
