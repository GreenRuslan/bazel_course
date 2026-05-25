# Bazel Query Masterclass: Navigating the Graph

Welcome! In this lecture, we're going to demystify Bazel queries using the **Feynman Technique**. We will strip away the jargon and explain things simply. We will use the workspace we just created as a **training floor** to run these commands ourselves.

Imagine your software project is a giant city. 
*   **Targets** (libraries, binaries, tests) are the buildings.
*   **Dependencies** (`deps`) are the power lines, roads, and plumbing connecting them. You can't build the skyscraper (`app`) until the foundation (`core`) is poured.

When a city gets to be the size of Google or your enterprise, you can't just trace a power line with your eyes. You need a computer system to ask: *"If I turn off this sub-station, which buildings lose power?"* 

That computer system is **`bazel query`**. Let's investigate how the city is built.

---

## 🏗️ Phase 1: Getting the Lay of the Land (Discovery)

When you first join a project, you don't know what exists. Your first question is: **"What packages and targets do we actually have here?"**

*   **Command:** `bazel query '//...'`
*   **Meaning:** The `//...` syntax is a wildcard. It means "recursively search from the workspace root (`//`) through all directories (`...`)."

**Learning Checkpoint:** Run this in your terminal. You'll see a wall of text. It's overwhelming. We need to filter this. Let's use a formatting flag:
*   **Command:** `bazel query '//...' --output=label_kind`
*   **Result:** Now you see `cc_library rule //core:memory` instead of just `//core:memory`. You can immediately read the map and tell what is an application (`*_binary`) and what is a test (`*_test`).

---

## 🔍 Phase 2: Dissecting an Application (`deps`)

Looking at our list, we spot `//app/server:main`. Let's investigate what it takes to build it.

*   **Command:** `bazel query 'deps(//app/server:main)'`
*   **Question Answered:** "What exact ingredients do I need to successfully compile this application?"

**The Problem:** If you run that, Bazel throws hundreds of built-in C++ standard toolchain targets at you. This is pure noise.
*   **The Fix:** `bazel query 'deps(//app/server:main)' --noimplicit_deps`
*   **Result:** It cuts out the invisible default dependencies (like the compiler itself). You will now clearly see `//app/server:main`, `//utils:string_utils`, and transitively `//core:memory`.

---

## 🤖 Phase 3: Isolating the Tooling Layer (`--notool_deps`)

### The Analogy

Imagine a restaurant kitchen. To make a smoothie, you need:
1.  **Ingredients** — fruit, yogurt, ice (these go *into* the smoothie and the customer consumes them).
2.  **A blender** — it transforms the ingredients, but nobody eats the blender.

In Bazel, the same split exists:
*   **`deps`** = ingredients. Code that gets linked into and shipped with your final binary.
*   **`tools`** = the blender. Programs that run *during the build* to produce something, but are never part of the final binary.

### Where This Shows Up in Our Workspace

Open `app/server/BUILD.bazel`. Look at the `genrule`:

```python
genrule(
    name = "generate_config",
    srcs = ["config.template"],         # The raw ingredient (input file)
    outs = ["generated_config.h"],       # The smoothie (output file)
    tools = ["//tools/codegen:generator"], # The BLENDER (runs during build, not shipped)
    cmd = "$(location //tools/codegen:generator) $< $@",
)
```

The `generator` script reads the template and spits out a `.h` file. But the generator itself is never compiled into the server binary. It's the blender — it did its job and goes back on the shelf.

### See It Yourself

**Step 1 — All deps (no implicit):**
*   `bazel query 'deps(//app/server:main)' --noimplicit_deps`
*   You'll see **13 results**, including `//tools/codegen:generator` and `//tools/codegen:generator.py`. The blender is in the list.

