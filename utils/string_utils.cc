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

std::string StringUtils::get_build_mode() {
#if defined(OPT_BUILD)
    return "Optimized (Release)";
#elif defined(DBG_BUILD)
    return "Debug";
#else
    return "Fastbuild (Default)";
#endif
}

} // namespace utils
