# summarize.py <logfile> [skip_ms]
import re, sys, statistics as st

skip = int(sys.argv[2]) if len(sys.argv) > 2 else 10000
pat = re.compile(r"\((\d+)\) wifi station: Draw time: ([\d.]+) us, Push time: ([\d.]+) us")
rows = [tuple(map(float, m.groups())) for l in open(sys.argv[1]) if (m := pat.search(l))]
rows = [r for r in rows if r[0] >= skip]          # discard warm-up
draw = [r[1] for r in rows]; push = [r[2] for r in rows]
print(f"samples={len(rows)}")
for name, v in (("draw", draw), ("push", push)):
    print(f"{name}: avg={st.mean(v):.0f} min={min(v):.0f} max={max(v):.0f} stdev={st.pstdev(v):.0f} us")