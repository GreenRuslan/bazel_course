def _my_repo_impl(ctx):
    ctx.file("BUILD.bazel", content = """
cc_library(
    name = "ssl",
    linkopts = ["-lssl"],
    visibility = ["//visibility:public"],
)
""")

my_repo = repository_rule(
    implementation = _my_repo_impl,
    environ = ["OPENSSL_ROOT_DIR"],
    local = True,
)
