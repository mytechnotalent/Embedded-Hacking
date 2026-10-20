#!/usr/bin/env python3
"""Audit owned C files against the strict embedded C standard.

Checks the no-blank-line-in-function-body rule, the eight-executable-line
function limit, the em/en dash ban, and the documentation placement rules:
a C file documents its static helpers but never its exported definitions,
while a header documents every prototype. Run from the repository root;
exit zero means the tree is clean.
"""
from pathlib import Path

EXCLUDED_PARTS = (
    ".opencode",
    "build",
    "build/",
    "generated/",
    ".venv/",
    "node_modules/",
    "packet_artifact.h",
    "third_party/",
    "test/unity",
    "unity.c",
    "unity_internals.h",
    "ghidra/",
)

EN_DASH = "\u2013"
EM_DASH = "\u2014"
MAX_EXEC_LINES = 8


def _owned_files() -> list[Path]:
    """
    Find owned C and header files.

    Parameters
    ----------
    None

    Returns
    -------
    list[pathlib.Path]
        Owned non-generated C and header paths.
    """
    paths = Path(".").glob("**/*")
    return sorted(path for path in paths if _is_owned(path))


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
        True when the path is an owned C or header file.
    """
    name = str(path)
    if any(part in name for part in EXCLUDED_PARTS):
        return False
    return path.suffix in {".c", ".h"}


def _next_is(line: str, index: int, mark: str) -> bool:
    """
    Test the character following an index against a marker.

    Parameters
    ----------
    line : str
        Source line.
    index : int
        Current character index.
    mark : str
        Single-character marker to compare.

    Returns
    -------
    bool
        True when the next character equals the marker.
    """
    return line[index + 1:index + 2] == mark


def _is_comment_open(line: str, index: int) -> bool:
    """
    Test whether a slash begins a line or block comment.

    Parameters
    ----------
    line : str
        Source line.
    index : int
        Current character index.

    Returns
    -------
    bool
        True when a comment opens at the index.
    """
    return line[index] == "/" and line[index + 1:index + 2] in ("*", "/")


def _comment_open(line: str, index: int) -> tuple[int, str]:
    """
    Advance past a comment opening at the index.

    Parameters
    ----------
    line : str
        Source line.
    index : int
        Index of the opening slash.

    Returns
    -------
    tuple[int, str]
        Next index and the resulting scanner mode.
    """
    if _next_is(line, index, "/"):
        return len(line), "normal"
    return index + 2, "block"


def _literal_mode(char: str) -> str:
    """
    Select the scanner mode for a quote character.

    Parameters
    ----------
    char : str
        Quote character.

    Returns
    -------
    str
        Scanner mode name for the literal.
    """
    return "string" if char == '"' else "char"


def _literal_close(mode: str) -> str:
    """
    Return the closing quote for a literal mode.

    Parameters
    ----------
    mode : str
        Scanner mode name.

    Returns
    -------
    str
        Matching closing quote character.
    """
    return '"' if mode == "string" else "'"


def _on_normal(out: list, line: str, index: int) -> tuple[int, str]:
    """
    Consume one character of ordinary code.

    Parameters
    ----------
    out : list
        Mutable code accumulator.
    line : str
        Source line.
    index : int
        Current character index.

    Returns
    -------
    tuple[int, str]
        Next index and the resulting scanner mode.
    """
    char = line[index]
    if _is_comment_open(line, index):
        return _comment_open(line, index)
    if char in "\"'":
        return index + 1, _literal_mode(char)
    out.append(char)
    return index + 1, "normal"


def _on_block(out: list, line: str, index: int) -> tuple[int, str]:
    """
    Consume one character inside a block comment.

    Parameters
    ----------
    out : list
        Mutable code accumulator.
    line : str
        Source line.
    index : int
        Current character index.

    Returns
    -------
    tuple[int, str]
        Next index and the resulting scanner mode.
    """
    if line[index] == "*" and _next_is(line, index, "/"):
        return index + 2, "normal"
    return index + 1, "block"


def _on_literal(out: list, line: str, index: int,
                mode: str) -> tuple[int, str]:
    """
    Consume one character inside a string or character literal.

    Parameters
    ----------
    out : list
        Mutable code accumulator.
    line : str
        Source line.
    index : int
        Current character index.
    mode : str
        Current literal scanner mode.

    Returns
    -------
    tuple[int, str]
        Next index and the resulting scanner mode.
    """
    char = line[index]
    if char == "\\":
        return index + 2, mode
    if char == _literal_close(mode):
        return index + 1, "normal"
    return index + 1, mode


def _advance(out: list, line: str, index: int, mode: str) -> tuple[int, str]:
    """
    Dispatch one character to its scanner handler.

    Parameters
    ----------
    out : list
        Mutable code accumulator.
    line : str
        Source line.
    index : int
        Current character index.
    mode : str
        Current scanner mode.

    Returns
    -------
    tuple[int, str]
        Next index and the resulting scanner mode.
    """
    if mode == "normal":
        return _on_normal(out, line, index)
    if mode == "block":
        return _on_block(out, line, index)
    return _on_literal(out, line, index, mode)


def _strip_line(line: str, in_block: bool) -> tuple[str, bool]:
    """
    Remove comments and literals from one source line.

    Parameters
    ----------
    line : str
        Source line.
    in_block : bool
        Whether a block comment is already open.

    Returns
    -------
    tuple[str, bool]
        Code-only text and the updated block-comment state.
    """
    out = []
    index = 0
    mode = "block" if in_block else "normal"
    while index < len(line):
        index, mode = _advance(out, line, index, mode)
    return "".join(out), mode == "block"


def _func_name(code: str) -> str:
    """
    Extract the function identifier from a signature head.

    Parameters
    ----------
    code : str
        Comment-free signature line.

    Returns
    -------
    str
        Trailing identifier before the opening parenthesis.
    """
    head = code.split("(")[0].strip()
    return head.split()[-1] if head else ""


def _scan_state(path: Path) -> dict:
    """
    Build the mutable scanner state for one file.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    dict
        Scanner accumulation state.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    state = {"lines": lines, "records": [], "protos": []}
    state.update(depth=0, in_block=False, cont=False, start=None)
    state.update(name="", static=False, sig=0, opened=0, body=[])
    return state


