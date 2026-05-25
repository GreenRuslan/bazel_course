# ===========================================================================
# WORKSPACE — The main configuration file for external dependencies in Bazel
# ===========================================================================
#
# This file defines the "root boundary" of a Bazel project.
# It serves three primary roles:
#   1. Declares the workspace name.
#   2. Loads external dependencies (libraries, toolchains, rules).
#   3. Registers toolchains and execution platforms for the build.
#
# ⚠️ IMPORTANT: Starting with Bazel 7, the official recommendation is to use
# MODULE.bazel (Bzlmod) instead of WORKSPACE. This file is kept for educational
# purposes and for projects that have not yet migrated to Bzlmod.
#
# 📖 Full lecture: workspace_lecture.md
# ===========================================================================


# ---------------------------------------------------------------------------
# 🏷️ STEP 1: workspace() — Declaring the workspace name
# ---------------------------------------------------------------------------
# workspace() — the first function that MUST be called in the WORKSPACE file.
# It sets the canonical name of your project. This name is used:
#   • As a prefix in external references: @bazel_query_tutorial//core:memory
#   • For identification in multi-repo setups.
#   • For forming runfiles paths for binaries.
#
# Naming rules:
#   ✅ Allowed: lowercase, digits, underscores (_), hyphens (-)
#   ❌ Forbidden: dots, spaces, uppercase (not recommended)
#
# Example: if another project depends on ours, it will reference targets as:
#   @bazel_query_tutorial//core:memory
workspace(
    name = "bazel_query_tutorial",
)


# ---------------------------------------------------------------------------
# 🔧 STEP 2: load() — Loading Starlark functions
# ---------------------------------------------------------------------------
# load() — this is NOT a dependency. It's an import of functions from .bzl files.
# Functions loaded here become available for calling below.
#
# Syntax: load("@repository//package:file.bzl", "function1", "function2")
#
# ⚠️ load() can ONLY be used at the TOP LEVEL of the file.
#     You cannot place load() inside if/for blocks or functions.
#
# Bazel has built-in (native) functions available without load():
#   • workspace(), bind(), register_toolchains(), register_execution_platforms()
#   • local_repository(), new_local_repository()
#
# Other functions require load() from their respective repository.
# For example: http_archive, git_repository, etc.

# Load http_archive — the most common way to add external dependencies
# in WORKSPACE. http_archive downloads an archive from a URL, extracts it,
# and creates a Bazel repository from it.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# Load git_repository — an alternative to http_archive for Git repositories.
# Bazel clones the repository at the specified commit/tag.
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")


# ---------------------------------------------------------------------------
# 📦 STEP 3: http_archive() — Downloading archived dependencies
# ---------------------------------------------------------------------------
# http_archive() — the "workhorse" of the WORKSPACE dependency system.
# It performs:
#   1. Downloads a .tar.gz/.zip archive from a URL.
#   2. Verifies integrity via sha256 hash (hermeticity!).
#   3. Extracts into bazel-external/<name>.
#   4. Can patch files post-extraction (patches, patch_cmds).
#   5. Can use strip_prefix to remove the archive's root folder.
#
# Parameters:
#   name          — Unique repository name. Referenced as @name//...
#   urls          — List of download URLs (fallback if the first is unavailable).
#   sha256        — Checksum for hermeticity (MANDATORY for CI!).
#   strip_prefix  — Removes the top-level directory from the archive.
#   build_file    — Path to a BUILD file, if the archive doesn't contain one.
#   patches       — List of patch files to apply after extraction.
#   patch_args    — Arguments for the patch command (usually ["-p1"]).
#   patch_cmds    — Shell commands to execute after extraction.
#
# ⚠️ Without sha256, the build is non-hermetic! Anyone could substitute the archive.
# ⚠️ Use urls (plural) with a list of mirrors for reliability.

# Example 1: Google Test — a C++ testing framework.
# This is a classic example: the library has its own BUILD file, so build_file
# is not needed.
http_archive(
    name = "com_google_googletest",
    urls = [
        # Primary URL (GitHub Release)
        "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz",
        # Mirror (for CI reliability, in case GitHub is temporarily unavailable)
        "https://mirror.bazel.build/github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz",
    ],
    # sha256 guarantees that we download the EXACT SAME archive every time.
    # If someone substitutes the archive — the build will fail with a hash error.
    # To get the hash: curl -sL <url> | shasum -a 256
    sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
    # strip_prefix removes the root folder "googletest-1.14.0/" from the archive.
    # Without it, all files would be under @com_google_googletest//googletest-1.14.0/...
    # With it — directly: @com_google_googletest//:gtest_main
    strip_prefix = "googletest-1.14.0",
)

