#!/usr/bin/env python3
"""Audit owned Python against the strict embedded Python standard.

Checks module/class/function docstrings with Parameters and Returns sections,
the eight-executable-line function limit, the no-blank-line-in-function-body
rule, the em/en dash ban, and full PEP 8 via pycodestyle at 79 columns. Run
from the repository root; exit zero means the tree is clean.
"""
import ast
import subprocess
import sys
from pathlib import Path

OWNED_ROOTS = ("gateway", "scripts", "tests")
EXCLUDED_PARTS = (".opencode", "build", ".venv", "node_modules", "__pycache__")
EN_DASH = "\u2013"
EM_DASH = "\u2014"
MAX_LINE = "79"


def _python_files() -> list[Path]:
    """
    Find owned Python files.

    Parameters
    ----------
    None

    Returns
    -------
    list[pathlib.Path]
        Owned Python paths under the owned roots.
    """
    files = []
    for root in OWNED_ROOTS:
        files.extend(Path(root).rglob("*.py"))
    return sorted(path for path in files if _is_owned(path))


def _is_owned(path: Path) -> bool:
    """
    Determine whether a path is in the audit scope.

    Parameters
    ----------
    path : pathlib.Path
        Candidate source path.

    Returns
    -------
    bool
        True when the path is owned Python.
    """
    name = str(path)
    return not any(part in name for part in EXCLUDED_PARTS)


def _doc_errors(node: ast.AST) -> list[str]:
    """
    Check one module, class, or function docstring.

    Parameters
    ----------
    node : ast.AST
        Module, function, or class syntax node.

    Returns
    -------
    list[str]
        Documentation error messages.
    """
    if isinstance(node, ast.Module):
        return [] if ast.get_docstring(node) else ["missing module docstring"]
    doc = ast.get_docstring(node, clean=False) or ""
    required = ("Parameters", "Returns")
    return [f"missing {item} section" for item in required if item not in doc]


def _walk_doc_errors(tree: ast.AST, path: Path) -> list[str]:
    """
    Collect missing documentation section errors.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.
    path : pathlib.Path
        Source path used in diagnostics.

    Returns
    -------
    list[str]
        Documentation diagnostics.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)
    nodes = [tree] + [item for item in ast.walk(tree)
                      if isinstance(item, kinds)]
    return [f"{path}: {type(node).__name__}: {error}"
            for node in nodes for error in _doc_errors(node)]


def _blank_function_lines(path: Path) -> list[str]:
    """
    Find blank physical lines inside Python function spans.

    Parameters
    ----------
    path : pathlib.Path
        Python source path.

    Returns
    -------
    list[str]
        Diagnostics for blank lines inside function spans.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    tree = ast.parse("\n".join(lines), filename=str(path))
    spans = _function_spans(tree)
    doc_lines = _docstring_lines(tree)
    return _blank_lines(path, lines, spans, doc_lines)


def _function_spans(tree: ast.AST) -> list[tuple[int, int]]:
    """
    Return executable spans for Python functions.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.

    Returns
    -------
    list[tuple[int, int]]
        Function start and end line pairs.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef)
    return [(node.lineno, node.end_lineno)
            for node in ast.walk(tree) if isinstance(node, kinds)]


def _docstring_lines(tree: ast.AST) -> set[int]:
    """
    Return physical lines occupied by Python docstrings.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.

    Returns
    -------
    set[int]
        One-based docstring line numbers.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)
    nodes = [tree] + [node for node in ast.walk(tree)
                      if isinstance(node, kinds)]
    return {line for node in nodes for line in _node_doc_lines(node)}


def _node_doc_lines(node: ast.AST) -> range:
    """
    Return the line range of one node's docstring.

    Parameters
    ----------
    node : ast.AST
        Module, class, or function node.

    Returns
    -------
    range
        One-based docstring line range, or an empty range.
    """
    body = getattr(node, "body", [])
    first = body[0] if body else None
    value = getattr(first, "value", None)
    has_doc = isinstance(value, ast.Constant) and isinstance(value.value, str)
    return range(first.lineno, first.end_lineno + 1) if has_doc else range(0)


def _blank_lines(path: Path, lines: list[str], spans: list[tuple[int, int]],
                 doc_lines: set[int]) -> list[str]:
    """
    Format blank executable-line diagnostics.

    Parameters
    ----------
    path : pathlib.Path
        Source path.
    lines : list[str]
        Source lines.
    spans : list[tuple[int, int]]
        Function spans.
    doc_lines : set[int]
        Docstring line numbers.

    Returns
    -------
    list[str]
        Blank-line diagnostics.
    """
    return [f"{path}:{number}: blank line in function"
            for number, line in enumerate(lines, 1)
            if number not in doc_lines and not line.strip()
            and any(start < number < end for start, end in spans)]


def _length_errors(tree: ast.AST, path: Path) -> list[str]:
    """
    Find functions whose executable body exceeds eight lines.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.
    path : pathlib.Path
        Source path used in diagnostics.

    Returns
    -------
    list[str]
        Function-length diagnostics.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef)
    return [f"{path}: {node.name}: {len(body)} executable lines"
            for node in ast.walk(tree)
            if isinstance(node, kinds)
            for body in [_executable_body(node)]
            if len(body) > 8]


def _executable_body(node: ast.AST) -> list[ast.AST]:
    """
    Return a function body excluding its docstring.

    Parameters
    ----------
    node : ast.AST
        Function syntax node.

    Returns
    -------
    list[ast.AST]
        Executable body statements.
    """
    has_doc = ast.get_docstring(node) is not None
    return node.body[1:] if has_doc else node.body


def _dash_errors(path: Path) -> list[str]:
    """
    Find em/en dashes in one file.

    Parameters
    ----------
    path : pathlib.Path
        Python source path.

    Returns
    -------
    list[str]
        Dash diagnostics.
    """
    text = path.read_text(encoding="utf-8")
    errors = []
    if EN_DASH in text:
        errors.append(f"{path}: en dash U+2013 present")
    if EM_DASH in text:
        errors.append(f"{path}: em dash U+2014 present")
    return errors


def _style_errors(files: list[Path]) -> list[str]:
    """
    Run pycodestyle over the owned files.

    Parameters
    ----------
    files : list[pathlib.Path]
        Owned Python paths.

    Returns
    -------
    list[str]
        PEP 8 diagnostics.
    """
    if not files:
        return []
    command = [sys.executable, "-m", "pycodestyle",
               f"--max-line-length={MAX_LINE}"]
    command.extend(str(path) for path in files)
    result = subprocess.run(command, capture_output=True, text=True)
    return [line for line in result.stdout.splitlines() if line.strip()]


def _file_errors(path: Path) -> list[str]:
    """
    Audit one Python file.

    Parameters
    ----------
    path : pathlib.Path
        Python source path.

    Returns
    -------
    list[str]
        All diagnostics for the file.
    """
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    errors = _walk_doc_errors(tree, path) + _blank_function_lines(path)
    return errors + _length_errors(tree, path) + _dash_errors(path)


def main() -> int:
    """
    Audit all owned Python tooling.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero for a clean audit, otherwise one.
    """
    files = _python_files()
    errors = [error for path in files for error in _file_errors(path)]
    errors += _style_errors(files)
    print("\n".join(errors))
    return int(bool(errors))


if __name__ == "__main__":
    raise SystemExit(main())