def _scan_line(state: dict, line: str, number: int) -> None:
    """
    Apply one source line to the scanner state.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    line : str
        Raw source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    code, state["in_block"] = _strip_line(line, state["in_block"])
    _scan_open(state, code, number)
    active = state["start"] is not None
    if active and number not in (state["start"], state["opened"]):
        _collect(code, number, state["body"])
    state["depth"] += code.count("{") - code.count("}")
    _scan_close(state, number)
    state["cont"] = line.rstrip().endswith("\\")


def _scan_open(state: dict, code: str, number: int) -> None:
    """
    Open a signature, body, or prototype when one begins.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    code : str
        Comment-free source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    if state["cont"] or state["start"] is not None:
        return
    if state["depth"] != 0:
        return
    _pending(state, code, number)


def _pending(state: dict, code: str, number: int) -> None:
    """
    Resolve a file-scope signature fragment into a record.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    code : str
        Comment-free source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    if state["sig"] == 0 and _proto_line(code):
        _add_proto(state, code, number)
    elif state["sig"] == 0 and _sig_start(code):
        _start_sig(state, code, number)
    elif state["sig"] and "{" in code:
        _start_body(state, number)
    elif state["sig"] and ";" in code:
        _finish_proto(state)


def _add_proto(state: dict, code: str, number: int) -> None:
    """
    Record a single-line function prototype.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    code : str
        Comment-free source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    state["protos"].append({"name": _func_name(code), "start": number})


def _start_sig(state: dict, code: str, number: int) -> None:
    """
    Begin a multi-line signature or same-line definition.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    code : str
        Comment-free source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    state["sig"] = number
    state["name"] = _func_name(code)
    state["static"] = code.lstrip().startswith("static")
    if "{" in code:
        _start_body(state, number)


def _finish_proto(state: dict) -> None:
    """
    Record a wrapped prototype at its terminating semicolon.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.

    Returns
    -------
    None
    """
    state["protos"].append({"name": state["name"], "start": state["sig"]})
    state["sig"] = 0
    state["name"] = ""
    state["static"] = False


def _start_body(state: dict, number: int) -> None:
    """
    Open a new function body at the given line.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    number : int
        One-based opening line number.

    Returns
    -------
    None
    """
    state["start"] = state["sig"]
    state["opened"] = number
    state["body"] = []
    state["sig"] = 0


def _scan_close(state: dict, number: int) -> None:
    """
    Close the current function body when its depth returns to zero.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    if state["start"] is None or state["depth"] > 0:
        return
    if number <= state["start"]:
        return
    state["records"].append(_record(state, number))
    _reset(state)


