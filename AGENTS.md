# AGENTS.md

## Project intent

`pixel` is a local pixel-art editor built in C with raylib + raygui.
The code should stay straightforward and easy to read.

## Design choices

- Keep architecture practical and explicit:
  - clear data structs,
  - clear input/update/render flow,
  - no hidden magic behavior.
- Prefer predictable behavior over clever optimization.
- Keep UI interactions discoverable and stable.
- Keep file format and path handling stable across Linux and Windows.

## C style rules

- Write plain C11.
- Prefer simple control flow and small helper functions.
- Avoid advanced tricks/macros unless absolutely necessary.
- Avoid dense one-liners when they reduce readability.
- Use descriptive names for functions/variables.
- Keep constants explicit (`#define` values with meaningful names).
- Validate bounds and indices defensively.
- Put reusable logic in `src/pixel_core.c`/`src/pixel_core.h` when feasible.

## Function comment rules

Every non-trivial function should have a short comment above it describing:

1. What it does.
2. Inputs/outputs or side effects.
3. Important assumptions.

Keep comments short and practical.

Example format:

```c
// Load app data paths for current platform.
// Side effects: updates library/font/palette paths.
// Assumes environment variables may be missing.
static void InitRuntimePaths(void) { ... }
```

## Line comment rules

- Add line comments only where logic is not obvious.
- Use comments for intent, not for restating code.
- Good use cases:
  - tricky math,
  - state synchronization,
  - parser edge cases,
  - UI interaction subtleties.

## raylib/raygui style rules

- Keep rendering path simple:
  - input/update first,
  - then drawing,
  - then modal overlays.
- Keep colors and spacing centralized when possible.
- Avoid heavy visual complexity; prefer readable contrast and clear hierarchy.
- Keep keyboard and mouse interactions consistent.
- Keep dialogs modal and avoid hidden shortcut behavior while typing.
- UI behavior should remain responsive at normal canvas sizes.

## Current UX contracts

- Save/load dialog behavior:
  - opening Save/Load should focus filename input immediately,
  - Enter confirms current dialog action,
  - Escape cancels current dialog.
- Save behavior:
  - `Save as PNG` also writes matching `.txt` project data for reload,
  - `Save as TXT` writes `.txt` project data only.
- Load behavior:
  - `Load TXT` reads `.txt` project files and must be robust to malformed rows.
- Quit behavior:
  - no default Escape-to-quit,
  - `Ctrl+Q` opens confirmation modal,
  - Enter or `Yes` confirms quit.
- Status bar should show key state hints (for example quit hint).

## Data paths

- User save directory should be platform-standard:
  - Linux: `$XDG_DATA_HOME/pixel/library` or fallback `~/.local/share/pixel/library`
  - Windows: `%LOCALAPPDATA%/pixel/library` (fallback `%APPDATA%/pixel/library`)
- Installed assets should resolve from shared app data directory when not run from repo.

## Testing and safety

- Prefer adding tests in `tests/test_pixel_core.c` for parser/helper behavior.
- After any behavior change, update existing tests (or add new ones) so tests reflect current behavior.
- Before every commit, run the test suite (`make test`) and fix failures first.
- For UI changes, preserve existing workflows (paint/erase, brush size wheel, save/load dialogs, quit confirm).
- Do not introduce destructive filesystem operations.

## Build and run

- Linux:
  - build: `make`
  - run: `make run`
  - test: `make test`
- Windows (MinGW):
  - build: `make -f Makefile.win`
  - run: `make -f Makefile.win run`

## Documentation

- Update `README.md` when user-visible controls, save format, or install behavior changes.
- Keep Makefile targets and runtime behavior aligned with docs.
