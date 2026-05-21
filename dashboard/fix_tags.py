from pathlib import Path

BAD = "</" + "motion.div>"
GOOD = "</motion.div>"  # will fix below
GOOD = "</div>"

observer_path = Path(__file__).parent / "src/lib/components/domains/ObserverMatrixPanel.svelte"
content = (Path(__file__).parent / "observer_template.txt").read_text(encoding="utf-8")
content = content.replace(BAD, GOOD)
observer_path.write_text(content, encoding="utf-8")

for p in Path(__file__).parent.joinpath("src").rglob("*.svelte"):
    t = p.read_text(encoding="utf-8")
    if BAD in t:
        p.write_text(t.replace(BAD, GOOD), encoding="utf-8")
        print("fixed", p.name)

print("done", observer_path.stat().st_size)
