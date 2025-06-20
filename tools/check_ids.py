#!/usr/bin/env python3
"""check_ids.py  —  quick integrity check for crop‑tracking results

Usage
-----
    python check_ids.py tracking_output.json [--max-gap 3]

If every object ID in the JSON refers to a single physical object (no ID
switches / re‑use) the script prints a ✅ summary and exits 0.
Otherwise it prints an error describing the first conflict and exits 1.

The check is purely geometric / logical — it does **not** look at the original
raw detections.  It simply asserts that for *any* frame pair inside the
configured `--max-gap` a given ID never has two active boxes whose centroids
are too far apart to plausibly belong to the same thing.

This is good enough for regression tests & CI.
"""

import argparse
import json
import math
import sys
from pathlib import Path


# --------------------------------- helpers ----------------------------------

def dist(a, b):
    """Euclidean distance between two (x, y) tuples in normalised units."""
    return math.hypot(a[0] - b[0], a[1] - b[1])


def load_frames(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


# ---------------------------------- main ------------------------------------

def check_ids(frames, max_gap=3, max_dist=0.5):
    """Return `(ok: bool, msg: str)` after scanning for ID re‑use/switches."""
    # Map id -> list[(frame_idx, cx, cy)]
    trajectory = {}
    for idx, frame in enumerate(frames):
        for obj in frame["tracked_objects"]:
            cx = float(obj["x"])
            cy = float(obj["y"])
            trajectory.setdefault(obj["id"], []).append((idx, cx, cy))

    # For each id, compare successive detections — they must be within max_dist
    for obj_id, points in trajectory.items():
        points.sort(key=lambda t: t[0])
        for (f1, x1, y1), (f2, x2, y2) in zip(points, points[1:]):
            gap = f2 - f1
            dist_value = dist((x1, y1), (x2, y2))
            if gap <= max_gap and dist_value > max_dist:
                print(f"Debug: ID {obj_id} jumps between frames {f1} and {f2}")
                print(f"  - Centroid 1: ({x1}, {y1}) in frame {f1}")
                print(f"  - Centroid 2: ({x2}, {y2}) in frame {f2}")
                print(f"  - Distance: {dist_value:.3f}")
                return (False,
                        f"ID {obj_id} jumps between frames {f1} and {f2} "
                        f"(gap {gap}, dist {dist_value:.3f})")
    return (True, "IDs are consistent")



if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Verify persistent IDs")
    ap.add_argument("json", type=Path, help="tracking_output.json file")
    ap.add_argument("--max-gap", type=int, default=3,
                    help="max allowed missing frames before we ignore the check")
    ap.add_argument("--max-dist", type=float, default=0.15,
                    help="max centroid distance (normalised) between two track points")
    args = ap.parse_args()

    try:
        frames = load_frames(args.json)
    except Exception as e:
        print(f"❌ Failed to load JSON: {e}")
        sys.exit(1)

    ok, msg = check_ids(frames, max_gap=args.max_gap, max_dist=args.max_dist)
    prefix = "✅" if ok else "❌"
    print(f"{prefix} {msg}  ({args.json})")
    sys.exit(0 if ok else 1)
