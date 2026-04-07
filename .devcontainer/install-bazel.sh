#!/bin/bash
set -e

echo "Installing Bazelisk..."

curl -Lo /usr/local/bin/bazel \
  https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64

chmod +x /usr/local/bin/bazel

echo "Bazelisk installed. Bazel version will be read from .bazelversion:"
cat .bazelversion
