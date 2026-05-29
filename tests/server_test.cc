#include <iostream>
#include <string>

// To test our utils and core
#include "utils/string_utils.h"
#include "core/memory.h"

// A very basic homemade assert since we're not pulling in gtest
// to keep the tutorial workspace simple and self-contained.
#define ASSERT_EQ(val1, val2) \
    if ((val1) != (val2)) { \
        std::cerr << "Assertion failed: " << #val1 << " != " << #val2 << std::endl; \
        return 1; \
    }

int main() {
    std::cout << "[RUNNING] server_test..." << std::endl;

    std::string test_str = "flaky test string";
    std::string result = utils::StringUtils::to_upper(test_str);
    ASSERT_EQ(result, "FLAKY TEST STRING");

    core::MemoryTracker tracker;
    utils::StringUtils::allocate_string(tracker, 100);
    ASSERT_EQ(tracker.allocated_bytes(), 100);

    // Verify select() and config_setting logic
    std::string sys_name = utils::StringUtils::get_system_name();
    std::string build_mode = utils::StringUtils::get_build_mode();
    
    std::cout << "[INFO] System detected via select(): " << sys_name << std::endl;
    std::cout << "[INFO] Build mode detected via select(): " << build_mode << std::endl;

    // Basic validity check (must return one of the expected strings, not empty)
    if (sys_name != "macOS" && sys_name != "Linux" && sys_name != "Unknown OS") {
        std::cerr << "Assertion failed: Invalid system name returned: " << sys_name << std::endl;
        return 1;
    }

    std::cout << "[PASSED] server_test." << std::endl;
    return 0;
}
