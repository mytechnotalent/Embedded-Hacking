---
name: embedded-python-standard
description: Use ONLY when writing, editing, or reviewing Python in a picokit project (gateway/, scripts/, tests/). Enforces full PEP 8 via pycodestyle at 79 columns, a module/class/function docstring with Parameters and Returns sections on everything, no function over 8 executable lines, no blank lines inside a function body, and a ban on em/en dashes. Run the bundled audit_python_standard.py before every commit.
---

# Embedded Python standard (picokit)

This is non-negotiable and identical in every project in the series. There is
no variation between projects.

## Rules

1. Full PEP 8, enforced by `pycodestyle` at `--max-line-length=79`. Four-space
   indent, no tabs, no trailing whitespace, two blank lines between top-level
   definitions.
2. No function or method longer than 8 executable lines. The docstring does not
   count toward the limit. Split the work into a helper.
3. No blank lines inside a function body.
4. Every module, class, function, and method has a docstring. Function and
   method docstrings contain a `Parameters` section and a `Returns` section
   (write `None` when nothing is returned).
5. No U+2013 (en dash) or U+2014 (em dash) anywhere. Use the hyphen-minus `-`.
6. Imports are grouped standard library, third party, local, with one import
   per line.

## Layout

```
gateway/    rylr998.py  codec.py  crypto.py  store.py  tui.py  web/  cli.py
            sim.py
scripts/    run_tests.py  check_coverage.py  build.sh
tests/
```

## Verify

Run the bundled audit from the repository root. Exit 0 is clean; any output is
a failure and must be fixed before the change is complete.

```bash
python3 .opencode/skill/embedded-python-standard/audit_python_standard.py
```
