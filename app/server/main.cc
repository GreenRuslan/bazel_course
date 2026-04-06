#include <iostream>
#include <string>

// Including from our utility package.
#include "utils/string_utils.h"
// Thanks to transitive dependencies, we can also use our core.
#include "core/memory.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting Backend Server..." << std::endl;

    core::MemoryTracker tracker;
    
    std::string message = "hello from bazel server!";
    std::string upper_message = utils::StringUtils::to_upper(message);
    
    std::cout << "Original: " << message << std::endl;
    std::cout << "Upper:    " << upper_message << std::endl;

    std::string large_string = utils::StringUtils::allocate_string(tracker, 1024);
    std::cout << "Current memory allocated: " << tracker.allocated_bytes() << " bytes." << std::endl;

    return 0;
}
