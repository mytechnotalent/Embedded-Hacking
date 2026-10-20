---
name: embedded-c-standard
description: Use ONLY when writing, editing, or reviewing C firmware in a picokit project (firmware/src, firmware/include, firmware/tests). Enforces the strict C standard: no function over 8 executable lines, no blank lines inside a function body, a Doxygen block with @brief/@param/@return on every function, a fixed module layout and naming scheme, and a ban on em/en dashes. Run the bundled audit_c_standard.py before every commit.
---

# Embedded C standard (picokit)

This is non-negotiable and identical in every project in the series. There is
no variation between projects. If a rule is inconvenient, refactor; do not
exempt.

## Rules

1. No function longer than 8 executable lines. The audit counts every
   non-blank, non-comment, non-brace line in the body, including continuation
   lines of a wrapped statement. Split the work into a `static` helper.
2. No blank lines inside a function body. Separate logical steps with a
   comment, never with a blank line.
3. Documentation lives in the header. Every function declaration in a `.h`
   carries a Doxygen block with `@brief`, a `@param` for each parameter
   (`@param void No parameters.` when there are none), and an `@return`. A
   `static` function, which has no declaration, carries its Doxygen block in
   the `.c`. Never duplicate the block in the `.c` for an exported function.
4. No U+2013 (en dash) or U+2014 (em dash) anywhere. Use the hyphen-minus `-`.
5. Every source file opens with the MIT header block: `Copyright (c) 2026 Kevin
   Thomas`, `Author`, `Email`, `GitHub`, `File`, `Desc`, `Created`.
6. Every header uses an include guard `#ifndef <MODULE>_H` / `#define
   <MODULE>_H` / `#endif // <MODULE>_H`.
7. Naming: files and modules `snake_case`; functions `module_action`; macros
   `UPPER_SNAKE`; types `name_t`; file-scope statics `g_name`.
8. One responsibility per file: `src/<module>.c` with `include/<module>.h`.
   All hardware access goes through a HAL seam so the host tests can substitute
   mocks.
9. 79-column lines, four-space indent, no tabs.

## Layout

```
firmware/
  src/        <module>.c
  include/    <module>.h
  tests/      host unit tests, mock/, harness/
  CMakeLists.txt
scripts/      run_tests.py  check_coverage.py  build.sh
```

## Verify

Run the bundled audit from the repository root. Exit 0 is clean; any output is
a failure and must be fixed before the change is complete.

```bash
python3 .opencode/skill/embedded-c-standard/audit_c_standard.py
```
