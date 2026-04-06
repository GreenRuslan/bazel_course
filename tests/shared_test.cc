#include "core/memory.h"
#include "utils/string_utils.h"

int main() {
    // This test deliberately includes core and utils simultaneously 
    // to build overlapping dependency trees for bazel query `intersect` operations
    return 0;
}