def _reset(state: dict) -> None:
    """
    Clear the active-function fields of the scanner state.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.

    Returns
    -------
    None
    """
    state["start"] = None
    state["name"] = ""
    state["sig"] = 0
    state["opened"] = 0
    state["static"] = False


def _sig_start(code: str) -> bool:
    """
    Detect the first line of a function definition signature.

    Parameters
    ----------
    code : str
        Comment-free source line.

    Returns
    -------
    bool
        True when the line begins a function definition signature.
    """
    text = code.strip()
    if not text or text.startswith("#") or text.startswith("}"):
        return False
    if "(" not in text or text.endswith(";"):
        return False
    return "=" not in text.split("(")[0]


def _proto_line(code: str) -> bool:
    """
    Detect a complete single-line function prototype.

    Parameters
    ----------
    code : str
        Comment-free source line.

    Returns
    -------
    bool
        True when the line is a function prototype.
    """
    text = code.strip()
    if not text or text.startswith("#") or "(" not in text:
        return False
    if not text.endswith(";") or "=" in text.split("(")[0]:
        return False
    return text.split()[0] not in {"typedef", "return"}


def _collect(code: str, number: int, body: list) -> None:
    """
    Append one executable or declaration line to the body list.

    Parameters
    ----------
    code : str
        Comment-free source line.
    number : int
        One-based line number.
    body : list
        Mutable body accumulator.

    Returns
    -------
    None
    """
    text = code.strip()
    if not text or text in {"{", "}"} or text.startswith("}"):
        return
    body.append((number, text))


def _record(state: dict, end: int) -> dict:
    """
    Build a function record from the scanner state.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    end : int
        Closing brace line number.

    Returns
    -------
    dict
        Function record.
    """
    return {"name": state["name"], "start": state["start"], "end": end,
            "count": len(state["body"]), "static": state["static"]}


def _scan(path: Path) -> dict:
    """
    Scan one file into records and prototypes.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    dict
        Scanner state with records and prototypes.
    """
    state = _scan_state(path)
    for number, line in enumerate(state["lines"], 1):
        _scan_line(state, line, number)
    return state


