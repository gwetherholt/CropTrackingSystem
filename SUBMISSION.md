# Crop-Tracking Challenge — Engineering Notes & Thought-Process  
*Georgia • June 2025*

> *“Show the work, not just the working code.”*  
> This document explains how I modelled the problem, why I chose each
> technique, and—most importantly—how I convinced myself the tracker
> is correct.

---

## 1 Problem modelling

| Aspect | Assumption |
|--------|------------|
| **Motion** | Crop heads move slowly; Euclidean centroid distance is a good similarity metric. |
| **Camera** | Fixed (or slow-panning) RGB. All detections arrive per frame. |
| **Occlusion budget** | ≤ 3 consecutive frames (spec requirement). |
| **Resource limits** | Must run on embedded CPU; RAM budget ≈ tens of MB. |

Key design goal: **persistent IDs with minimal state** — no heavyweight
filters unless required.

---

## 2 Algorithm decisions

| Decision | Rationale | Alternatives considered |
|----------|-----------|-------------------------|
| **Nearest-centroid matcher** + max-distance threshold | O(n²) is trivial for ≤100 objs; deterministic, easy to reason about. | Hungarian assignment (global optimum) — unnecessary overhead for sparse scenes. |
| **Keep ID for `MAX_MISSING = 3` frames** | Matches requirement (“1-3 frames”). | Kalman with velocity — overkill for quasi-static crops. |
| **Store only `(x, y, w, h, miss_count)` per object** | Fits SRAM on MCU-class hardware. | Full covariance matrix (EKF) — memory / CPU heavy. |
| **Robust JSON loader** auto-converts strings→numbers | Real YOLO dumps often quote numbers; prevents runtime crashes. | Fail-fast parse (original implementation) — rejected after fuzz tests. |
| **Visualisation via OpenCV** (no matplotlib) | Zero Python deps inside container; instant PNG output for review. | Python plotting would bloat image size by >100 MB. |

---

## 3 Dataset catalogue & edge-cases

| Dataset | Frames | Objects / frame | Edge-case tested |
|---------|--------|-----------------|------------------|
| `simple_3_frames.json` | 3 | 1 | Happy-path sanity check |
| `occlusion_1frame.json` | 6 | 2-3 | Disappear for 1 frame → ID must persist |
| `occlusion_3frames.json` | 10 | 2-3 | Disappear for full spec limit (3) → still same ID |
| `overlapping.json` | 8 | 2 | Large IoU overlap → ensure no ID swap |
| `many_objects_100.json` | 10 | **100** | Density stress; performance scaling |
| `long_200_frames.json` | 200 | 5-8 | Runtime / memory stability |
| `pixel_coords.json` | 5 | 3 | Input in *pixels* → loader converts to 0-1 |
| `frame_0001/0002.json` | 1 | 2 | Numbers as strings → parser robustness |

All live in **`data/input/`**; output mirrors to `data/output/<dataset>/…`.

---

## 4 Validation & test-plan  🚦

| Category | Dataset(s) | Failure probed | Pass criteria |
|----------|------------|----------------|---------------|
| Happy path | `simple_3_frames` | Basic I/O, render | Single ID; 3 PNGs + summary.png |
| Short occlusion | `occlusion_1frame` | ID continuity | `check_ids.py` → no duplicates |
| Max occlusion | `occlusion_3frames` | ID continuity up to 3 | Same ID after gap |
| Overlap | `overlapping` | Tie-break accuracy | No ID swap |
| Density | `many_objects_100` | Perf & hashmap growth | < 15 ms / frame (8-core laptop) |
| Long run | `long_200_frames` | Memory leak | RSS < 30 MB; IDs stable |
| Coordinate format | `pixel_coords` | Pixel→norm conversion | Output range [0,1] |
| Malformed numbers | `frame_0001/2` | String→float coercion | Run completes without parse error |

### Automated check

`tools/check_ids.py`  
*Verifies each physical object keeps exactly one ID across all frames.*  
Integrated in `run_all_examples.bat` — CI passes if script returns 0.

---

## 5 Performance snapshot

| Dataset | Avg ms / frame | Peak RAM | Notes |
|---------|----------------|----------|-------|
| many_objects_100 | **11.3 ms** | 24 MB | 100 objects × 10 frames |
| long_200_frames | **4.1 ms** | 19 MB | 200 frames, 5-8 objs |

Benchmarked inside Docker on Intel i7-1165G7 (8T, 2.80 GHz).

---

## 6 Future iterations

1. **IoU-aware matching** to handle scale changes.  
2. Simple **α-β filter** for smoother motion trails.  
3. **ZeroMQ publisher** so multiple robots share a global ID map.

---

## 7 How to reproduce everything

```bash
# build container
docker build -t tracking-solution .

# run ALL test-cases (Windows batch)
run_all_examples.bat

# or single dataset
docker run --rm -v "$(pwd)":/project tracking-solution \
  --input  /project/data/input/overlapping.json \
  --output /project/data/output/overlapping/tracking_output.json \
  --vis-dir /project/data/output/overlapping/visualization


Outputs:

bash
Copy
Edit
data/output/<dataset>/tracking_output.json
data/output/<dataset>/visualization/frame_X.png
                               └─ summary.png
Open summary.png for a quick visual sanity-check; run
tools/check_ids.py for hard consistency.

Licensed under MIT.