# Example 2: Abseil — Google's C++ utility library.
# Demonstrates that each http_archive is a separate Bazel repository.
http_archive(
    name = "com_google_absl",
    urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.2.tar.gz"],
    sha256 = "733726b8c3a6d39a4120d7e45ea8b41a434cdacde401cba500f14236c49b39dc",
    strip_prefix = "abseil-cpp-20240116.2",
)

# Example 3: http_archive with build_file — for projects WITHOUT Bazel support.
# Imagine we want to include the C library "zlib", which uses Make/CMake
# and has no BUILD file. We create one ourselves!
#
# build_file_content allows writing a BUILD file inline:
http_archive(
    name = "zlib",
    urls = ["https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz"],
    sha256 = "9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23",
    strip_prefix = "zlib-1.3.1",
    # build_file_content: An inline BUILD file for libraries without Bazel support.
    # Here we declare a cc_library that compiles all .c files with standard
    # definitions and makes headers available via hdrs.
    build_file_content = """
cc_library(
    name = "zlib",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    copts = ["-Wno-implicit-function-declaration"],
    # Definitions for correct compilation on different platforms
    defines = ["HAVE_UNISTD_H"],
    includes = ["."],
    visibility = ["//visibility:public"],
)
""",
)

# Example 4: http_archive with patches — patching external code.
# Sometimes an external library has a bug, or its BUILD file needs changes.
# Instead of forking, you can apply a patch file.
#
http_archive(
    name = "some_external_lib",
    urls = ["file:///tmp/lib-1.0.tar.gz"],
    sha256 = "81d5fa03487218ee508cf3825d20ab675f431138cf1ed7224846da95527d141c",
    strip_prefix = "lib-1.0",
    # patches: list of patch files applied AFTER extraction.
    # Files must be in our workspace (e.g., third_party/patches/).
    patches = [
        "//third_party/patches:fix_build.patch",
        "//third_party/patches:add_visibility.patch",
    ],
    # patch_args: arguments for the UNIX `patch` command.
    # "-p1" — removes the first level of the path from the patch file
    # (standard for git diff output).
    patch_args = ["-p1"],
)

# Example 5: http_archive with patch_cmds — shell commands after extraction.
# Useful for minor changes without creating a .patch file.
#
http_archive(
    name = "legacy_lib",
    urls = ["file:///tmp/legacy-2.0.tar.gz"],
    sha256 = "128471112cb81f869aa18afc56335eae230d5a757d287ded9515614e79e43671",
    strip_prefix = "legacy-2.0",
    # patch_cmds: list of shell commands executed AFTER extraction.
    # Handy for quick hacks: rename a file, remove extras, etc.
    patch_cmds = [
        # Create an empty BUILD file so Bazel recognizes the package
        "touch BUILD",
        # Remove unneeded directories
        "rm -rf docs examples",
        # Replace text in a file (e.g., fix an include path)
        "sed -i.bak 's|/usr/local/include|.|g' config.h",
    ],
)


# ---------------------------------------------------------------------------
# 🔗 STEP 4: git_repository() — Cloning Git repositories
# ---------------------------------------------------------------------------
# git_repository() — an alternative to http_archive for Git-hosted projects.
# Bazel executes `git clone` + `git checkout` at the specified commit/tag.
#
# Parameters:
#   name    — Repository name (@name//...).
#   remote  — Git repository URL (https:// or git@...).
#   commit  — Full SHA hash of the commit (40 characters). MANDATORY!
#   tag     — Git tag (alternative to commit, but less reliable).
#   shallow_since — Date for optimization (shallow clone from this date).
#
# ⚠️ ALWAYS use commit, not tag!
#    Tags can be moved (force-push), but commits cannot.
#    This is critical for build hermeticity!
#
# ⚠️ git_repository() is slower than http_archive because it clones Git history.
#    In CI, always prefer http_archive for speed.
#
# Example: Connecting the fmt library for C++ string formatting.
#
git_repository(
    name = "com_github_fmtlib_fmt",
    remote = "https://github.com/fmtlib/fmt.git",
    # commit: FULL SHA hash. Guarantees identical code every time.
    commit = "4b50ad794422c6ecbf773141a09592fd9061a6fb",
    # shallow_since: Commit date (optimization for faster cloning).
    # Bazel will do a shallow clone starting from this date
    # instead of a full clone.
    shallow_since = "2024-01-04",
)


