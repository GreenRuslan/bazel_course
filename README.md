# Bazel Course

Hands-on Bazel exercises. Follow `lecture.md` to work through the examples.

## Run in GitHub Codespaces (recommended)

No local setup needed — everything is pre-installed automatically.

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/GreenRuslan/bazel_course)

1. Click the badge above (or **Code → Codespaces → Create codespace on main** on the repo page).
2. Wait for the container to build — Bazel is installed automatically (~2-3 minutes on first launch).
3. Verify Bazel is ready:
   ```bash
   bazel version
   ```
4. Open `lecture.md` and start the exercises.

## Run locally

**Requirements:** [Bazelisk](https://github.com/bazelbuild/bazelisk) (recommended) or Bazel 7.1.0, GCC/G++, Python 3.

```bash
git clone https://github.com/GreenRuslan/bazel_course.git
cd bazel_course
bazel build //...
```

## Useful commands

```bash
# Build everything
bazel build //...

# Run a specific target
bazel run //app/server:main

# List all targets in a package
bazel query //app/server:all

# Show all dependencies of a target
bazel query "deps(//app/server:main)"

# Run tests
bazel test //...
```
