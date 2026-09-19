---
name: Generate Course PDF
description: Renders a markdown file into the strict PDF format used by the Embedded-Hacking course, ensuring strict ASCII box alignment and LaTeX math rendering.
---

# Generate Course PDF

When creating, editing, or generating PDFs for the Embedded-Hacking course in the strict/exact format, follow these mandatory steps:

## 1. Clean Emojis from Markdown
Emojis and certain high-surrogate Unicode characters can cause the Puppeteer/PDF.js render to crash with unresolved pattern errors in VS Code.
Run a script to strip emojis and high-surrogate Unicode characters from the target Markdown file before converting:
```python
import re, sys
with open(sys.argv[1], 'r', encoding='utf-8') as f:
    text = f.read()
clean_text = re.sub(r'[\U00010000-\U0010ffff]', '', text)
with open(sys.argv[1], 'w', encoding='utf-8') as f:
    f.write(clean_text)
```

## 2. Mandatory ASCII Art Box Alignment Audit
All ASCII art boxes in the course notes must have pixel-perfect character alignment.
- The standard course ASCII box width is exactly **67 characters** (`+` followed by 65 `-` followed by `+`).
- Every line inside the box MUST begin with `|` and end with `|` at the exact same column width (length 67).
- A closing `|` that is even one character too far to the left or right is strictly forbidden.
- Always run an automated audit before rendering:
```python
import sys
with open(sys.argv[1], 'r', encoding='utf-8') as f:
    lines = [l.rstrip('\r\n') for l in f.readlines()]
in_code = False
box = []
for idx, l in enumerate(lines, 1):
    if l.strip().startswith('```'):
        in_code = not in_code
        box = []
        continue
    if in_code and len(l.strip()) > 2 and ((l.strip().startswith('+') and l.strip().endswith('+')) or (l.strip().startswith('|') and l.strip().endswith('|'))):
        box.append((l, idx))
    else:
        if len(box) >= 3 and box[0][0].startswith('+') and box[-1][0].startswith('+'):
            target_len = len(box[0][0])
            for bl, bidx in box:
                if len(bl) != target_len:
                    raise ValueError(f"ASCII box alignment error on line {bidx}: expected {target_len} chars, got {len(bl)}: {bl}")
        box = []
```

## 3. LaTeX Math Rendering
- The PDF renderer uses `marked-katex-extension` with self-contained, inlined Base64 KaTeX `woff2` fonts.
- Inline math must use `$formula$` and display math must use `$$formula$$`.
- Never leave broken or unbalanced LaTeX delimiters.
- Verify in the final PDF that all math expressions render cleanly with zero raw `$` delimiters in the body text.

## 4. Run the Render Script
The strict course format rendering script is located in `Documents/data-science/EH`:
- Working Directory: `Documents/data-science/EH`
- Command: `node render-lesson-pdf.mjs [ABSOLUTE_PATH_TO_SOURCE_MD] [ABSOLUTE_PATH_TO_OUTPUT_PDF]`

## 5. Verification & Git Hygiene
- Inspect the generated PDF to confirm proper page layout, fonts, and headers/footers (`Course Notes`).
- Ensure `.DS_Store` or other OS metadata files are never staged or committed to the repository (verify `.gitignore`).