**Step 2 — Strip the tools:**
*   `bazel query 'deps(//app/server:main)' --noimplicit_deps --notool_deps`
*   Now you see **11 results**. The two `//tools/codegen:*` entries vanish.
*   **What happened?** `--notool_deps` tells Bazel: *"When you walk the dependency graph, skip any edge that goes through a `tools = [...]` attribute."* Since `generator` was declared in `tools`, not `deps`, it gets pruned.

**The takeaway:** `--notool_deps` answers: *"If I unzip the final binary and look inside, what code is actually there?"* The blender stays in the kitchen.

*(There's a related flag `--nohost_deps` for cross-compilation — we'll explore the difference in Phase 12.)*

---

## 💥 Phase 4: The Ripple Effect (`rdeps`)

You have been tasked with updating `//core:memory`. But you are terrified: **"If I change this memory tracker, who else's code will break?"**

*   **Command:** `bazel query 'rdeps(//..., //core:memory)' --keep_going`
*   **Question Answered:** "Who depends on me?"
*   **Result:** `rdeps(universe, target)`. We search the whole repository (`//...`) for targets overlapping with `//core:memory`. It reveals `//utils:string_utils`, `//app/server:main`, `//tests:server_test`, and `//tests:shared_test`. Yes, an application in an entirely different folder depends on your core module transitively!