# ---------------------------------------------------------------------------
# 📂 STEP 5: local_repository() — Local Bazel projects
# ---------------------------------------------------------------------------
# local_repository() — connects ANOTHER Bazel project from the local
# filesystem. The project MUST have its own WORKSPACE (or MODULE.bazel).
#
# Parameters:
#   name — Repository name (@name//...).
#   path — Absolute or relative path to the project root.
#
# 🔑 Key difference from http_archive:
#   • local_repository — for projects that ARE already Bazel projects.
#   • new_local_repository — for projects WITHOUT BUILD files (see Step 6).
#
# Typical use cases:
#   • Monorepo with multiple Bazel projects.
#   • Developing two related libraries simultaneously (without publishing).
#   • Temporarily connecting a fork for debugging.
#
# Example:
#
local_repository(
    name = "my_other_project",
    # path can be relative (from the WORKSPACE root) or absolute.
    path = "../my-other-bazel-project",
)
#
# After this, you can reference targets from the other project:
#   deps = ["@my_other_project//lib:core"]


# ---------------------------------------------------------------------------
# 📁 STEP 6: new_local_repository() — Local non-Bazel projects
# ---------------------------------------------------------------------------
# new_local_repository() — for connecting local directories
# that are NOT Bazel projects. You create a BUILD file for them.
#
# Parameters:
#   name              — Repository name.
#   path              — Path to the local directory.
#   build_file        — Path to a BUILD file you created for this project.
#   build_file_content — Alternative to build_file: inline BUILD content.
#
# Typical use cases:
#   • Connecting system libraries (/usr/local/lib).
#   • Connecting third-party libraries built manually.
#   • Testing integration with legacy code.
#
# Example 1: Connecting system OpenSSL.
#
new_local_repository(
    name = "system_openssl",
    path = "/tmp/system_openssl",
    build_file_content = """
cc_library(
    name = "ssl",
    srcs = glob(["lib/*.dylib", "lib/*.so", "lib/*.a"]),
    hdrs = glob(["include/openssl/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
)
#
# Example 2: Connecting via a separate BUILD file.
#
new_local_repository(
    name = "vendored_protobuf",
    path = "/tmp/vendored_protobuf",
    # build_file: points to a BUILD file in OUR repository
    # that describes the structure of this external project.
    build_file = "//third_party:protobuf.BUILD",
)


# ---------------------------------------------------------------------------
# 📥 STEP 7: http_file() — Downloading individual files
# ---------------------------------------------------------------------------
# http_file() — downloads a SINGLE file from a URL.
# Useful for: ML models, configs, certificates, test data.
#
# Parameters:
#   name        — Repository name.
#   urls        — URL to the file.
#   sha256      — File checksum.
#   downloaded_file_path — Name of the saved file (default: "downloaded").
#   executable  — Whether to make the file executable (Boolean).
#
# Example: Downloading a tool binary (e.g., a protocol compiler).
#
http_file(
    name = "protoc_binary",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v25.1/protoc-25.1-linux-x86_64.zip"],
    sha256 = "ed8fca87a11c888fed329d6a59c34c7d436165f662a2c875246ddb1ac2b6dd50",
    downloaded_file_path = "protoc.zip",
    executable = True
)
#
# Example: Downloading test data.
#
http_file(
    name = "test_dataset",
    urls = ["file:///tmp/fixture.json"],
    sha256 = "8086e86215ff45ce1a3a41bf8e726868a5871042ae22b26cb3c89581da538049",
    downloaded_file_path = "fixture.json",
)
#
# Usage in a BUILD file:
#   data = ["@test_dataset//file:fixture.json"]


# ---------------------------------------------------------------------------
# ⚙️ STEP 8: register_toolchains() — Registering toolchains
# ---------------------------------------------------------------------------
# register_toolchains() — registers toolchains for cross-compilation
# and custom programming languages.
#
# A toolchain is a set of tools (compiler, linker, etc.) that Bazel
# selects automatically depending on the target platform.
#
# Syntax:
#   register_toolchains("@repo//package:toolchain_target")
#
# Typical use cases:
#   • Multiple compiler versions (GCC vs Clang).
#   • Cross-compilation (Linux -> Android, Mac -> iOS).
#   • Custom languages (Protocol Buffers, Thrift, etc.).
#
# Example:
#
local_repository(name="llvm_toolchain", path="/tmp/llvm_toolchain")
local_repository(name="rules_python", path="/tmp/rules_python")

register_toolchains(
    # Toolchain for C++ (Clang 17)
    "@llvm_toolchain//:cc-toolchain-x86_64-linux",
    # Toolchain for Python 3.11
    "@rules_python//python:autodetecting_toolchain",
    # Custom toolchain for building Protocol Buffers
    "//toolchains:protobuf_toolchain",
)


# ---------------------------------------------------------------------------
# 🌍 STEP 9: register_execution_platforms() — Execution platforms
# ---------------------------------------------------------------------------
# register_execution_platforms() — registers platforms on which
# Bazel can EXECUTE build actions (compilation, code generation).
#
# This is different from the target platform (where the code will RUN).
#
# Syntax:
#   register_execution_platforms("//platforms:my_platform")
#
# Example:
#
register_execution_platforms(
    # Local developer machine
    "@local_config_platform//:host",
    # Remote powerful server for building (Remote Build Execution)
    "//platforms:remote_linux_x86_64",
)


# ---------------------------------------------------------------------------
# 🔀 STEP 10: bind() [DEPRECATED] — Aliases for external dependencies
# ---------------------------------------------------------------------------
# bind() — creates a "virtual" target in the special //external package.
# This allows different parts of the project to reference one canonical name,
# while the concrete implementation is defined in WORKSPACE.
#
# ⚠️ DEPRECATED! Google officially discourages bind().
# Use alias() in BUILD files instead of bind().
# Or better — migrate to MODULE.bazel, where this problem is solved
# through label_mapping.
#
# bind() is kept here for understanding legacy code.
#
# Syntax:
#   bind(name = "alias_name", actual = "@repo//pkg:target")
#
# Example:
#
bind(
    name = "protobuf",
    actual = "@com_google_protobuf//:protobuf",
)
#
# Now you can use //external:protobuf instead of the full path.
# deps = ["//external:protobuf"]  # ← bind() alias
# deps = ["@com_google_protobuf//:protobuf"]  # ← equivalent without bind()


# ---------------------------------------------------------------------------
# 🧩 STEP 11: Dependency chains (Transitive Dependencies)
# ---------------------------------------------------------------------------
# In WORKSPACE, each library must declare ALL of its dependencies.
# Bazel does NOT support transitive dependency resolution in WORKSPACE!
#
# This means: if you include library A, which depends on B,
# you MUST also declare B in WORKSPACE.
#
# This is the biggest problem with WORKSPACE and the main reason Bzlmod exists!
#
# Typical pattern — libraries provide a *_deps() macro:
#
# load("@com_google_googletest//:googletest_deps.bzl", "googletest_deps")
# googletest_deps()  # ← This macro declares all googletest dependencies
#
# The "diamond dependency" problem:
#   If A depends on C@1.0, and B depends on C@2.0 — in WORKSPACE, the one
#   declared FIRST wins. This can lead to subtle errors.
#
#   # C@1.0 wins because it was declared first!
#   http_archive(name = "C", urls = ["...C-1.0.tar.gz"], ...)
#   http_archive(name = "C", urls = ["...C-2.0.tar.gz"], ...)  # ← IGNORED!
#
# In MODULE.bazel, this problem is solved via MVS (Minimal Version Selection).


# ---------------------------------------------------------------------------
# 🏗️ STEP 12: Full real-world example — connecting rules_proto
# ---------------------------------------------------------------------------
# Here's what a real Protocol Buffers rules setup looks like
# in the legacy WORKSPACE style (with the dependency chain):
#
# Step 1: Load rules_proto
http_archive(
    name = "rules_proto",
    urls = ["https://github.com/bazelbuild/rules_proto/archive/refs/tags/5.3.0-21.7.tar.gz"],
    sha256 = "dc3fb206a2cb3441b485eb1e423165b231235a1ea9b031b4433cf7bc1fa460dd",
    strip_prefix = "rules_proto-5.3.0-21.7",
)

# Step 2: Load and call ALL transitive dependencies of rules_proto
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
rules_proto_dependencies()
rules_proto_toolchains()
#
# # Step 3: Now you can use proto_library in BUILD files:
# # load("@rules_proto//proto:defs.bzl", "proto_library")
# # proto_library(name = "my_proto", srcs = ["my.proto"])


# ---------------------------------------------------------------------------
# 🏠 STEP 13: Custom repository rules
# ---------------------------------------------------------------------------
# You can also create your OWN repository rules using Starlark.
# This is a powerful mechanism for: auto-detecting system libraries,
# generating BUILD files, downloading from private sources.
#
# Example Starlark repository rule (stored in a .bzl file):
#
load("//build_tools:my_repo.bzl", "my_repo")
my_repo(name = "system_ssl")

# TODO: cover the override


# ---------------------------------------------------------------------------
# 📋 SUMMARY: Order of directives in WORKSPACE
# ---------------------------------------------------------------------------
# The correct order of directives in WORKSPACE:
#
#   1. workspace(name = "...")
#   2. load() — load required functions
#   3. http_archive / git_repository — external dependencies
#   4. load() from new repositories + call *_deps() macros
#   5. register_toolchains()
#   6. register_execution_platforms()
#
# ⚠️ Order MATTERS! Bazel reads WORKSPACE SEQUENTIALLY, top to bottom.
#    You cannot use load() from a repository that hasn't been declared yet.
