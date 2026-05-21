"""Generate a clean Markdown file tree of D:\\roc-ai-vision."""
import os
from pathlib import Path

ROOT = Path(r"D:\roc-ai-vision")
OUT = ROOT / "FILE_TREE.md"

EXCLUDE_DIRS = {
    "node_modules", ".svelte-kit", ".git", "build", ".venv", "venv",
    "__pycache__", ".cache", "dist", ".firecrawl",
    "_deps", "CMakeFiles",   # cmake build artifacts
}
# Hide large binary blobs from display but still count
BINARY_EXT = {".onnx", ".dll", ".lib", ".exe", ".obj", ".pdb", ".a", ".so", ".pyd"}


def fmt_size(n: int) -> str:
    for unit in ("B", "KB", "MB", "GB"):
        if n < 1024:
            return f"{n:.1f} {unit}" if unit != "B" else f"{n} B"
        n /= 1024
    return f"{n:.1f} TB"


def walk(path: Path, prefix: str, lines: list, depth: int, max_depth: int):
    try:
        entries = sorted(
            path.iterdir(),
            key=lambda p: (not p.is_dir(), p.name.lower()),
        )
    except (PermissionError, OSError):
        return

    entries = [e for e in entries if not (e.is_dir() and e.name in EXCLUDE_DIRS)]

    for i, e in enumerate(entries):
        last = i == len(entries) - 1
        connector = "└── " if last else "├── "
        if e.is_dir():
            # Count children of excluded/deep dirs
            try:
                child_count = sum(1 for _ in e.iterdir())
            except Exception:
                child_count = 0
            if depth >= max_depth:
                lines.append(f"{prefix}{connector}{e.name}/  ({child_count} entries — collapsed)")
                continue
            lines.append(f"{prefix}{connector}{e.name}/")
            walk(e, prefix + ("    " if last else "│   "), lines, depth + 1, max_depth)
        else:
            try:
                size = e.stat().st_size
            except OSError:
                size = 0
            tag = ""
            if e.suffix.lower() in BINARY_EXT:
                tag = f"  *[{fmt_size(size)}]*"
            elif size > 64 * 1024:
                tag = f"  *[{fmt_size(size)}]*"
            lines.append(f"{prefix}{connector}{e.name}{tag}")


def count_excluded(root: Path) -> dict:
    counts = {}
    for ex in EXCLUDE_DIRS:
        total = 0
        for dirpath, dirnames, filenames in os.walk(root):
            if Path(dirpath).name == ex:
                total += len(filenames)
                # Also recurse
                for _dirpath, _dirnames, _filenames in os.walk(dirpath):
                    total += len(_filenames)
                # Don't descend further to avoid double count
                dirnames[:] = []
        if total:
            counts[ex] = total
    return counts


def main():
    lines = ["# ROC AI Vision — Disk File Tree",
             "",
             "> Auto-generated snapshot of the actual files on disk.",
             "> For the curated, living architectural tree see [`workspace_blueprint/project_tree.md`](workspace_blueprint/project_tree.md).",
             ""]
    # Header stats
    total_files = sum(len(f) for _, _, f in os.walk(ROOT))
    total_size = sum(
        (Path(d) / f).stat().st_size
        for d, _, files in os.walk(ROOT)
        for f in files
        if (Path(d) / f).exists()
    )
    lines.append(f"- **Root:** `{ROOT}`")
    lines.append(f"- **Total files on disk:** {total_files:,}")
    lines.append(f"- **Total size:** {fmt_size(total_size)}")
    lines.append("")
    excluded = count_excluded(ROOT)
    if excluded:
        lines.append("**Excluded from tree (noise / build artifacts):**")
        lines.append("")
        for name, count in sorted(excluded.items(), key=lambda x: -x[1]):
            lines.append(f"- `{name}/` — {count:,} files hidden")
        lines.append("")
    lines.append("---")
    lines.append("")
    lines.append("```")
    lines.append(f"{ROOT.name}/")
    walk(ROOT, "", lines, depth=0, max_depth=6)
    lines.append("```")

    OUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote {OUT}  ({len(lines)} lines)")


if __name__ == "__main__":
    main()