**⚠️ Why `--keep_going`?** Notice we *must* use `--keep_going` here. The `//...` universe means Bazel tries to resolve *every* package, including our intentionally broken `//broken_package`. Without `--keep_going`, Bazel would abort the entire query with an error about `//core:fake_target_for_lecture`. With it, Bazel prints the error, warns you that results may be incomplete, and still gives you the answer for the healthy parts of the repo. (We'll cover `--keep_going` in depth in Phase 9.)

---

## 🗺️ Phase 5: Pathfinding Visualizations (`somepath` & `allpaths`)

You notice that `//tests:server_test` depends on `//core:memory`. But *how*? You open `tests/BUILD.bazel`, but `core:memory` is entirely absent from the file!

*   **Command:** `bazel query 'somepath(//tests:server_test, //core:memory)'`
*   **Question Answered:** "How in the world did this dependency get here? Show me one valid chain."
*   **Result:** The query traces a line: `//tests:server_test` -> `//utils:string_utils` -> `//core:memory`.

**Make it Visual:**
Sometimes there isn't just one path. There are dozens (`allpaths`). Text is boring. Let's draw the map!
*   **Command:** `bazel query 'allpaths(//tests:server_test, //core:memory)' --output=graph > graph.in`
*   **Result:** You've generated a file ready to be rendered by Graphviz (`dot`), creating a beautiful architecture diagram to present to your team.

---

## 🧲 Phase 6: Needles in a Haystack (`kind` & `attr`)

You are doing a database migration and only care about Python files. Or maybe you want a list of failing, unstable tests.

### Filter by Kind

`kind` filters by the **rule type** (the function name in the BUILD file):

*   **Command:** `bazel query 'kind("py_.*", //...)'`
*   *Returns:* Only `py_binary`, `py_library`, `py_test` targets — 5 results in our workspace.

### Filter by Attribute

`attr` is far more powerful than it first appears. It can filter on **any attribute** a rule declares. The syntax is `attr("attribute_name", "regex_pattern", target_set)`.

*   **By tags:** `bazel query 'attr("tags", "flaky", //tests/...)'`
    *   *Returns:* `//tests:server_test` — the only test tagged `"flaky"`.
    *   *Use case:* Your CI pipeline can dynamically query for flaky tests and run them in isolation!

*   **By deps:** `bazel query 'attr("deps", "//core:memory", //...)'`
    *   *Returns:* `//tests:shared_test` and `//utils:string_utils` — every target that **directly** lists `//core:memory` in its `deps`.
    *   *Key insight:* This is different from `rdeps`! `rdeps` finds *transitive* dependents (anything that depends on `memory` through a chain). `attr("deps", ...)` only finds targets that **literally wrote** `//core:memory` in their BUILD file.

*   **By size:** `bazel query 'attr("size", "large", //tests/...)'`
    *   *Returns:* `//tests:server_test` — the only test with `size = "large"`.
    *   *Use case:* Find all heavyweight tests to schedule them on beefier CI machines.

*   **By visibility:** `bazel query 'attr("visibility", "//app/server", //core/...)'`
    *   *Returns:* `//core:secret_module` — the only target whose visibility rule mentions `//app/server`.
    *   *Use case:* Audit which targets have restricted visibility and who they're restricted to.

*   **By srcs pattern:** `bazel query 'attr("srcs", "\\.py", //...)'`
    *   *Returns:* All 5 targets whose `srcs` include `.py` files.
    *   *Use case:* During a Python 2→3 migration, find every rule that touches Python source files.

---

## 🧮 Phase 7: Set Operations (Venn Diagrams)

Bazel queries support mathematical set logic: `intersect`, `except`, and `union`. Let's use our new `//tests:shared_test`!

*   **Intersection:** *"What tool logic is shared between the C++ server and the new shared test?"*
    *   **Command:** `bazel query 'deps(//app/server:main) intersect deps(//tests:shared_test)'`
*   **Exception:** *"Show me everything the test runs, EXCEPT the core memory module that we know is safe."*
    *   **Command:** `bazel query 'deps(//tests:shared_test) except //core:memory'`
*   **Union:** *"Group all the python client dependencies and the python tests into one single list."*
    *   **Command:** `bazel query 'deps(//app/client:cli) union deps(//tests:client_test)'`

---

## 🕵️ Phase 8: Inspecting Configuration (`buildfiles`)

Sometimes a Macro in Bazel hides a bunch of magic. What ACTUAL Starlark files are determining how the server builds?

*   **Command:** `bazel query 'buildfiles(//app/server:main)'`
*   **Result:** It returns `//app/server:BUILD.bazel` — the file that defines this target. In a larger project with macros and `.bzl` imports, this command would also list every `.bzl` file loaded by that BUILD file, revealing hidden macro layers you might not know about.

*   **Advanced Peeking:** `bazel query '//app/server:main' --output=build`
    *   *Result:* Instantly prints the raw, *evaluated* rule definition directly to your terminal. This shows you the rule as Bazel sees it after macros have expanded — useful for seeing what a macro actually generated without opening your editor.

---

## 🛡️ Phase 9: Surviving a Broken Monorepo (`--keep_going`)

A coworker in another department just pushed terrible code to `//broken_package`.

If you run `bazel query '//...'` right now, Bazel will crash! It throws an error: `//broken_package:bad_target has an invalid dependency`.
Since you are working in `//app/server`, you shouldn't be blocked!

*   **Command:** `bazel query '//...' --keep_going`
*   **Question Answered:** "Please ignore the broken parts of the city and give me whatever answers you can salvage." 
*   **Result:** Bazel skips the broken package dependencies gracefully, allowing you to see the rest of your targets (including `//app/server:main`).

---

## 🔒 Phase 10: The Access Control List (`visible`)

Bazel uses `visibility` to stop people from depending on private implementation details. In our `//core` package, we've hidden a target called `//core:secret_module`. It is strictly marked `visibility = ["//app/server:__pkg__"]`. 

What happens if you are working in the `//tests` folder and want to see if you can depend on it?

*   **Command:** `bazel query 'visible(//tests:shared_test, //core:*)'`
*   **Question Answered:** "Out of all targets in the `//core` package, which ones am I legally allowed to depend on from my test?"
*   **Result:** The query returns `//core:memory` and `//core:base_logger`. Notice that `//core:secret_module` is completely absent from the results, even though it exists in `//core`!

*   **Proof:** If we run `bazel query 'visible(//app/server:main, //core:*)'`, our `server` (which is whitelisted) *will* see `//core:secret_module` in the output!

---

## 📦 Phase 11: Zooming Out to Packages (`--output=package`)

Targets are useful, but sometimes you want a bird's-eye view: **"Which folders (packages) in this repository actually contain build rules?"**

*   **Command:** `bazel query 'deps(//app/server:main)' --noimplicit_deps --output=package`
*   **Question Answered:** "What *folders* in this project are involved in building the server?"
*   **Result:** Instead of listing individual targets like `//core:memory` and `//utils:string_utils`, Bazel collapses them to package names: `core`, `utils`, `app/server`, `tools/codegen`. This instantly tells you which directories a new teammate needs to understand.

**Learning Checkpoint:** Compare this with other output formats:
*   `bazel query 'deps(//tests:shared_test)' --noimplicit_deps --output=package`
*   *Result:* `core`, `utils`, `tests` (plus `@platforms//os` from the platform layer). In seconds, you've mapped the project footprint for any target without reading a single `BUILD` file.

---

## 🌐 Phase 12: Cross-Compilation Clarity (`--nohost_deps`)

### The Analogy

You own a toy company. Your **factory is in Germany** (the *host*), but the toys ship to **customers in Japan** (the *target*). To make a toy, the factory needs:
1.  **Plastic, paint, screws** — these go *into the toy* and arrive in Japan. (= regular `deps`)
2.  **An injection-molding machine** — it stays bolted to the factory floor in Germany. The customer never sees it. (= host dependency)

`--nohost_deps` says: *"Only show me things that travel to Japan. Leave out the German factory equipment."*

### But Wait — Isn't That the Same as `--notool_deps`?

Great question! In our workspace, if you run both side by side:

```
bazel query 'deps(//app/server:main)' --noimplicit_deps --notool_deps   # ← 11 results
bazel query 'deps(//app/server:main)' --noimplicit_deps --nohost_deps   # ← 11 results (identical!)
```

You get the **exact same output**. Both strip `//tools/codegen:generator`. So what's the difference?

The answer: they look at **different labels on the same edge** in the dependency graph.

### The Precise Difference (in Plain English)

Every dependency edge in Bazel has two independent properties:

| Property | What it describes | Which flag reads it |
|---|---|---|
| **Attribute name** | Was this declared in `tools = [...]` or `deps = [...]`? | `--notool_deps` |
| **Configuration** | Will this be compiled for my laptop (host/exec) or for the target device? | `--nohost_deps` |

In built-in rules like `genrule`, these two properties happen to **overlap perfectly**: everything in `tools` is also configured for the host. So both flags prune the same things.

### When They Diverge

They split apart when someone writes a **custom Starlark rule** like this:

```python
my_custom_rule = rule(
    attrs = {
        "deps": attr.label_list(),
        "compiler": attr.label(cfg = "exec"),   # <-- KEY LINE
    },
)
```

Here, `compiler` lives in the **regular attributes** (not `tools`), but `cfg = "exec"` tells Bazel to compile it for the host machine.

*   `--notool_deps` will **NOT** filter it out (it's not in `tools`).
*   `--nohost_deps` **WILL** filter it out (it's configured for the host/exec platform).

### The Command

*   **Command:** `bazel query 'deps(//app/server:main)' --noimplicit_deps --nohost_deps`
*   **Question Answered:** "If I'm cross-compiling (say, building for Android on my Mac), what code actually ends up on the device?"
*   **Result:** In our workspace, identical to `--notool_deps`. But in a real-world monorepo with custom rules, `--nohost_deps` is the *more precise* filter because it looks at how Bazel actually *configures* the target, not just which BUILD attribute it was written in.

**Rule of thumb:** Use `--notool_deps` when you care about the *intent of the BUILD author* ("what did they put in `tools`?"). Use `--nohost_deps` when you care about the *actual compilation platform* ("what runs on my laptop vs. what runs on the device?").

---

## 🤖 Phase 13: Machine-Readable Output for CI (`--output=xml/json/proto`)

So far, we've used output modes designed for humans. But what if you need to feed query results into a CI pipeline, a monitoring dashboard, or a custom analysis script?

*   **Command:** `bazel query '//app/...' --output=xml`
*   **Question Answered:** "Give me the full structured metadata of every target so my script can parse it."
*   **Result:** Each target comes wrapped in XML with all its attributes: `srcs`, `deps`, `visibility`, `tags`, `size`—everything the BUILD file defines, plus rule kind and location info.

**Learning Checkpoint:** Try these variations:
*   `bazel query 'kind("cc_test", //tests/...)' --output=proto`
    *   *Use case:* Protocol Buffers are the most compact format. Feed this to a Go or Python service that monitors test health.
*   `bazel query 'attr("tags", "flaky", //tests/...)' --output=xml`
    *   *Use case:* A nightly CI job parses this XML to build a report of all flaky tests, their sizes, and their dependency counts, automatically filing tickets for test owners.

These machine-readable formats turn `bazel query` from a developer's debugging tool into the backbone of your **build infrastructure automation**.

---

## 🧬 Phase 14: Composing Complex Queries (Putting It All Together)

So far, every phase used one or two query functions at a time. But the real power of `bazel query` is that **every function returns a set, and every function accepts a set**. You can nest them, chain them, and combine them just like LEGO bricks. Let's build some real investigations.

### Investigation 1: "What C++ libraries does the server need that the Python client doesn't?"

You're splitting the monorepo into two deployable units. You need to know what the server depends on that the client doesn't — those are the libraries you'll need to package with the server.

*   **Command:** `bazel query 'kind("cc_library", deps(//app/server:main) except deps(//app/client:cli))' --noimplicit_deps`

**Reading it inside-out:**
1.  `deps(//app/server:main)` → all server ingredients
2.  `deps(//app/client:cli)` → all client ingredients
3.  `... except ...` → things the server has that the client *doesn't*
4.  `kind("cc_library", ...)` → from that difference, keep only C++ libraries

*   **Result:** `//core:memory` and `//utils:string_utils`. The client is pure Python — it shares zero C++ code with the server.

---

### Investigation 2: "Which tests will break if I change `//core:memory`?"

Before you refactor a core module, you want to know the blast radius — but only the *tests*, not every transitive dependent.

*   **Command:** `bazel query 'kind(".*_test", rdeps(//..., //core:memory))' --keep_going`

**Reading it inside-out:**
1.  `rdeps(//..., //core:memory)` → everything in the repo that depends on `memory` (with `--keep_going` to survive `broken_package`)
2.  `kind(".*_test", ...)` → from that set, keep only test rules

*   **Result:** `//tests:server_test` and `//tests:shared_test`. These are the tests you should run after your refactor.

---

### Investigation 3: "What shared C++ libraries do the server and the shared_test have in common?" (using `let`)

This is the same question as Phase 7's `intersect`, but now we use a **`let` variable** to avoid computing the intersection from scratch each time, and we filter down to only `cc_library` results.

*   **Command:** `bazel query 'let shared = deps(//app/server:main) intersect deps(//tests:shared_test) in kind("cc_library", $shared)' --noimplicit_deps`

**What's new here:**
-  `let shared = ... in ...` defines a variable `$shared` so the intersection is computed once and reused.
-  `kind("cc_library", $shared)` narrows the intersection to only C++ library rules.

*   **Result:** `//core:memory` and `//utils:string_utils` — the C++ foundation shared by both.

---

### Investigation 4: "What source files are unique to the server build?"

You're writing a minimal Dockerfile and need to know which source files to copy in for the server that aren't needed by the shared_test (which you build separately).

*   **Command:** `bazel query 'kind("source file", deps(//app/server:main) except deps(//tests:shared_test))' --noimplicit_deps`

**Reading it inside-out:**
1.  `deps(//app/server:main) except deps(//tests:shared_test)` → deps unique to the server
2.  `kind("source file", ...)` → from those, keep only actual source files (not rules)

*   **Result:** `//app/server:config.template`, `//app/server:main.cc`, and `//tools/codegen:generator.py`. These are the files you'd need to `COPY` into a server-only Docker image.

---

### Investigation 5: "Find the C++ rules in `//core` that the server depends on" (using `filter`)

Instead of `deps(...) intersect //core/...` (which forces Bazel to load the entire `//core` package tree), `filter` applies a regex to labels — much faster in large repos.

*   **Command:** `bazel query 'kind("cc_.*", filter("//core", deps(//app/server:main)))' --noimplicit_deps`

**Reading it inside-out:**
1.  `deps(//app/server:main)` → all server deps
2.  `filter("//core", ...)` → keep only targets whose label contains `//core`
3.  `kind("cc_.*", ...)` → from those, keep only C++ rules

*   **Result:** `//core:memory` — the only C++ rule in core that the server uses.

---

### The Pattern

Every complex query follows the same mental model:

```
outer_filter(  middle_filter(  inner_set_operation  )  )
```

1.  **Inner:** Build your raw set (`deps`, `rdeps`, `intersect`, `except`, `union`)
2.  **Middle:** Narrow by label pattern (`filter`) or attribute (`attr`)
3.  **Outer:** Narrow by rule type (`kind`) or visibility (`visible`)

Once you see this pattern, you can compose arbitrarily complex investigations in a single command.

---

# 📚 Comprehensive Cheatsheet: Query Functions

### Core Graph Traversal

| Query Function / Syntax | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- |
| `deps(X)` or `deps(X, depth)` | "What ingredients are required to build X?" | When a build fails unexpectedly, or a target takes 40 minutes to compile and you suspect bloat. The optional `depth` integer limits how many hops to follow (e.g., `deps(X, 1)` = direct deps only). |
| `rdeps(U, X)` or `rdeps(U, X, depth)` | "If I break X, what inside U will fail?" | When modifying a core API or deleting a utility, you must know every target in the repository (`U = //...`) that uses it. Optional `depth` limits reverse hops. |
| `allrdeps(X)` or `allrdeps(X, depth)` | "Same as `rdeps`, but the universe is set by `--universe_scope`." | **SkyQuery only.** When you've pre-set a universe scope and want reverse deps without re-typing it. |
| `somepath(X, Y)` | "Why does X depend on Y? Give me one valid path." | When you see a weird dependency (like a frontend UI depending on a backend database driver) and need to find the node responsible. |
| `allpaths(X, Y)` | "What are *all* the ways X relies on Y?" | When you want to visualize a complex dependency web, usually exported to Graphviz (`--output=graph`). |
| `same_pkg_direct_rdeps(X)` | "What targets in X's own package directly depend on X?" | When refactoring a target and you need to know the local impact within the same `BUILD` file. |

### Filtering & Selection

| Query Function / Syntax | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- |
| `kind(regex, X)` | "Out of this group, show me only the rules of this specific type." | Filtering for specific rule types across a massive codebase, like isolating only Python binaries or only test targets. |
| `attr(name, regex, X)` | "Show me targets where a specific attribute matches my search string." | Finding all targets marked with a specific `size`, `timeout`, `tags`, `deps`, or `visibility`. |
| `filter(regex, X)` | "Filter the target *labels* themselves by a regex pattern." | A faster alternative to `intersect` when filtering deps by package name, e.g., `filter("//bar", deps(//foo))` instead of `deps(//foo) intersect //bar/...`. Also useful for filtering by file extension: `filter("\\.cc$", deps(//foo))`. |
| `visible(X, Y)` | "Filter the list of targets in Y to only show the ones that X has permission to see." | Preventing dependency graphs from breaking by validating access controls *before* writing code. |
| `labels(attr, X)` | "What targets are listed in attribute `attr` of rule X?" | When you want just the `srcs`, `deps`, or `data` of a rule without the transitive closure. e.g., `labels(srcs, //foo:bar)` returns the direct source files. |
| `tests(X)` | "Expand `test_suite` rules and show me the individual test targets." | When you need the actual flat list of tests that `bazel test X` would run (test_suites are expanded, tag/size filters applied). |
| `executables(X)` | "Which targets in X can be directly run with `bazel run`?" | Finding all runnable binaries (but not tests — combine with `tests()` for those). |
| `some(X)` or `some(X, count)` | "Give me an arbitrary sample of `count` targets from the set." | When debugging large result sets and you just want a quick representative sample. |

### Package & Build File Introspection

| Query Function / Syntax | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- |
| `buildfiles(X)` | "Which `BUILD` and `.bzl` files are required to parse X?" | When you need to understand which Bazel macros or extensions are secretly affecting your target's build config. |
| `rbuildfiles(path, ...)` | "Which packages load this `.bzl` file?" (inverse of `buildfiles`) | **SkyQuery only.** When you modify a Starlark macro and need to know which BUILD files are affected. |
| `loadfiles(X)` | "Which `.bzl` files are `load()`-ed by X's package?" | Similar to `buildfiles` but focuses exclusively on the `load()` statements, not the BUILD file itself. |
| `siblings(X)` | "What other targets live in the same package as X?" | Quick way to see everything defined alongside a target without knowing the package path. |

### Set Operations & Composition

| Query Function / Syntax | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- |
| `X intersect Y` (or `X ^ Y`) | "What do X and Y have in common?" | Finding shared foundational libraries between two completely different applications to see where code is successfully reused. |
| `X except Y` (or `X - Y`) | "What is in X that is NOT in Y?" | Debugging why App A builds fine but App B fails. (e.g., App A has a dependency App B is missing). |
| `X union Y` (or `X + Y`) | "Combine X and Y together into one list." | When you want to run an analysis or apply a tag to a custom grouping of disjoint dependencies. |
| `set(a b c ...)` | "Create a target set from a whitespace-separated list of labels." | Piping shell-manipulated target lists back into `bazel query`. e.g., `bazel query "kind(cc_binary, set($(<targets.txt)))"`. |
| `let name = expr1 in expr2` | "Define a variable to reuse in a complex query expression." | Shortening repetitive queries: `let v = deps(//foo) in kind("cc_library", $v) intersect kind("cc_library", deps(//bar))`. |

---

# 🛠️ Comprehensive Cheatsheet: Query Flags

Query Functions control **what** you search for. Flags control **how** Bazel searches and **what format** the response takes.

### Output Format Flags

| Flag | Allowed Values | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- | :--- |
| `--output` | `label` (default) <br> `label_kind` <br> `build` <br> `package` <br> `location` <br> `graph` <br> `xml` <br> `proto` <br> `streamed_proto` <br> `textproto` <br> `streamed_jsonproto` <br> `minrank` <br> `maxrank` | **"How do you want to consume this data?"** <br> Human-readable text, raw BUILD code, visual graphs, machine-readable protos, or rank-based analysis. | `label_kind`: see rule types. `build`: audit macro expansion. `location`: grep-style file:line output. `graph`: Graphviz diagrams. `xml`/`proto`/`streamed_jsonproto`: CI automation. `minrank`/`maxrank`: measure graph depth. |

**Output format details:**

| `--output` value | What it produces |
| :--- | :--- |
| `label` | Target labels, one per line (default). |
| `label_kind` | Rule type + label (e.g., `cc_library rule //core:memory`). |
| `build` | The evaluated BUILD rule definition as Bazel sees it after macro expansion. |
| `package` | Package (directory) names only, deduped. |
| `location` | `file:line:col` + kind + label. Grep-compatible for editor integration. |
| `graph` | Graphviz DOT format. Pipe to `dot -Tpng` for visual diagrams. |
| `xml` | Full structured XML with all attributes, rule-inputs, and rule-outputs. |
| `proto` | Binary Protocol Buffer (`QueryResult` message). |
| `streamed_proto` | Length-delimited stream of `Target` protocol buffers (avoids protobuf size limits). |
| `textproto` | Text-format Protocol Buffer (human-readable proto). |
| `streamed_jsonproto` | Newline-delimited JSON (ndjson) stream of `Target` messages. |
| `minrank` | Targets ranked by shortest path from a root node. |
| `maxrank` | Targets ranked by longest path from a root node. Useful for finding the critical build path. |

### Dependency Filtering Flags

| Flag | Allowed Values | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- | :--- |
| `--noimplicit_deps` | Boolean | **"Should we hide compiler magic?"** <br> "Implicit" dependencies are automatically injected by Bazel (e.g., standard C++ toolchains, Java JDKs). | When analyzing your team's actual written source code architecture. You rarely care about the internal C++ compiler dependencies. |
| `--notool_deps` | Boolean | **"Should we hide execution-time tools?"** <br> "Tool" dependencies are things run *during* the build (like code generators or custom compilers) rather than linked into the final binary. | When you want a purely *runtime* architecture view of your production application. |
| `--nohost_deps` | Boolean | **"Should we hide host environment tools?"** <br> Similar to tool deps, but specifically targets dependencies built for the machine running Bazel (the "host/exec"), rather than the target machine. | When cross-compiling (e.g., building Android apps on a Mac) and analyzing target-only dependencies. |

### Error Handling & Scope Flags

| Flag | Allowed Values | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- | :--- |
| `--keep_going` | Boolean | **"Should we fail fast or push through errors?"** | When a coworker pushed a broken BUILD file in a random folder, but you still need to query your completely unrelated package. |
| `--universe_scope` | Comma-separated target patterns | **"What is the boundary of the universe for SkyQuery?"** | Required for `allrdeps` and `rbuildfiles`. Pre-loads the transitive closure of given patterns as the search universe. |
| `--infer_universe_scope` | Boolean | **"Automatically infer `--universe_scope` from my query expression."** | Convenience flag for SkyQuery so you don't have to manually specify the universe scope. |
| `--strict_test_suite` | Boolean | **"Should non-test targets inside `test_suite` be an error?"** | When using `tests()` and you want strict validation that test suites only reference actual test targets. |

### Result Ordering Flags

| Flag | Allowed Values | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- | :--- |
| `--order_output` | `auto` (default) <br> `no` <br> `deps` <br> `full` | **"In what order should results be printed?"** <br> `auto`: lexicographic (or deps for `somepath`). `no`: arbitrary (fastest). `deps`: topological. `full`: fully deterministic. | Use `no` for maximum speed in scripts. Use `deps` when order matters for visualization. Use `full` when you need reproducible output for diffing. |

### Graph & XML Sub-Options

| Flag | The Human Question It Answers | When to Use This (Real-world Case) |
| :--- | :--- | :--- |
| `--graph:factored` / `--nograph:factored` | "Should topologically equivalent nodes be merged in the graph output?" | Default is factored (merged). Use `--nograph:factored` for raw, unfactored graphs that are easier to parse with tools like `grep`. |
| `--graph:node_limit n` | "How long can node labels be before truncation?" | Default is 1024 chars (GraphViz limit). Set to `-1` to disable truncation. |
| `--xml:line_numbers` / `--noxml:line_numbers` | "Should XML output include line numbers in locations?" | Disable to get stable output for diffing across edits. |
| `--xml:default_values` / `--noxml:default_values` | "Should XML output include attributes at their default values?" | Enable to see *every* attribute, even ones not explicitly written in the BUILD file. |

Happy querying! By mastering these functions and flags, you transition from someone who just "runs builds" to a true architect who understands the lifeblood of the software graph.