def _blank_lines(path: Path, records: list) -> list[int]:
    """
    Find blank lines inside function bodies.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    records : list
        Function records for the file.

    Returns
    -------
    list[int]
        One-based blank-line numbers.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    spans = [(r["start"], r["end"]) for r in records]
    return [number for number, line in enumerate(lines, 1)
            if not line.strip() and _inside(number, spans)]


def _inside(number: int, spans: list) -> bool:
    """
    Check whether a line lies inside a function span.

    Parameters
    ----------
    number : int
        One-based source line number.
    spans : list
        Function start and end line pairs.

    Returns
    -------
    bool
        True when the line is inside a function body.
    """
    return any(start < number < end for start, end in spans)


def _block_end(lines: list, start: int) -> int | None:
    """
    Locate the closing line of the doc block above a signature.

    Parameters
    ----------
    lines : list
        Source lines.
    start : int
        One-based signature line number.

    Returns
    -------
    int or None
        Zero-based closing `*/` index, or None when absent.
    """
    index = start - 2
    while index >= 0 and not lines[index].strip():
        index -= 1
    if index < 0 or "*/" not in lines[index]:
        return None
    return index


def _block_lines(lines: list, index: int) -> list:
    """
    Collect the Doxygen block lines ending at an index.

    Parameters
    ----------
    lines : list
        Source lines.
    index : int
        Zero-based closing `*/` index.

    Returns
    -------
    list
        Block lines from opening `/**` through closing `*/`.
    """
    block = []
    while index >= 0:
        block.append(lines[index])
        if "/*" in lines[index]:
            break
        index -= 1
    block.reverse()
    return block


def _is_doxygen(block: list) -> bool:
    """
    Test whether a comment block is a Doxygen block with a brief.

    Parameters
    ----------
    block : list
        Comment block lines from opener through closer.

    Returns
    -------
    bool
        True when the block opens with `/**` and carries `@brief`.
    """
    opener = block[0] if block else ""
    return "/**" in opener and "@brief" in "\n".join(block)


def _doc_block(lines: list, start: int) -> list | None:
    """
    Return the Doxygen block immediately above a signature.

    Parameters
    ----------
    lines : list
        Source lines.
    start : int
        One-based signature line number.

    Returns
    -------
    list or None
        Block lines, or None when no Doxygen block precedes the signature.
    """
    index = _block_end(lines, start)
    if index is None:
        return None
    block = _block_lines(lines, index)
    if not _is_doxygen(block):
        return None
    return block


def _full_doc(block: list | None) -> bool:
    """
    Check a static-function block for all required tags.

    Parameters
    ----------
    block : list or None
        Doxygen block lines.

    Returns
    -------
    bool
        True when @brief, @param, and @return are all present.
    """
    if block is None:
        return False
    text = "\n".join(block)
    return all(tag in text for tag in ("@brief", "@param", "@return"))


def _brief_doc(block: list | None) -> bool:
    """
    Check a prototype block for a brief tag.

    Parameters
    ----------
    block : list or None
        Doxygen block lines.

    Returns
    -------
    bool
        True when a @brief tag is present.
    """
    return block is not None and "@brief" in "\n".join(block)


def _c_doc_errors(path: Path, lines: list, records: list) -> list[str]:
    """
    Find documentation-placement errors in a C source file.

    Parameters
    ----------
    path : pathlib.Path
        C source path.
    lines : list
        Source lines.
    records : list
        Function records for the file.

    Returns
    -------
    list[str]
        Human-readable documentation errors.
    """
    errors = []
    for record in records:
        block = _doc_block(lines, record["start"])
        if record["static"] and not _full_doc(block):
            errors.append(f"{path}: {record['name']} static needs full doc")
        if not record["static"] and block is not None:
            errors.append(f"{path}: {record['name']} exported has doc block")
    return errors


def _h_doc_errors(path: Path, lines: list, protos: list) -> list[str]:
    """
    Find prototypes missing a @brief block in a header.

    Parameters
    ----------
    path : pathlib.Path
        Header path.
    lines : list
        Source lines.
    protos : list
        Prototype records for the file.

    Returns
    -------
    list[str]
        Human-readable documentation errors.
    """
    errors = []
    for proto in protos:
        if not _brief_doc(_doc_block(lines, proto["start"])):
            errors.append(f"{path}: {proto['name']} missing @brief")
    return errors


def _doc_errors(path: Path, state: dict) -> list[str]:
    """
    Apply the suffix-specific documentation rules.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    state : dict
        Scanner state with records and prototypes.

    Returns
    -------
    list[str]
        Human-readable documentation errors.
    """
    lines = state["lines"]
    if path.suffix == ".c":
        return _c_doc_errors(path, lines, state["records"])
    return _h_doc_errors(path, lines, state["protos"])


def _dash_errors(path: Path) -> list[str]:
    """
    Find em/en dashes in one file.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

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


def _audit_file(path: Path) -> int:
    """
    Audit one file and print its violations.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    int
        One when any violation was found, otherwise zero.
    """
    state = _scan(path)
    blank = _blank_lines(path, state["records"])
    long = [r for r in state["records"] if r["count"] > MAX_EXEC_LINES]
    docs = _doc_errors(path, state)
    dashes = _dash_errors(path)
    _report(path, blank, long, docs, dashes)
    return int(bool(blank) or bool(long) or bool(docs) or bool(dashes))


def _report(path: Path, blank: list, long: list, docs: list,
            dashes: list) -> None:
    """
    Print one file's audit findings.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    blank : list
        Blank line numbers inside bodies.
    long : list
        Functions over the executable-line limit.
    docs : list
        Documentation errors.
    dashes : list
        Dash errors.

    Returns
    -------
    None
    """
    if blank:
        print(f"{path}: blank lines in body {blank}")
    for record in long:
        print(f"{path}: {record['name']} has {record['count']} exec lines")
    for error in docs:
        print(error)
    for error in dashes:
        print(error)


def main() -> int:
    """
    Audit all owned C and header files.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero for a clean audit, otherwise one.
    """
    failures = 0
    for path in _owned_files():
        failures += _audit_file(path)
    return int(bool(failures))


if __name__ == "__main__":
    raise SystemExit(main())
