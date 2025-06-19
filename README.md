# Crop Tracking — C++ Reference Implementation  
(☘ Agricultural-Robotics Take-Home Challenge)

![summary demo](data/output/simple_3_frames/visualization/summary.png)

## ✨ What this repo contains

| Folder / File                   | Purpose |
|---------------------------------|---------|
| `src/`                           | C++17 source (`main.cpp`, `tracker.hpp/.cpp`) |
| `CMakeLists.txt`                 | Cross-platform build script |
| `Dockerfile` (multistage)       | Reproducible Ubuntu 22.04 image |
| `data/`                          | **All I/O lives here** |
|  └─ `input/`                     | JSON test-datasets (edge-cases) |
|  └─ `output/` (autocreated)     | Tracker writes results here (`tracking_output.json` + PNGs) |
| `run_all_examples.bat`          | Windows batch script – processes every JSON in `data/input/` |
| `LICENSE` (MIT)                 | Open-source license |

---

## 1  Building locally (Windows / Linux / macOS)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc)
Outputs: crop_tracking[.exe] in build/.

2 Running with Docker
No local OpenCV or compiler needed.

bash
Copy
Edit
docker build -t tracking-solution .
docker run --rm -v "$(pwd)":/project tracking-solution \
  --input  /project/data/input/simple_3_frames.json \
  --output /project/data/output/simple_3_frames/tracking_output.json \
  --vis-dir /project/data/output/simple_3_frames/visualization
Batch-run every dataset (Windows)
cmd
Copy
Edit
run_all_examples.bat
Creates data/output/<dataset>/… for each file in data/input/.

3 Dataset catalogue & edge-cases
Dataset (JSON)	Frames	Objects/frame	Edge-case demonstrated
simple_3_frames.json	3	1	Happy-path sanity check
occlusion_1frame.json	6	2-3	Temporary disappearance (≤1 frame) – IDs persist
occlusion_3frames.json	10	2-3	Long disappearance (3 frames) – tracker drops & re-IDs (spec limit)
overlapping.json	8	2	Bounding-boxes overlap significantly – nearest-centroid tie-break
many_objects_100.json	10	100	Stress-test: ID consistency under density
long_200_frames.json	200	5-8	Runtime & memory stability on long sequences
pixel_coords.json	5	3	Inputs in pixel coordinates — pre-process conversion path
frame_0001/0002.json	1 each	raw YOLO-style outputs (strings → numbers) – parser robustness	

All coordinates are normalised 0-1 unless noted.
Each file lives in data/input/; output mirrors under data/output/.

4 Tracker behaviour
Nearest-centroid association with per-frame Hungarian fallback

ID hold-over: objects may vanish up to max_missing frames (default 3)

Motion trails & summary heatmap in visualisations

Zero external deps except OpenCV ≥ 4.5 and nlohmann/json.

5 Extending
Drop new JSON into data/input/ → rerun batch or Docker line.

Tweak algorithm thresholds in tracker.hpp (MAX_DIST, MAX_MISSING).

Add new visual layers in tracker.cpp::visualize() (e.g., FPS, class labels).

License
Released under the MIT License — see LICENSE file.