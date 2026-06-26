load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

def cc_library_with_test(name, srcs, hdrs = None, deps = None, test_srcs = None, test_deps = None, **kwargs):
    """A macro that creates a cc_library and an optional associated cc_test.

    Args:
        name: Name of the library target. The test target is automatically named '<name>_test'.
        srcs: Private source files for the library.
        hdrs: Public headers for the library.
        deps: Dependencies of the library.
        test_srcs: Source files for the test. If empty, no test target is generated.
        test_deps: Additional dependencies required only by the test target.
        **kwargs: Other standard attributes (e.g., visibility, tags, copts) forwarded to both targets.
    """
    # Resolve optional arguments to empty lists
    hdrs = hdrs or []
    deps = deps or []
    test_srcs = test_srcs or []
    test_deps = test_deps or []

    # 1. Define the library target
    cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        deps = deps,
        **kwargs
    )

    # 2. Define the test target (if test sources are specified)
    if test_srcs:
        cc_test(
            name = name + "_test",
            srcs = test_srcs,
            # The test automatically depends on the library we just built plus test_deps
            deps = deps + test_deps + [":" + name],
            **kwargs
        )

